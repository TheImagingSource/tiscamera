

# What is this

This tool allows you to update the firmware of your TIS USB camera.

# Build

You will need gcc >= 4.7 to successfully compile this tool.

Simply call:

make

## Dependencies

In order to build this tool you need:

libusb-1.0  and its header files
libzip      and its header files

# Usage

### examples

List all available cameras:
firmware-update -l

Get information about a single camera:
firmware-update -id <serial number>

Apply firmware
firmware-update -ud <serial number> -f firmware-file

Switch camera to UVC/proprietary mode
firmware-update -d <serialnumber> -m uvc
firmware-update -d <serialnumber> -m proprietary

Switching to uvc mode is only necessary for USB2 cameras. 

# Firmware Files

If you need firmware files for USB3 cameras,
please send us a request.

http://www.theimagingsource.com/en_US/company/contact/

# Problems & Questions

For both visit us at:

https://github.com/TheImagingSource/tiscamera

