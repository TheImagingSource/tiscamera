# Udev and UVC extensions {#udev-rules}

## Udev

udev (userspace /dev) is a device manager for the Linux kernel.
It can be used to add actions to the attachment/removal of certain devices.

This is used to make certain devices available without root privileges
or to recognize older devices.

The following devices require our udev rules:

- DFK AFU050
- DFK AFU420
- All USB-2.0 with a 21, 31 or 41 in the name

The default installation path is: _/etc/udev/rules.d_

## UVC Extension Units

UVC extension units are also applied through udev rules.  

The extension units are json files that describe how certain device interactions
should be mapped to UVC and v4l2 properties.  
Most USB cameras require these units for full functionality.

The default installation path is: _/usr/share/theimagingsource/tiscamera/uvc-extension__
