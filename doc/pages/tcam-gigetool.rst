.. _tcam_gigetool:

#############
tcam-gigetool
#############

Network camera configuration tool.
Allows IP configuration, firmware upload, etc.

Arguments
---------

The following arguments may be required by a command:

:IDENTIFIER: Unique identifier of the camera: serial number, MAC or IP address
:FILENAME:   Filename of firmware file to upload

.. program:: gigetool

.. option:: -h, --help

   Show a help message

.. option:: list

   List connected cameras.

   .. program:: gigetool-list
   
   .. option:: --format

      Format of the device-specific information that will be printed.
      
      Default is *msui*.

      .. list-table:: list format options
         :header-rows: 1
         :widths: 10 90

         * - Option
           - Description
         * - | m
           - Name of the camera model
         * - s
           - Serial number of the camera
         * - u
           - User defined name, may be empty
         * - i
           - Current ip
         * - n
           - Current netmask
         * - g
           - Current gateway
         * - I
           - Persistent IP, may be empty
         * - N
           - Persistent netmask, may be empty
         * - G
           - Persistent gateway, may be empty
         * - f
           - Interface name, network interface that was used to reach the camera
         * - d
           - Yes/No field, stating if dhcp is enabled
         * - S
           - Yes/No field, stating if static ip settings are enabled
         * - M
           - MAC address of the camera
         * - r
           - Yes/No field saying if the camera is reachable for streaming

.. option:: info IDENTIFIER
            
   Show details of a camera.

   Possible identifier would be the camera's serial number or its MAC address.
   
.. option:: set [--ip IP] [--netmask NETMASK] [--gateway GATEWAY] [--mode {dhcp,static,linklocal}] [--name NAME] IDENTIFIER

   Permanently set configuration options on the camera

   .. program:: gigetool-set
            
   .. option:: --ip IP

      IP address to be set.
      
   .. option:: --netmask NETMASK

      Netmask to be set.

   .. option:: --gateway GATEWAY

      Gateway address to be set.
      
   .. option:: --mode {dhcp,static,linklocal}
               
      IP configuration mode to be set.
      
   .. option:: --name NAME

      Set a user-defined name. This is restricted to 15 characters or less.


.. option:: rescue --ip IP --netmask NETMASK --gateway GATEWAY IDENTIFIER


   Temporarily set IP configuration on the camera.

   .. program:: gigetool-rescue

   .. option:: --ip IP

      temporary IP address to be assigned
            
   .. option:: --netmask NETMASK

      temporary netmask to be assigned
            
   .. option:: --gateway GATEWAY

      temporary gateway address to be assigned

   
.. option:: upload IDENTIFIER FILENAME

   Upload a firmware file to the camera.
   
.. option:: batchupload [-n] [-b baseadress] INTERFACE FILENAME

   Upload a firmware file to all cameras connected to a
   network interface

   .. program:: gigetool-batchupload
   
   .. option:: -n, --noconfigure

      do not auto-configure IP addresses before upload
               
   .. option:: -b BASEADDRESS, --baseaddress BASEADDRESS
               
      lowest IP address to use for auto-configuration
      (default=x.x.x.10)

      
.. option:: check-control IDENTIFIER

   Checks if given camera is currently in use.
   Prints IP and port that currently controls the camera.

