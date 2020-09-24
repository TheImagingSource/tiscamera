#######
Scripts
#######

Tiscamera has helper scripts that aim to help with deployment and installation.

.. _dependency_manager:

==================
dependency-manager
==================

The dependency-manager script is a simple helper tool for the dependency management of tiscamera.

.. note::

   Currently only Debian/Ubuntu are supported.

To list all dependencies execute:

.. code-block:: sh

   dependency-manager list

This can be used to generate a dependency list that is compatible with the package dependency description of your distribution. This list will only contain runtime dependencies

.. code-block:: sh

   dependency-manager list --package deb

Sample output:
   
.. code-block::

   libzip4 (>= 1.0.1), libglib2.0-0 (>= 2.48.2), libgirepository-1.0-1 (>= 1.46.0), libusb-1.0-0 (>= 2:1.0.20), libuuid1 (>= 2.27), libudev1 (>= 229), libgstreamer1.0-0 (>= 1.8.3), gstreamer1.0-plugins-base (>= 1.8.0), gstreamer1.0-plugins-good (>= 1.8.0), gstreamer1.0-plugins-bad (>= 1.8.0), gstreamer1.0-plugins-ugly (>= 1.8.3), libxml2 (>= 2.9.3), libpcap0.8 (>= 1.7.4-2), python3-pyqt5 (>= 5.5.1), python3-gi (>= 3.20.0)

File Format
-----------

The file format the dependency manager uses is JSON.

All dependency descriptions are located in `<tiscamera>/dependencies/`

.. code-block:: JSON

   {
       dependencies: [
           {
               "name": "package name",
               "version": "version with modifiers",
               "phase": "runtime/compilation",
               "modules": ["aravis", "base"]
           }
       ]
   }
   
=======================
install-dependencies.sh
=======================

.. warning::

   Deprecated as of `tiscamera 0.13.0`. Please use dependency-manager instead.

install-dependencies.sh is a shell script that allows the installation
of dependencies on Debian-based systems.
It allows for the installation of compilation and/or runtime dependencies. These can be
selected with ``--compilation`` and ``--runtime``.

Additionally, the argument ``--yes`` can be given to suppress the confirmation prompt of ``apt-get``.

Since the usage of installed packages by other software can not be tracked,
an uninstall option is not available.

To install all dependencies, call

.. code-block:: sh

   scripts/install-dependencies.sh --compilation --runtime

======
env.sh
======

env.sh is a Bourne shell script that can be sourced to integrate the build directory
into the current environment.

It will append directories to the PATH and library search path for the dynamic linker
and GStreamer, thus enabling usage of tiscamera resources without installation.

To source it, call the following in the build directory

.. code-block:: sh

   . ./env.sh

Now additional commands like :ref:`tcam_ctrl` or :ref:`tcam_capture` should be available.

================
tiscamera-env.sh
================

tiscamera-env.sh is a Bourne shell script that can be sourced to integrate the installation directories
of the installation into the current environment.

It will append directories to the PATH and library search path for the dynamic linker
and GStreamer, thus enabling usage of tiscamera resources without installation.

To source it, call the following in the build directory

.. code-block:: sh

. ./tiscamera-env.sh

The script is not installed. It can be found in the build directory under `./tiscamera-env.sh`

.. _create_release:

==============
create-release
==============

The create-release script aims to simplify the steps of versioning and tagging when creating a new release.
For the rules that are applied by this script, see :any:`Versioning and Releases<versioning_and_release>`.
These include:

- Updating the CHANGELOG.md file to the next version number
- Creating a new commit for said changes
- Tagging the new commit with the appropriate tag
