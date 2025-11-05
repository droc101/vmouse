# vmouse
A DKMS Linux kernel module that provides a virtual mouse that can be used to press and release buttons on both x11 and wayland

## Usage
The driver provides a device file at `/dev/vmouse` for commands. Each byte written to this file is an independant command in the following format, withc each letter representing a bit: `MCCCCDDD` where `M` must be zero, `CCCC` is the 4-bit command ID, and `DDD` is the mouse button ID.

| Command (Decimal) | Description          | Payload   |
|-------------------|----------------------|-----------|
| 0                 | Releases all buttons | Ignored   |
| 1                 | Press a button       | Button ID |
| 2                 | Release a button     | Button ID |
| 3                 | Click a button       | Button ID |

Button IDs can be found in [`linux/include/uapi/linux/input-event-codes.h`](https://github.com/torvalds/linux/blob/c9cfc122f03711a5124b4aafab3211cf4d35a2ac/include/uapi/linux/input-event-codes.h#L356). Subtract 0x110 to get the vmouse button ID.

## Installing
This repository contains a PKGBUILD file for Arch Linux. To use it, clone this repo and run `makepkg -si` in the repo's folder.