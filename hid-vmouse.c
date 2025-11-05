// Copyright (c) 2025 droc101
// GNU General Public License v3.0+ (see COPYING or https://www.gnu.org/licenses/gpl-3.0.txt)

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/input.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("droc101");

#define BUTTON_CMD_RESET 0
#define BUTTON_CMD_DOWN 1
#define BUTTON_CMD_UP 2
#define BUTTON_CMD_CLICK 3

#define BUTTONS_BASE BTN_MOUSE

int allow_non_root_write = 0;
module_param(allow_non_root_write, int, 0);

#pragma region mouse

static struct input_dev *virtual_mouse = NULL;

static int init_vmouse(void)
{
    int ret;
    virtual_mouse = input_allocate_device();
    if (!virtual_mouse)
    {
        printk(KERN_ERR "vmouse: Failed to allocate input device\n");
        return -ENOMEM;
    }

    virtual_mouse->name = "Droc101 Development Simulated Clicking and Pointing Device (/dev/vmouse)";
    virtual_mouse->phys = "vmouse_input";
    virtual_mouse->id.bustype = BUS_VIRTUAL;
    virtual_mouse->id.vendor = 0;
    virtual_mouse->id.product = 0;
    virtual_mouse->id.version = 1;

    set_bit(EV_KEY, virtual_mouse->evbit);
    set_bit(EV_REL, virtual_mouse->evbit);
    set_bit(REL_X, virtual_mouse->relbit);
    set_bit(REL_Y, virtual_mouse->relbit);

    set_bit(BTN_LEFT, virtual_mouse->keybit);
    set_bit(BTN_RIGHT, virtual_mouse->keybit);
    set_bit(BTN_MIDDLE, virtual_mouse->keybit);
    set_bit(BTN_SIDE, virtual_mouse->keybit);
    set_bit(BTN_EXTRA, virtual_mouse->keybit);
    set_bit(BTN_FORWARD, virtual_mouse->keybit);
    set_bit(BTN_BACK, virtual_mouse->keybit);
    set_bit(BTN_TASK, virtual_mouse->keybit);

    ret = input_register_device(virtual_mouse);
    if (ret)
    {
        printk(KERN_ERR "vmouse: Failed to register virtual mouse\n");
        input_free_device(virtual_mouse);
        return ret;
    }

    printk(KERN_INFO "vmouse: Virtual mouse initialized\n");
    return 0;
}

#pragma endregion

static void handle_button_command(unsigned char command, unsigned char payload)
{
    if (!virtual_mouse)
    {
        printk(KERN_ALERT "vmouse: virtual_mouse is NULL!");
        return;
    }
    // printk(KERN_INFO "vmouse: received command %u with payload %u\n", command, payload);
    switch (command)
    {
    case BUTTON_CMD_RESET:
        for (int i = 0; i < 8; i++)
        {
            input_report_key(virtual_mouse, BUTTONS_BASE + i, 0);
        }
        break;
    case BUTTON_CMD_DOWN:
        // printk(KERN_INFO "vmouse: pressing button 0x%x\n", BUTTONS_BASE + payload);
        input_report_key(virtual_mouse, BUTTONS_BASE + payload, 1);
        break;
    case BUTTON_CMD_UP:
        // printk(KERN_INFO "vmouse: releasing button 0x%x\n", BUTTONS_BASE + payload);
        input_report_key(virtual_mouse, BUTTONS_BASE + payload, 0);
        break;
    case BUTTON_CMD_CLICK:
        // printk(KERN_INFO "vmouse: clicking button 0x%x\n", BUTTONS_BASE + payload);
        input_report_key(virtual_mouse, BUTTONS_BASE + payload, 1);
        input_sync(virtual_mouse);
        input_report_key(virtual_mouse, BUTTONS_BASE + payload, 0);
        break;
    default:
        printk(KERN_INFO "vmouse: received unknown command %u with payload %u\n", command, payload);
    }

    input_sync(virtual_mouse);
}

#pragma region device

#define DEVICE_NAME "vmouse"

static dev_t dev_number;
static struct cdev my_cdev;
static struct class *dev_class = NULL;

static int device_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
    return 0;
}

static ssize_t device_write(struct file *file, const char __user *buf,
                            size_t count, loff_t *ppos)
{
    unsigned char commands[128];

    if (count > 128) {
        return -EFAULT;
    }

    if (copy_from_user(commands, buf, count))
        return -EFAULT;

    for (size_t i = 0; i < count; i++)
    {
        unsigned char fullPayload = commands[i];
        unsigned char isMove = fullPayload & 0b10000000;

        if (isMove)
        {
            // TODO
        }
        else
        {
            unsigned char command = (fullPayload >> 4) & 0b00001111;
            unsigned char payload = fullPayload & 0b00000111;
            handle_button_command(command, payload);
        }
    }

    return count;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .release = device_release,
    .write = device_write};

static int device_uevent(const struct device *dev, struct kobj_uevent_env *env)
{
    if (allow_non_root_write)
    {
        add_uevent_var(env, "DEVMODE=0222"); // c-w--w--w-
    }
    else
    {
        add_uevent_var(env, "DEVMODE=0200"); // c-w-------
    }
    return 0;
}

int init_device(void)
{
    int ret;

    ret = alloc_chrdev_region(&dev_number, 0, 1, DEVICE_NAME);
    if (ret < 0)
    {
        printk(KERN_ALERT "vmouse: Failed to allocate device number\n");
        return ret;
    }

    cdev_init(&my_cdev, &fops);
    ret = cdev_add(&my_cdev, dev_number, 1);
    if (ret < 0)
    {
        unregister_chrdev_region(dev_number, 1);
        printk(KERN_ALERT "vmouse: Failed to add cdev\n");
        return ret;
    }

    dev_class = class_create("vmouse_class");
    if (IS_ERR(dev_class))
    {
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev_number, 1);
        printk(KERN_ALERT "vmouse: Failed to create class\n");
        return PTR_ERR(dev_class);
    }

    dev_class->dev_uevent = device_uevent;

    if (IS_ERR(device_create(dev_class, NULL, dev_number, NULL, DEVICE_NAME)))
    {
        class_destroy(dev_class);
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev_number, 1);
        printk(KERN_ALERT "vmouse: Failed to create device\n");
        return -1;
    }

    printk(KERN_INFO "vmouse: Device created successfully: /dev/%s\n", DEVICE_NAME);
    return 0;
}

#pragma endregion

static int __init vmouse_init(void)
{
    int dStatus = init_device();
    if (dStatus != 0)
        return dStatus;
    int mStatus = init_vmouse();
    if (mStatus != 0)
        return mStatus;
    return 0;
}

static void __exit vmouse_exit(void)
{
    if (dev_class)
    {
        device_destroy(dev_class, dev_number);
        class_destroy(dev_class);
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev_number, 1);
        printk(KERN_INFO "vmouse: Device removed\n");
    }
    if (virtual_mouse)
    {
        input_unregister_device(virtual_mouse);
        printk(KERN_INFO "vmouse: Virtual mouse removed\n");
    }
}

module_init(vmouse_init);
module_exit(vmouse_exit);