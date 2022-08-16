.. _supported_devices:

#################
Supported devices
#################


All `The Imaging Source` cameras can be addressed under Linux.  
Some legacy hardware is not supported by the tiscamera libraries.  
Exceptions to these rules will be explicitly stated in the table.

.. list-table:: compatability list
   :header-rows: 1

   * - Type
     - Linux I/O
     - tiscamera <= 0.14
     - tiscamera >= 1.0
   * - 21, 31, 41
     - yes
     - no
     - no
   * - 22, 42, 72
     - yes
     - yes
     - yes
   * - 52
     - yes
     - no
     - yes [#]_
   * - 23
     - yes
     - yes
     - no
   * - 25
     - yes
     - yes
     - yes
   * - 33
     - yes
     - yes
     - yes
   * - 36
     - yes
     - yes
     - yes
   * - 37
     - yes
     - yes
     - yes
   * - 38
     - yes
     - yes
     - yes
   * - Firewire
     - yes
     - no
     - no
   * - Converter / Grabber
     - yes
     - no
     - no
   * - HDMI-to-USB converter
     - yes
     - yes
     - yes
   * - AFU 050
     - no
     - yes
     - yes
   * - AFU 130
     - yes
     - yes
     - no
   * - AFU 420
     - no
     - yes
     - yes
   * - 73 USB
     - no
     - yes
     - no
   * - Zoom Cameras
     - yes
     - yes
     - yes
       

Your camera is not listed or you are still unsure?
:ref:`Contact our support<contact>`, they will gladly help you.

**Linux I/O** means that device interaction are possible either through
standard Linux kernel drivers or open source third party libraries such as aravis.

.. [#] Supported since 1.1
