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
   Requires the serial number of the camera that shall be queried.

.. option:: -f <SERIAL>
.. option:: -c <SERIAL>

   Print GStreamer 1.0 caps the device offers.
   Requires the serial number of the camera that shall be queried.
   These caps are caps the device directly offers. Implicitly converted caps
   that are additionally offered by the tcambin through
   conversion are not included.

.. option:: --version

   Print version information about the used tiscamera and aravis versions.

