########
Backends
########

This page describes the different backends that are used to interact with cameras.

V4L2
####

UVC compatible cameras can be addressed through the Video4Linux2(V4L2) kernel API.

To know which device is associated with an /dev/videoX node, read the contents of the
directory `/dev/v4l/by-id/`. This directory contains softlinks containing the full
camera name and serial pointing to the associated dev-node.

The source code for this backend can be found in *src/v4l2/*

For further reading, :any:`check out the documentation or mailing list<reading_v4l2>`.

v4l-utils
^^^^^^^^^^

`v4l-utils` is a package that contains helper tools for V4L2 device interaction.
These tools allow direct interaction with the V4L2 device.

To check the available formats of a camera:

.. code-block:: sh
                
   v4l2-ctl --list-formats-ext -d /dev/video0

Sample output:

.. code-block:: text

   ioctl: VIDIOC_ENUM_FMT
   Index       : 0
   Type        : Video Capture
   Pixel Format: 'GRBG'
   Name        : GRBG Bayer (GRBG)
           Size: Discrete 2592x1944
                   Interval: Discrete 0.133s (7.500 fps)
                   Interval: Discrete 0.143s (7.000 fps)
                   Interval: Discrete 0.200s (5.000 fps)
                   Interval: Discrete 0.250s (4.000 fps)
           Size: Discrete 1920x1080
                   Interval: Discrete 0.067s (15.000 fps)
                   Interval: Discrete 0.100s (10.000 fps)
                   Interval: Discrete 0.133s (7.500 fps)
                   Interval: Discrete 0.200s (5.000 fps)
           Size: Discrete 1280x720
                   Interval: Discrete 0.033s (30.000 fps)
                   Interval: Discrete 0.040s (25.000 fps)
                   Interval: Discrete 0.067s (15.000 fps)
                   Interval: Discrete 0.100s (10.000 fps)
           Size: Discrete 640x480
                   Interval: Discrete 0.017s (60.000 fps)
                   Interval: Discrete 0.033s (30.000 fps)
                   Interval: Discrete 0.040s (25.000 fps)
                   Interval: Discrete 0.067s (15.000 fps)

To list the currently available properties:

.. code-block:: sh

   v4l2-ctl -L -d /dev/video0

.. code-block:: sh

                gain 0x00980913 (int)    : min=144 max=1200 step=1 default=144 value=144
       exposure_auto 0x009a0901 (menu)   : min=0 max=3 default=3 value=3
                     1: Manual Mode
                     3: Aperture Priority Mode

   exposure_absolute 0x009a0902 (int)    : min=1 max=50000 step=1 default=333 value=333
    exposure_time_us 0x0199e201 (int)    : min=10 max=5000000 step=1 default=33333 value=33333
         gain_db_100 0x0199e204 (int)    : min=0 max=921 step=1 default=0 value=0
        trigger_mode 0x0199e208 (bool)   : default=0 value=0
    software_trigger 0x0199e209 (button) : flags=write-only
       trigger_delay 0x0199e210 (int)    : min=0 max=10000000 step=10 default=0 value=150
       strobe_enable 0x0199e211 (bool)   : default=0 value=0
     strobe_polarity 0x0199e212 (bool)   : default=0 value=0
     strobe_exposure 0x0199e213 (bool)   : default=0 value=0
               gpout 0x0199e216 (bool)   : default=0 value=0
                gpin 0x0199e217 (bool)   : default=0 value=0
        roi_offset_x 0x0199e218 (int)    : min=0 max=1024 step=16 default=0 value=0
        roi_offset_y 0x0199e219 (int)    : min=0 max=1008 step=16 default=0 value=0
     roi_auto_center 0x0199e220 (bool)   : default=1 value=1
    trigger_polarity 0x0199e234 (bool)   : default=0 value=0

The columns can be read as follows:

1. name
2. V4L2 id - unique identifier used for ioctl commands
3. V4L2 property type
4. description of the V4L2 property

For menus, the index/name association is listed beneath the menu property.
    
Aravis
######

Aravis is a user space library which allows interaction with GigE Vision devices.

Since Aravis works in the user space, performance limitations may apply.
To circumvent theses limitations, read :ref:`real_time_threading`.

The source code for this backend can be found in *src/aravis/*

For further reading, :any:`check out the documentation or mailing list<reading_aravis>`.

USB3 Vision
^^^^^^^^^^^

.. note::

   This section is only relevant when using a USB 33, 37 or 38 camera.

In some rare cases platforms do not offer a valid media stack,
thus preventing the usage of USB cameras with a UVC backend.
As a workaround, Aravis can be compiled with `--enable-usb`.

This enables Aravis to iterate USB cameras through libusb.

To enable this set the cmake option `TCAM_ARAVIS_USB_VISION` to `ON`.

A selection of the backend can be done via the `type` properties of tcamsrc and tcambin.
Alternatively it can be appended to the serial.

The backend descriptor for Usb3Vision via aravis is `aravis`.

A valid serial in this context would look like `12345678-aravis`

libusb
######

Cameras that are not UVC compatible and can thus not be addressed via V4L2 have an implementation via libusb-1.0.

The source code for this backend can be found in *src/libusb/*

For further reading, :any:`check out the documentation or mailing list<reading_libusb>`.
