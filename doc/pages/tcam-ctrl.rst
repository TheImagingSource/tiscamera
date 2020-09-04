.. _tcam_ctrl:

#########
tcam-ctrl
#########

tcam-ctrl is a commandline tool to allow inspection and configuration of devices.

Arguments
=========

.. option:: -h, --help

   Print available options.

.. option:: -l, --list

   Print available cameras.

   .. code-block:: text
      :caption: Sample Output
      :name: Sample Output

      Available devices:
      Model             Type        Serial

      DFK 72UB02        V4L2        37410696
      DFK 39GX265-Z20   Aravis      12919949
      DFK AFU050-L34    LibUsb      04614259

.. option:: -p, --properties <SERIAL>

   Print available properties.

   *Requires the serial number of the camera to be queried.*

.. option:: -f, --format <SERIAL>

   Print information about available video formats.

   *Requires the serial number of the camera to be queried.*
            
.. option:: -c, --caps <SERIAL>

   Print GStreamer 1.0 caps the device offers.

   *Requires the serial number of the camera to be queried.*
   
   These caps are offered directly by the device.
   Implicitly converted caps offered by the tcambin through conversion are not included.

.. option:: --version

   Print version information about the used tiscamera and aravis versions.

   Print information about the library versions of tiscamera and aravis.

.. option:: -t,--type {aravis,v4l2,libusb,unknown}

   Device type that shall be used.
   When a device offers multiple backends,
   this flag allows the user to choose the appropriate backend.
   Defaults to 'unknown', which causes the first device with matching serial to be choosen.

.. option:: --save <SERIAL>

   Prints a JSON description of the device properties and their values.

   *Requires the serial number of the camera to be queried.*

.. option:: --load <SERIAL> <JSON>

   Load the JSON string and set the properties to the specified values.

   See :any:`state` for a JSON description.

   .. code-block:: sh

      gst-launch-1.0 tcamsrc serial=12341234 state='{\"Exposure\":3000,"Exposure\ Auto\":false}' ! ....

   Alternatively a file path to a file containing the JSON description can be used.
