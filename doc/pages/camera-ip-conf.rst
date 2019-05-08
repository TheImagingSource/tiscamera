
.. _camera_ip_conf:

##############
camera-ip-conf
##############

camera-ip-conf is a tool that allows the ip configuration of GigE network cameras.
Use it to configure the camera through the command line.


Cameras can be named using the following methods:

.. code-block:: text

   -s  --serial - The serial number of the camera
   -m  --mac    - The mac of the camera
   -n  --name   - The user defined name (assuming the name is not empty)


Arguments
---------

.. program:: camera-ip-conf

.. option:: -h, --help

   Print a help message

.. option:: -l, list

   List available GigE cameras.

.. option:: -i, info (-s SERIAL|-n NAME|-m MAC)

   Print information about a certain camera.

.. option:: set [ip=IP] [subnet=NETMASK] [gateway=GATEWAY] [dhcp={on,off}] [static={on,off}] [name=NAME] (-s SERIAL|-n NAME|-m MAC)

   Configure one or more settings in the camera.
            
   .. program:: camera-ip-conf-set

   .. option:: ip=IP

      Option to set static IP settings IP.
               
   .. option:: subnet=NETMASK

      Option to set static IP settings netmask.

   .. option:: gateway=GATEWAY

      Option to set static IP settings gateway.
               
   .. option:: dhcp={on,off}

      Toggle for usage of dynamic IP address settings.
               
   .. option:: static={on,off}

      Toggle for usage of static IP address settings.
               
   .. option:: name=NAME

      The maximum length of this name is 15 characters.
               
.. option:: rescue [ip=IP] [subnet=NETMASK] [gateway=GATEWAY] (-s SERIAL|-m MAC)

   Temporarily assign another address to a camera

   .. program:: camera-ip-conf-rescue

   .. option:: ip=IP

      IP address that shall be temporarily assigned.

   .. option:: subnet=NETMASK

      Netmask address that shall be temporarily assigned.
               
   .. option:: gateway=GATEWAY

      Gateway address that shall be temporarily assigned.

.. option:: upload firmware=FILE -s SERIAL

   Upload a firmware file to a camera. 
            
   .. option:: firmware=FILE

      .fw or .fwpack file that shall be used.

Examples
--------

To list all cameras:

.. code-block:: sh

   camera-ip-conf -l

To get information about a single camera:

.. code-block:: sh

   camera-ip-conf -i -s <CAMERA_SERIAL>

To set the name of a camera:

.. code-block:: sh

   camera-ip-conf set name="CAMERA_NAME" -s <CAMERA_SERIAL>

This name is restricted to 15 characters.

To delete the name, simply set it to an empty name:

.. code-block:: sh

   camera-ip-conf set name="" -s <CAMERA_SERIAL>

To set the static ip configuration of a camera:

.. code-block:: sh

   camera-ip-conf set ip=192.168.100.100 subnet=255.255.0.0 gateway=192.168.100.1

To toggle static ip usage:

.. code-block:: sh

   camera-ip-conf set static=on -s <CAMERA_SERIAL>

To upload a new firmware:

.. code-block:: sh

   camera-ip-conf upload firmware=<FILE> -s <CAMERA_SERIAL>

.. warning::
   
   **CAUTION: Failures when upgrading the firmware can make the camera unusable. Use at your own risk!**


Troubleshooting
===============

- The camera is not detected.
  There might be several issues to consider:
  
  - Ensure the camera is responding as it should.
    This can be done via wireshark. If a gvcp pong arrives, the
    camera is responding correctly.
  - Turn off the firewall.
    The firewall might be blocking the packages.
  - Turn off rp_filter.
    rp_filter is a kernel module that drops packets from addresses
    that are not within the address range of the configured network.
    If the camera happens to use LLA (or a static IP not within
    the configured network). This may be the reason.
    To temporarily turn off the rp_filter, execute:
    
    .. code-block:: sh

       sudo sysctl -w net.ipv4.conf.all.rp_filter=0

    .. warning::
       **WARNING:** These actions may pose a security threat to the
       computer. Please contact the system administrator before executing these actions.
       We take no responsibility for damage to persons, systems or environments.
       Use at your own risk.

  - The camera is misconfigured.
    If the camera has a temporary IP configuration and is not
    reachable, simply reconnecting the camera should reset the
    configuration.

    If the static ip is misconfigured and the camera has a address
    that can not be represented in a reliable way (e.g. broadcast
    addresses such as 192.168.0.255/24), execute
  
    .. code-block:: sh

       camera-ip-conf rescue ip=<IP> subnet=<SUBNET> gateway=<GATEWAY> mac=<MAC>

  To get the mac address of the camera, use tools like wireshark
  to listen to incoming traffic. The camera should still send
  pong packets as a response to the discovery pings.
