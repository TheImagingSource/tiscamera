

# What is this

This tool allows you to update the firmware of your TIS USB camera.

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

# Usb2 

Firmware file for usb-2.0 cameras are located in data/firmware/usb2

# Usb3

If you need firmware files for USB3 cameras,
please send us a request.

http://www.theimagingsource.com/en_US/company/contact/
