###
UVC
###

.. _uvc_extensions:

===============
Extension Units
===============

UVC extension units are applied through udev rules.

The extension units are json files that describe how certain device interactions
should be mapped to UVC and V4L2 properties.
Most USB cameras require these units for full functionality.

The default installation path is: /usr/share/theimagingsource/tiscamera/uvc-extension

For more information on how the UVC extension units are loaded, read :any:`tcam-uvc-extension-loader`.

===========
UVC Logging
===========

To enable logging:

.. code-block:: sh

   sudo sh -c 'echo 0xffffffff > /sys/module/uvcvideo/parameters/trace'

To disable logging:
   
.. code-block:: sh

   sudo sh -c 'echo 0x0 > /sys/module/uvcvideo/parameters/trace'

   
The logging output can be found in the system log.

