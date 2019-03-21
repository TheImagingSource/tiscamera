#######
Cameras
#######

===============
Firmware-Update
===============

For the event that a firmware update is required take the following steps:

Identifying the camera
----------------------

To identify the available devices execute the following command in a terminal.

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


Locate the firmware
-------------------

.. tabs::

   .. group-tab:: Usb-2.0

      Go to the directory `<tiscamera>/data/firmware/usb2/`.
      
      Locate the latest firmware for the camera.

   .. group-tab:: Usb-3.X

      Please :any:`contact the Imaging Source <contact>` to receive the necessary files.

   .. group-tab:: GigE

      Please :any:`contact the Imaging Source <contact>` to receive the necessary files.

Writing the firmware
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

         camera-ip-conf --serial <SERIAL> firmware=<path to firmware file>
      

            
