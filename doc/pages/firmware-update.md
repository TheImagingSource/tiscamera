# firmware-update {#firmware-update}

firmware-update is a commandline tool to update the firmware of The Imaging Source USB cameras. 

### examples

List all available cameras:
```
firmware-update -l
```

Get information about a single camera:
```
firmware-update -id <serial number>
```

Apply firmware
```
firmware-update -ud <serial number> -f firmware-file
```

Switch camera to UVC/proprietary mode
```
firmware-update -d <serialnumber> -m uvc
firmware-update -d <serialnumber> -m proprietary
```

Switching to uvc mode is only necessary for USB2 cameras. 

## Firmware Files

If you need firmware files for USB-3.X cameras, please send us a request.

http://www.theimagingsource.com/en_US/company/contact/

Firmware for our USB-2.0 cameras can be found in our Linux repository in data/firmware.

If you are unsure on how to proceed, [please contact our support](@ref contact).
