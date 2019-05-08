.. _firmware_update:

###############
firmware-update
###############

firmware-update is a commandline tool to update the firmware of The Imaging Source USB cameras.

This tool is rarely needed and, therefore not installed alongside other tools.

Examples
========

List all available cameras:

.. code-block:: sh
                
   firmware-update -l


Get information about a single camera:

.. code-block:: sh
                
   firmware-update -id <serial number>


Apply firmware:

.. code-block:: sh
                
   firmware-update -ud <serial number> -f firmware-file


Switch camera to UVC/proprietary mode:

.. code-block:: sh

   firmware-update -d <serialnumber> -m uvc
   firmware-update -d <serialnumber> -m proprietary


Switching to UVC mode is only necessary for USB-2.0 cameras.

Firmware Files
==============

For USB-3.X firmware files, please send a request.

http://www.theimagingsource.com/en_US/company/contact/

Firmware for our USB-2.0 cameras can be found in our Linux repository in data/firmware.

