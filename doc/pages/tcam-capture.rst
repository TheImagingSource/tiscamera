.. _tcam_capture:

############
tcam-capture
############

| tcam-capture is the general purpose image retrieval application of The Imaging Source under Linux.  
| It is considered an application for debugging/testing purposes.

==================
Keyboard Shortcuts
==================

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
           
=====================
Commandline Arguments
=====================

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

=======
Dialogs
=======

The following is a description of all dialog windows tcam-capture may open.

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

Versions
--------

Lists version information about tiscamera and other `The Imaging Source` packages.

tiscamera will only be listed as installed when installed as a debian package.

State
-----

The state tab will display the current json property description.

Clicking `Reset` will update the the description.

Clicking `Apply` will apply the string to the tcambin.


Options
=======

Conversion Element
------------------

**Default**: Auto

Selector for the tcambin property `conversion-element`.

See :ref:`tcambin properties <tcambin_properties>` for details.
   
======
Config
======

tcam-capture has a config directory that can be found at
`$XDG_CACHE_DIR/the_imaging_source/`.

The default is: `~/.cache/the_imaging_source/`
