.. _tcam_capture:

############
tcam-capture
############

tcam-capture is the general purpose image retrieval application of The Imaging Source under Linux.

.. image:: ../images/tcam-capture-running.png

=====================
Commandline Arguments
=====================

tcam-capture has several optional arguments to change its behavior:

.. option:: -h, --help

   Show this help message and exit
   
.. option:: --serial SERIAL

   Open device with serial immediately.

.. option:: --format CAPS_STR

   Open device with this gstreamer caps.

.. option:: --verbose, -v

   Increase logging level. Maximum is 5.

.. option:: --reset

   Reset application settings and clear cache.
   This deletes all user settings.

.. option:: --fullscreen

   Start the application in fullscreen mode.


Additionally GStreamer arguments can be passed to retrieve debug information about the streams.
The following GStreamer commandline arguments are currently supported:

.. code-block:: text

    --gst-debug
    --gst-debug-level
    --gst-debug-no-color

For more information concerning gstreamer debugging/logging, see :ref:`logging`

===========
Preferences
===========

tcam-capture offers several options to change its behavior.
To open the options dialog, press the `Preferences` button in the toolbar.
The configuration file can be found under `$XDG_CONFIG_DIR/tcam-capture.conf`.

**Default** : `~/.config/tcam-capture.conf`

Image/Video
===========

.. image:: ../images/tcam-capture-options-saving.png
   :width: 400
   :align: right

Save Location
-------------

Default: /tmp

Folder in which images/videos will be saved.

Image Type
----------

Image encoding that will be used when saving images.

_Default_: png

Video Type
----------

_Default_: avi

Video encoding that will be used when saving videos.

Naming Options
--------------

The available options are identical for images and videos.

:User Prefix:
   Random string defined by the user that is prepended to the
   file name. The maximum length is 100 characters.
   Default: Empty
:Include Serial:
   Adds the serial number of the used device to the filename.
   Default: True
:Include Format:  Include a simple format description.
                  This description contains all information concerning the currently used device caps.
                  The string will have the format:
                  ``format_widthxheight_framerate-numerator_framerate-denominator``.
                  To ensure the file can be saved, characters like '/' are replaced with underscores.
                  Default: True
:Include Counter:  Include a unique counter in the filename. If the
                   application is restarted, the counter will pickup where it left off, assuming all
                   other parts of the name remain identical.
                   Default: True
:Counter Size:  Padding size the counter shall have
                Maximum: 10 digits
                Default: 5 digits
:Include Timestamp:  Include a timestamp with local time in the
                     filename. The timestamp will be in ISO format i.e. YYYYmmddTHHMMSS.
                     When both timestamp and counter are active, the counter
                     will be reset once the timestamp changes.
                     Default: True


General
=======

.. image:: ../images/tcam-capture-options-general.png
   :align: right
   :width: 400

Show Device Dialog On Startup
    Whether or not to show the device selection dialog on startup
    will be ignored when a device is reopened.
    Default: True

Reopen Device On Startup:
  If a device was open during the last application shutdown, tcam-capture will
  automatically try to reopen the device. If the device does not exist, it will
  fall back to its default behavior.
  Default: True

Use Dutils:
  A toggle to disable the usage of tiscamera-dutils.
  The package tiscamera-dutils will have to be installed for this to be enabled.
  Default: True

Apply cached properties:
  When closing a device tcam-capture saves a snapshot of all properties and their current value.
  These will be written into the device when it is opened the next time.
  The files can be found in the tcam-capture cache directory.
  When disabled the device will be opened 'as is' and tcam-capture will not touch any properties.
  
=======
Caching
=======

tcam-capture has a cache directory that can be found at
`$XDG_CACHE_DIR/tcam-capture/`.

The default is: `~/.cache/tcam-capture/`
