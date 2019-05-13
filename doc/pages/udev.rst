####
Udev
####

udev (userspace /dev) is a device manager for the Linux kernel.
It can be used for the following:

- Add actions to the attachment/removal of certain devices.
- Make devices available without root privileges.
- Recognize older devices
- Load UVC extension units.

The udev rules file is generated during the build step;
the template is located at data/udev/
 
The default installation path is: /etc/udev/rules.d

For more information, refer to the man pages of udev (7,8).
