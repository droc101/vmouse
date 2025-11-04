# vmouse
A DKMS Linux kernel module that provides a virtual mouse that can be used to press and release buttons on both x11 and wayland

## Usage
The driver provides a device file at `/dev/vmouse` for commands. To use it, write one byte at a time to send commands. The two most significant bits specify the command, and the remaining bits are used as the payload.

| Command (Decimal) | Description          | Payload   |
|-------------------|----------------------|-----------|
| 0                 | Releases all buttons | Ignored   |
| 1                 | Press a button       | Button ID |
| 2                 | Release a button     | Button ID |

Button IDs can be found in [`linux/include/uapi/linux/input-event-codes.h`](https://github.com/torvalds/linux/blob/c9cfc122f03711a5124b4aafab3211cf4d35a2ac/include/uapi/linux/input-event-codes.h#L356). Subtract 0x110 to get the vmouse button ID.