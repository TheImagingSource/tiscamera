.. _tcam_uvc_extension_loader:

#########################
tcam-uvc-extension-loader
#########################

The tcam-uvc-extension-loader is our internal replacement for uvcdynctrl.

Its only purpose is to load UVC extension units for USB cameras.

The description files for the extension units are written in json (with comments allowed) and are a
direct representation of the underlying struct that is submitted to the kernel.

For a description of the UVC driver mapping command, please refer to the `kernel
documentation <https://www.kernel.org/doc/html/latest/media/v4l-drivers/uvcvideo.html#uvcioc-ctrl-map-map-a-uvc-control-to-a-v4l2-control>`_.

Examples
========

The examples expect a default installation.
If any paths have been changed some things may have to adjusted.

.. code-block:: sh

    # 
    # /dev/v4l/by-id contains softlinks to the actual /dev/video0
    tcam-uvc-extension-loader -f /usr/share/theimagingsource/tiscamera/uvc-extension/usb<X>.json -d /dev/v4l/by-id/usb-The_Imaging_Source_Europe_GmbH_<camera ident>-video-index0

Arguments
=========

tcam-uvc-extension-loader has the following arguments:

.. code-block:: text

    -h, --help    Show help message and exit
    -d, --device  Device that shall receive the extension unit. (Default: /dev/video0)
    -f, --file    Extension unit file that shall be loaded (This flag is required)

Plug & Play
===========

With a default installation, `tcam-uvc-extension-loader` is automatically called when a
compatible device is attached. This is done via :ref:`UDev rules <udev>`.
    
What extension to use?
======================

Currently the following associations are used for automatic loading.
To see the association of a specific camera execute `lsusb` and use the the second part of the ID (199e:XXXX).

.. list-table:: PID - json association
   :header-rows: 1

   * - Device PID
     - Extension File
   * - 82XX
     - usb2.json
   * - 83XX
     - usb2.json
   * - 84XX
     - usb23.json
   * - 85XX
     - usb23.json
   * - 86XX
     - usb23.json
   * - 87XX
     - usb23.json
   * - 90XX
     - usb33.json
   * - 94XX
     - usb37.json
   * - 98XX
     - usb33.json


Extension Unit
==============

The following description is a generic example showing
what a file that is loadable by this program must look like.

C-Style comments are allowed.

.. code-block:: text

    {
        // execute `lsusb -vd <vid>:<pid>`
        // and search for `guidExtensionCode`
        // the guid from that field has to be entered here
        "guid": "0aba49de-5c0b-49d5-8f71-0be40f94a67a",

        // the list of mappings
        "mappings": [
            {
                // string containing the u32 id number that V4L2
                // will give when querying the device
                "id": "0x199e9999",
                // String containing the name. Maximum of 31 characters
                "name", "My property",
                // UVC control selector
                "selector": "0xB1",
                // type the UVC property has.
                // possible values are: unsigned, signed, raw, enum, booean, bitmask
                "uvc_type": "",
                // type the V4L2 property has.
                // possible values are: bitmask, boolean, button, integer, menu, string
                v4l2_type: "",
                // size of the UVC property in bit
                size_bits: 8,
                // size of the offset in bit.
                // It is possible to map multiple V4L2 onto the same UVC property field.
                // E.g. this offset allows access to single bit in a bitfield.
                offset_bits 0,

                // entries is an optional field.
                // It is a list of menu entries
                // That shall be mapped
                entries: [
                    {
                        // integer value the V4L2 property shall set in UVC upon selection.
                        value: 999,
                        // String for the entry. Maximum of 31 characters
                        "entry": "My Menu Entry"
                    }
                ]
            }
        ]
    }

Exit status
===========

The following return states are possible:

.. code-block:: text

   0   - Normal operation, everthing went ok.
   1   - Device was not found.
   3   - File for extension unit could not be loaded.
   106 - A required argument is missing.
