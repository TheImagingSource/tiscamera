.. _tcam_ctrl:

#########
tcam-ctrl
#########

tcam-ctrl is a commandline tool to allow inspection and configuration of devices.

Arguments
=========

.. option:: -h, --help

   Print available options.

.. option:: -l

   Print available cameras.

   .. code-block:: text
      :caption: Sample Output
      :name: Sample Output

      Available devices:
      Model             Type        Serial

      DFK 72UB02        V4L2        37410696
      DFK 39GX265-Z20   Aravis      12919949
      DFK AFU050-L34    LibUsb      04614259

.. option:: -p <SERIAL>

   Print available properties.
   *Requires the serial number of the camera to be queried.*

.. option:: -f <SERIAL>

   Print information about available video formats.
   *Requires the serial number of the camera to be queried.*
            
.. option:: -c <SERIAL>

   Print GStreamer 1.0 caps the device offers.
   *Requires the serial number of the camera to be queried.*
   
   These caps are offered directly by the device.
   Implicitly converted caps offered by the tcambin through conversion are not included.

.. option:: --version

   Print version information about the used tiscamera and aravis versions.

   Print information about the library versions of tiscamera and aravis.

