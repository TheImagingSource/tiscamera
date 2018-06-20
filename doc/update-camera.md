# Updating your camera

If you are unsure if you require a firmware update or require a firmware file,
please contact our support at support@theimagingsource.com

### GStreamer 1.0 compatability

**This is only required if you have a USB2 72, 42 or 22 color camera**

### USB

To upgrade you need to have built tiscamera with usb support enabled.

Please ensure that the camera you are trying to update is the only one connected to your PC.

To identify your camera use the command

    sudo tcam-ctrl --list

This will list all supported cameras.

Now get information about your camera by executing

    sudo tcam-ctrl -id SERIAL

The serial is printed on your camera and can also be retrieved by the --list command.

The command to upgrade your camera is

    sudo tcam-ctrl -ud SERIAL -f FILEPATH

For usb2 cameras you can find all firmware files in <tiscamera>/data/firmware/usb2.
For usb3 camera firmware files, please contact our support.

### GigE

To upgrade you need to have built tiscamera with gige support enabled.

Please ensure that the camera you are trying to update is the only one connected to your PC.

To identify your camera use the command

    camera-ip-conf-cli -l


This will list all supported cameras.

Now get information about your camera by executing

    camera-ip-conf-cli -i -s <CAMERA_SERIAL>

The serial is printed on your camera and can also be retrieved by the --list command.

The command to upgrade your camera is

    camera-ip-conf-cli upload firmware=<FILE> -s <CAMERA_SERIAL>
