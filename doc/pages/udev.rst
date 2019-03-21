####
Udev
####

udev (userspace /dev) is a device manager for the Linux kernel.
It can be used to add actions to the attachment/removal of certain devices.

This is used to make devices available without root privileges,
 to recognize older devices and too load UVC extension units.

The udev rules file is generate during the build step.
The template is located at data/udev/
 
The default installation path is: /etc/udev/rules.d

For more information refer to the man pages of udev (7,8).
