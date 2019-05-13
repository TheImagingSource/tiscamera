#############
Optimizations
#############

The topics on this page deal with the optimization of programs/systems related to image capture.
If a capture process is already running without issues,
we recommend to not implement any of these steps.

.. _real_time_threading:

Real Time Threading
===================

Real time threads are threads which the kernel gives a high priority to.

The capture threads tiscamera uses are set to real time.

By default, most Linux distributions prevent users and their application from manipulating
the scheduling priority of threads.
A real time thread can cause harm to the stability of the system.
Change this at your own risk.

Please be aware that the presence of SELinux or other security enhancements might
require additional steps that are not covered in this document.

Edit `/etc/security/limits.conf` and add the following lines at the end of the file:

.. code-block:: text
                
    <username>    hard    rtprio    99
    <username>    soft    rtprio    25

Where \<username\> should be replaced with the user under which the application will be run.

A restart may be required to apply these changes.

The `hard limit` is the absolute limit to which the soft limit can be set.
The `soft limit` is the highest priority a thread can be set to.
All capture threads operate with a priority of 10.

To change the soft limit at runtime enter:

.. code-block:: sh

   ulimit -Sr <limit>

tmpfs
=====

When using slow or delicate storage like a SD-card, it can be useful to store image/video files in a tmpfs.

A tmpfs is a file system that behaves like an ordinary file system but exists purely in memory.
The advantages are that nothing is written to the actual system hard drive,
making write/read times faster.
Please be aware that the content of the filesystem is lost on every reboot.
Please be aware: using a tmpfs will reduce the amount of memory available to the rest of the system.

To permanently add a tmpfs to a system, add the following line to the file **/etc/fstab**

.. code-block:: text

   tmpfs   /tmp/tiscamera         tmpfs   nodev,nosuid,default,size=2G          0  0


This line will mount a tmpfs with the size of 2 GB in /tmp/tiscamera.
The mount point for the tmpfs must exist before mount is called.
