# Udev Rules {#udev-rules}

udev (userspace /dev) is a device manager for the Linux kernel.
It can be used to add actions to the attachment/removal of certain devices.

This is used to make certain devices available without root privileges
or to recognize older devices.

The following devices require our udev rules:

- DFK AFU050
- DFK AFU420
- All USB-2.0 with a 21, 31 or 41 in the name

## UVC Extension Units

uvc extension units are also applied through udev rules. 
These rules are provided by uvcdynctrl and not by tiscamera.
