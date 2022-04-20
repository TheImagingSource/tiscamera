.. _tcam_ctrl:

#########
tcam-ctrl
#########

tcam-ctrl is a commandline tool to allow inspection and configuration of devices.

Arguments
=========

.. option:: -h, --help

   Print available options.

.. option:: --version

   Print information about the library versions of tiscamera and aravis
   and the configured tiscamera modules.

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

.. option:: --list-serial-long

   Lists all serial numbers in long form, i.e. `<serial>-<backend>`.

.. option:: --packages

   Lists all packages from `The Imaging Source` that are installed on the system,
   as well as their version.

.. option:: --system-info

   Print system information to stdout.

.. option:: --gige-info

   Print information about network capabilities of the system to stdout.

.. option:: --usb-info

   Print information about usb capabilities of the system to stdout.

.. option:: --all-info

   Convenience wrapper. Calls `--gige-info`, `--usb-info` and `--system-info`.


.. option:: -c, --caps <SERIAL>

   Print GStreamer 1.0 caps the device offers.

   *Requires the serial number of the camera to be queried.*

   These caps are offered directly by the device.
   Implicitly converted caps offered by the tcambin through conversion are not included.

.. option:: -p, --properties <SERIAL>

   Print available properties the device offers.

   *Requires the serial number of the camera to be queried.*

.. option:: --save <SERIAL>

   Print a string to stdout containing a GstStructure description of existing device properties and their current values.

   This string contains escape sequences that allow usage in commandline programs such as tcam-ctrl and gst-launch.
   For a default GstStructure string description use `--no-console`.

   *Requires the serial number of the camera to be queried.*

   .. option:: --no-console

      Optional flag for `--save`.

      Switch from commandline friendly output to unsanitized output.
      This is equivalent to calling `gst_structure_to_string` directly.

.. option:: --load <SERIAL>

   Load properties from a GstStructure like string.

   The string must be correctly escaped to be loadable.

   *Requires the serial number of the camera to be queried.*

.. option:: --save-json <SERIAL>

   Prints a JSON description of the device properties and their values.

   *Requires the serial number of the camera to be queried.*

.. option:: --load-json <SERIAL> <JSON>

   Load the JSON string and set the properties to the specified values.
   Alternatively a file path to a file containing the JSON description can be used.

   See :ref:`tcam-properties-json` for a JSON description.

   .. code-block:: sh

      # load from file
      tcam-ctrl --load-json <SERIAL> <FILEPATH>

      # load string
      tcam-ctrl --load-json <SERIAL> '{\"Exposure\":3000,"Exposure\ Auto\":false}'

.. option:: --transform

   List transformations a GStreamer element offers.
   Without arguments tcamconvert will be queried.
   Without `--in` or `--out` both will be listed.

   .. option:: -e,--element <element>

      GstTransformElement that shall be queried.

      Default is `tcamconvert`.

   .. option:: --in <CAPS>

      List GstCaps that `<element>` can transform `<CAPS>` into.

      .. code-block:: sh

         tcam-ctrl --transform --in video/x-bayer,format=rggb

      output:

      .. code-block:: text

         Probing tcamconvert:
         video/x-bayer,format=rggb;
         video/x-raw,format=BGRx

   .. option:: --out <CAPS>

      List GstCaps that `<element>` can transform into `<CAPS>`.

      .. code-block:: sh

         tcam-ctrl --transform --out video/x-raw,format=BGRx

      output:

      .. code-block:: text

         Probing tcamconvert:
         video/x-raw,format=GRAY8;

         # some output omitted

         video/x-bayer,format=grbg;
         video/x-bayer,format=grbg10;
         video/x-bayer,format=grbg10sp;
         video/x-bayer,format=grbg10m;
         video/x-bayer,format=grbg12;
         video/x-bayer,format=grbg12p;
         video/x-bayer,format=grbg12sp;
         video/x-bayer,format=grbg12m;
         video/x-bayer,format=grbg16
