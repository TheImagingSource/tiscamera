###############
Troubleshooting
###############

=======
General
=======

A good starting point is to enable logging.
GStreamer logging and tiscamera logging will most likely be helpful.
See :ref:`logging`.

If in doubt, please :ref:`contact our support<contact>`. We will gladly answer any questions and help with troubleshooting.

===
USB
===

Cstate Handling
===============

cstates are a method of CPU power saving. When dealing with real time processes like video streaming,
aggressive power management can cause latency spikes.

For a good overview/introduction read: https://access.redhat.com/articles/65410

The solution boils down to: Setting kernel arguments in GRUB.
This should be used to verify that the problem really has to do with cstates.
To do this, add

.. code-block:: text
                
   processor.max_cstate=1 idle=poll

to the kernel arguments.

To achieve the same during run time, let this program run. It has to be  manually stopped by pressing `Ctrl-C`.
This will re-enable cstates.

.. code-block:: c

   #include <cstdio>
   #include <errno.h>
   #include <unistd.h>
   #include <cstdlib>
   #include <fcntl.h>
   #include <cstring>
   #include <cstdint>
   // cstate
   // introduction: https://access.redhat.com/articles/65410
   static int pm_qos_fd = -1;
   void start_low_latency(void)
   {
       int32_t target = 0;
       if (pm_qos_fd >= 0)
           return;
       pm_qos_fd = open("/dev/cpu_dma_latency", O_RDWR);
       if (pm_qos_fd < 0) {
           fprintf(stderr, "Failed to open PM QOS file: %s",
           strerror(errno));
           exit(errno);
       }
       write(pm_qos_fd, &target, sizeof(target));
   }
   
   void stop_low_latency(void)
   {
       if (pm_qos_fd >= 0)
           close(pm_qos_fd);
   }
   
   int main (int argc, char *argv[])
   {
       start_low_latency();
       while(true)
       {}
       stop_low_latency();
       return 0;
   }


====
GigE
====

The Camera Is Not Found
=======================

One or more cameras are delivering no/incomplete images
=======================================================

Assuming the computer is not simply having processing problems due to low performance two problems may be the cause:

1. The network card in the computer, or a switch between the computer and the camera, is not gigabit capable.
2. The packet size of the network card, or a switch between the computer and the camera, is too small.

Verifying Gigabit Capabilities
------------------------------

Identify the Network Interface
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Execute the following in the terminal
   
.. code-block:: sh
                
   ip a

Sample ouput:

.. code-block:: text

   1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
       link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
       inet 127.0.0.1/8 scope host lo
          valid_lft forever preferred_lft forever
       inet6 ::1/128 scope host 
          valid_lft forever preferred_lft forever
   2: enp31s0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 9000 qdisc mq state UP group default qlen 1000
       link/ether 70:85:c2:84:10:4f brd ff:ff:ff:ff:ff:ff
       inet 192.168.0.111/24 brd 192.168.0.255 scope global dynamic noprefixroute enp31s0
          valid_lft 26756sec preferred_lft 26756sec
       inet6 fe80::7285:c2ff:fe84:104f/64 scope link 
          valid_lft forever preferred_lft forever
   3: enp37s0: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 9000 qdisc pfifo_fast state DOWN group default qlen 1000
       link/ether 68:05:ca:84:32:19 brd ff:ff:ff:ff:ff:ff
       inet 169.254.100.1/16 brd 169.254.255.255 scope global enp37s0
          valid_lft forever preferred_lft forever

Here the interfaces are named `enp31s0` and `enp37s0`.
Interfaces may also have names like `enp31s0` or `eth0`.

Verify the Speed of the Network Interface
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: sh

   cat /sys/class/net/<INTERFACE_NAME>/speed

Sample output:
   
.. code-block:: text

   1000

If the output is `1000`, the network interface has gigabit.

Checking the Network Interface's MTU Size
-----------------------------------------

Identifying the Network Interface
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Execute the following in the terminal

.. code-block:: sh

   ip a

Sample ouput:

.. code-block:: text

   1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
       link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
       inet 127.0.0.1/8 scope host lo
          valid_lft forever preferred_lft forever
       inet6 ::1/128 scope host 
          valid_lft forever preferred_lft forever
   2: enp31s0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 9000 qdisc mq state UP group default qlen 1000
       link/ether 70:85:c2:84:10:4f brd ff:ff:ff:ff:ff:ff
       inet 192.168.0.111/24 brd 192.168.0.255 scope global dynamic noprefixroute enp31s0
          valid_lft 26756sec preferred_lft 26756sec
       inet6 fe80::7285:c2ff:fe84:104f/64 scope link 
          valid_lft forever preferred_lft forever
   3: enp37s0: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 9000 qdisc pfifo_fast state DOWN group default qlen 1000
       link/ether 68:05:ca:84:32:19 brd ff:ff:ff:ff:ff:ff
       inet 169.254.100.1/16 brd 169.254.255.255 scope global enp37s0
          valid_lft forever preferred_lft forever

Here the interfaces are named `enp31s0` and `enp37s0`.
Interfaces may also have names like `enp31s0` or `eth0`.

.. _verify_mtu:
   
Verifying the MTU size
^^^^^^^^^^^^^^^^^^^^^^

Execute the following in the terminal

.. code-block:: sh
                   
   cat /sys/class/net/<INTERFACE_NAME>/mtu

Sample ouput:

.. code-block:: text

   9000

The optimal setting for large data transmissions has the MTU at 9000.
The default setting is typically at 1500.

Temporarily adjusting the MTU
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^   

To temporarily adjust the MTU for a network interface, execute the following in the terminal:

.. code-block:: sh

   sudo ip link set <INTERFACE_NAME> mtu 9000

This will set the MTU to the recommended setting.
To verify this, execute the steps described under :any:`Verifying the MTU size <verify_mtu>`.

Permanently adjusting the MTU
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This step depends on the setup of the computer in question.
   
