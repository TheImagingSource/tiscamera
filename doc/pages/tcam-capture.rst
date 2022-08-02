.. _tcam_capture:

############
tcam-capture
############

| tcam-capture is the general purpose image retrieval application of The Imaging Source under Linux.  
| It is considered an application for debugging/testing purposes.

******************
Keyboard Shortcuts
******************

The following is a complete list of shortcuts tcam-capture offers.

.. list-table:: keyboard shortcuts
   :header-rows: 1
   :widths: 25 75

   * - Shortcut
     - Action

   * - Ctrl-Shift-Q
     - Quit tcam-capture
   * - Ctrl-U
     - Open format dialog
       Requires a device to already be open.
   * - Ctrl-I
     - Open info dialog
   * - Ctrl-O
     - Open device dialog
   * - Ctrl-P
     - Open property dialog
       Requires a device to be already open.
           
*********************
Commandline Arguments
*********************

tcam-capture has several optional arguments to change its behavior:

.. option:: -h, --help

   Show this help message and exit.

.. option:: --help-all

   Display Qt specific options.
   
.. option:: --serial <serial>

   Open device with serial immediately.

.. option:: -v, --version

   Display version information.

Additionally GStreamer arguments can be passed to retrieve debug information about the streams.
The following GStreamer commandline arguments are currently supported:

.. code-block:: text

    --gst-debug
    --gst-debug-level
    --gst-debug-no-color

For more information concerning gstreamer debugging/logging, see :ref:`logging`

*******
Dialogs
*******

The following is a description of all dialog windows tcam-capture may open.

=============
Device Dialog
=============

The device dialog will list all `The Imaging Source` devices that are supported by tiscamera.

To see which devices are supported by tiscamera, see :ref:`supported devices<supported_devices>`.

Property Dialog
===============

The property dialog allows interaction with all device properties.

Properties on the same tab are refreshed when:

- The active tab is switched
- The `Refresh` button is clicked
- The user presses `F5`.
- A property is changed

Caps Dialog
===========

The caps dialog allows a full configuration of the used device format.


Info
====

Stream
------

Displays information about the current stream. These include:

- Pipeline: The GStreamer pipeline used by tcam-capture.
- Device caps- The GStreamer caps given to the source element by tcam-capture.
- GstMeta - TcamMetaStatistics for the current stream.

Versions
--------

Lists version information about tiscamera and other `The Imaging Source` packages.

tiscamera will only be listed as installed when installed as a debian package.

State
-----

The state tab will display the current json property description.

Clicking `Reset` will update the the description.

Clicking `Apply` will apply the string to the tcambin.

*******
Options
*******

=======
General
=======

Conversion Element
==================

**Default**: Auto

Selector for the tcambin property `conversion-element`.

See :ref:`tcambin properties <tcambin_properties>` for details.

============
Image Saving
============

These properties are used to configure 

Image Type
==========

**Default**: bmp

Type in which images shall be saved.

Possible values are:

- BMP - Windows Bitmap 
- GIF - Graphic Interchange Format
- JPG - Joint Photographic Experts Group 
- JPEG - Joint Photographic Experts Group
- PNG - Portable Network Graphics 
- PBM - Portable Bitmap Read
- PGM - Portable Graymap Read
- PPM - Portable Pixmap 
- XBM - X11 Bitmap 
- XPM - X11 Pixmap

Image Save Location
===================

**Default**: /tmp/

Directory to which images shall be saved.

Image Filename Structure
========================

**Default**: tcam-capture-{serial}-{caps}-{timestamp}.{extension}

| The filename is a generic string that has to be a legal filename.
| The following strings will be replaced by tcam-capture:

.. list-table:: image filename replacements
   :header-rows: 1
   :widths: 25 75

   * - string
     - substitute
   * - {serial}
     - long serial of the device in use
   * - {caps}
     - simplified caps string
       Will be filename friendly and as short as possible
   * - {timestamp}
     - Timestamp in the format yyyyMMddthhmmss_zzz
   * - {extension}
     - filename extension compatible with the selected image type

============
Video Saving
============

Video Type
==========

**Default**: h264

| h264 will be saved as mp4.
| mjpeg will be saved as avi.

Video Save Location
===================

**Default**: /tmp/

Directory to which video shall be saved.

Video Filename Structure
========================

**Default**: tcam-capture-{serial}-{caps}-{timestamp}.{extension}

| The filename is a generic string that has to be a legal filename.
| The following strings will be replaced by tcam-capture:

.. list-table:: video filename replacements
   :header-rows: 1
   :widths: 25 75

   * - string
     - substitute
   * - {serial}
     - long serial of the device in use
   * - {caps}
     - simplified caps string
       Will be filename friendly and as short as possible
   * - {timestamp}
     - Timestamp in the format yyyyMMddthhmmss_zzz
   * - {extension}
     - filename extension compatible with the selected video type

******************
Configuration File
******************

tcam-capture has a config directory that can be found at
`$XDG_CACHE_DIR/the_imaging_source/`.

The default is: `~/.cache/the_imaging_source/`
