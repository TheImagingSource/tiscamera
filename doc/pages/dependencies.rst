############
Dependencies
############

The tiscamera project has a few dependencies.

The listed versions are the minimal supported versions of the current reference system.
This system is currently Ubuntu 18.04 LTS.

Compilation Dependencies
========================

The dependencies are also listed in the `*.dep` file. Please use the file for your distribution.

+---------------------------------+------------------+--------------------------+
| Name                            |Minimal Version   |Note                      |
+---------------------------------+------------------+--------------------------+
| git                             |1:2.7.4           |                          |
+---------------------------------+------------------+--------------------------+
| g++                             |4:5.3.1           |                          |
+---------------------------------+------------------+--------------------------+
| cmake                           |3.2               |                          |
+---------------------------------+------------------+--------------------------+
| pkg-config                      |0.29.1            |                          |
+---------------------------------+------------------+--------------------------+
| libzip-dev                      |1.0.1             |                          |
+---------------------------------+------------------+--------------------------+
| python3-setuptools              |20.7.0-1          |                          |
+---------------------------------+------------------+--------------------------+
| libgstreamer1.0-dev             |1.8.3-1           |                          |
+---------------------------------+------------------+--------------------------+
| libgstreamer-plugins-base1.0-dev|1.8.3-1           |                          |
+---------------------------------+------------------+--------------------------+
| libglib2.0-dev                  |2.48.2            |                          |
+---------------------------------+------------------+--------------------------+
| libgirepository1.0-dev          |1.46.0            |                          |
+---------------------------------+------------------+--------------------------+
| **usb specific dependencies**                                                 |
+---------------------------------+------------------+--------------------------+
| libusb-1.0-0-dev                |2:1.0.20.1        |                          |
+---------------------------------+------------------+--------------------------+
| uuid-dev                        |2.27              |                          |
+---------------------------------+------------------+--------------------------+
| libudev-dev                     |229               |                          |
+---------------------------------+------------------+--------------------------+
| **documenation specific dependencies**                                        |
+---------------------------------+------------------+--------------------------+
| python3-sphinx                  |1.4               | Also installable via pip |
+---------------------------------+------------------+--------------------------+
| **aravis specific dependencies**                                              |
+---------------------------------+------------------+--------------------------+
| libxml2-dev                     |2.9.3             |                          |
+---------------------------------+------------------+--------------------------+
| autoconf                        |2.69-9            |                          |
+---------------------------------+------------------+--------------------------+
| intltool                        |0.51.0            |                          |
+---------------------------------+------------------+--------------------------+
| gtk-doc-tools                   |1.25              |                          |
+---------------------------------+------------------+--------------------------+

  
Runtime Dependencies
====================

The dependencies are also listed in the `*.dep` file. Please use the file for your distribution.

+-----------------------------+----------------+-------------------------------+
|Name                         |Minimal Version |Note                           |
+-----------------------------+----------------+-------------------------------+
|**general dependencies**                                                      |
|                                                                              |
+-----------------------------+----------------+-------------------------------+
|libgstreamer1.0-0            |1.8.3           |                               |
+-----------------------------+----------------+-------------------------------+
|gstreamer1.0-tools           |1.8.0           |                               |
+-----------------------------+----------------+-------------------------------+
|gstreamer1.0-x               |1.8.3           |                               |
|                             |                |                               |
+-----------------------------+----------------+-------------------------------+
|gstreamer1.0-plugins-base    |1.8.0           |                               |
+-----------------------------+----------------+-------------------------------+
|gstreamer1.0-plugins-good    |1.8.0           |                               |
+-----------------------------+----------------+-------------------------------+
|gstreamer1.0-plugins-bad     |1.8.0           |                               |
+-----------------------------+----------------+-------------------------------+
|gstreamer1.0-plugins-ugly    |1.8.0           |                               |
+-----------------------------+----------------+-------------------------------+
|libxml2                      |2.9.3           |                               |
+-----------------------------+----------------+-------------------------------+
|libzip4                      |1.0.1           | Ubuntu 20.04 requires libzip5 |
+-----------------------------+----------------+-------------------------------+
|libglib2.0-0                 |2.48.2          |                               |
+-----------------------------+----------------+-------------------------------+
|libgirepository-1.0.1        |1.46.0          |                               |
+-----------------------------+----------------+-------------------------------+
|**usb specific dependencies**                                                 |
+-----------------------------+----------------+-------------------------------+
|libudev1                     |229             |                               |
+-----------------------------+----------------+-------------------------------+
|libusb-1.0.0                 |2:1.0.20        |                               |
+-----------------------------+----------------+-------------------------------+
|libuuid1                     |2.27            |                               |
+-----------------------------+----------------+-------------------------------+
|**tool specific dependencies**                                                |
+-----------------------------+----------------+-------------------------------+
|libxml2                      |2.9.3           |                               |
+-----------------------------+----------------+-------------------------------+
|python3-pyqt5                |5.5.1           |                               |
+-----------------------------+----------------+-------------------------------+
|python3-gi                   |3.20.0          |                               |
+-----------------------------+----------------+-------------------------------+


Inofficial Dependencies
=======================

Scripts or tests that are not intended for day-to-day use
may have additional dependencies.



These currently are:

gitpython
