#######
Cameras
#######

Compatibility
-------------

All `The Imaging Source` cameras can be addressed under Linux.
Some legacy hardware is not supported by the tiscamera libraries.

:ref:`supported devices<supported_devices>`



===============
Firmware Update
===============

In the event a firmware update is required, take the following steps:

Prerequisites
-------------

The following tools are required for a firmware update:

.. tabs::

   .. group-tab:: Usb-2.0

      ``tcam-firmware-update``

      tcam-firmware-update is not part of a standard tiscamera installation.
      You can find it here: https://github.com/theimagingsource/tcam-firmware-update

   .. group-tab:: Usb-3.X

      ``tcam-firmware-update``

      tcam-firmware-update is not part of a standard tiscamera installation.
      You can find it here: https://github.com/theimagingsource/tcam-firmware-update
                  
   .. group-tab:: GigE

      ``tcam-gigetool``

      tcam-gigetool is part of a standard tiscamera installation when GigE support is enabled.


Identifying the Camera
----------------------

To identify the available devices, execute the following command in a terminal:

.. tabs::

   .. group-tab:: Usb-2.0

      .. code-block:: sh

         tcam-firmware-update -l

   .. group-tab:: Usb-3.X

      .. code-block:: sh
                   
         tcam-firmware-update -l
      
   .. group-tab:: GigE

      .. code-block:: sh

         tcam-gigetool list


Locate the Firmware
-------------------

.. tabs::

   .. group-tab:: Usb-2.0

      Go to the directory `<tcam-firmware-update>/firmware/usb2/`.
      
      Locate the latest firmware for the camera.

   .. group-tab:: Usb-3.X

      Please :ref:`contact the Imaging Source <contact>` to receive the necessary files.

   .. group-tab:: GigE

      Please :ref:`contact the Imaging Source <contact>` to receive the necessary files.

Writing the Firmware
--------------------

.. tabs::

   .. group-tab:: Usb-2.0

      .. code-block:: sh

         sudo tcam-firmware-update -u -d <SERIAL> -f <path to firmware file>

   .. group-tab:: Usb-3.X

      .. code-block:: sh
                   
         sudo tcam-firmware-update -u -d <SERIAL> -f <path to firmware file>
               

   .. group-tab:: GigE
      
      .. code-block:: sh

         tcam-gigetool upload --serial <SERIAL> firmware=<path to firmware file>
      

            
