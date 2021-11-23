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

      ``firmware-update``

      firmware-update is not part of a standard tiscamera installation.
      To get the tool you have to manually compile the tiscamera repository.
      firmware-update can then be found in the build directory under `tools/firmware-update/`.

   .. group-tab:: Usb-3.X

      ``firmware-update``

      firmware-update is not part of a standard tiscamera installation.
      To get the tool you have to manually compile the tiscamera repository.
      firmware-update can then be found in the build directory under `tools/firmware-update/`.
                  
   .. group-tab:: GigE

      ``camera-ip-conf``

      camera-ip-conf is part of a standard tiscamera installation when aravis support is enabled.


Identifying the Camera
----------------------

To identify the available devices, execute the following command in a terminal:

.. tabs::

   .. group-tab:: Usb-2.0

      .. code-block:: sh

         sudo firmware-update -l

   .. group-tab:: Usb-3.X

      .. code-block:: sh
                   
         sudo firmware-update -l
      
   .. group-tab:: GigE

      .. code-block:: sh

         camera-ip-conf -l


Locate the Firmware
-------------------

.. tabs::

   .. group-tab:: Usb-2.0

      Go to the directory `<tiscamera>/data/firmware/usb2/`.
      
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

         cd <tiscamera-build-dir>/tool/firmware-update/
         sudo .firmware-update -u -d <SERIAL> -f <path to firmware file>


   .. group-tab:: Usb-3.X

      .. code-block:: sh
                   
         cd <tiscamera-build-dir>/tool/firmware-update/
         sudo .firmware-update -u -d <SERIAL> -f <path to firmware file>
               

   .. group-tab:: GigE
      
      .. code-block:: sh

         camera-ip-conf upload --serial <SERIAL> firmware=<path to firmware file>
      

            
