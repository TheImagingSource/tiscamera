
.. _camera_ip_conf:

##############
camera-ip-conf
##############

camera-ip-conf is a tool that allows the ip configuration of GigE network cameras.
It offers the possibility to configure via cli or gui.

Examples
--------

To list all cameras:

.. code-block:: sh

   camera-ip-conf -l

Cameras can be named using the following methods:

.. code-block:: text

   -s  --serial - The serial number of the camera
   -m  --mac    - The mac of the camera
   -n  --name   - The user defined name (assuming the name is not empty)


To get information about a single camera:

.. code-block:: sh

   camera-ip-conf -i -s <CAMERA_SERIAL>

To set the name of a camera:

.. code-block:: sh

   camera-ip-conf set name="CAMERA_NAME" -s <CAMERA_SERIAL>

This name is restricted to 15 characters.

To delete the name simply set it to an empty name

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
   
   **CAUTION: This can break the camera. Use at own risk!**


Troubleshooting
===============

- The camera is not detected.
  If the camera is not detected, there are various possible reasons.

  - Assure the camera responds as expected.
    This can be done via wireshark. If an gvcp pong arrives, the
    camera responds correctly.
  - Turn of the firewall.
    It could be filtered
  - Turn of rp_filter
    rp_filter is a kernel module that drops packets from addresses
    that are not within the address range of the configured network.
    If the camera happens to use LLA or a static IP not within
    the configured network. This may be the reason.
    To temporarily turn off the rp_filter execute:
    
    .. code-block:: sh

       sudo sysctl -w net.ipv4.conf.all.rp_filter=0

    .. warning::
       **WARNING:** These actions may pose a security threat to the
       computer. If there is a professional administrator at your
       facility, please contact this person to assure the working
       environment is not endangered by these actions.
       We do not take responsibility for any damage to you, your
       system or your environment caused by these actions.
       Usage at own risk.

  - The camera is misconfigured.
    If the camera has a temporary IP configuration and is not
    reachable a simple reconnecting of the camera should reset the
    configuration.

    If the static ip is misconfigured and the camera has a address
    that can not be represented in a reliable way (e.g. broadcast
    addresses such as 192.168.0.255/24), execute
  
    .. code-block:: sh

       camera-ip-conf rescue ip=<IP> subnet=<SUBNET> gateway=<GATEWAY> mac=<MAC>

  To get the mac address of the camera use tools like wireshark
  to listen to incoming traffic. The camera should still send
  pong packets as a response to the discovery pings.
