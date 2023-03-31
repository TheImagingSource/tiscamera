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

.. list-table:: list arguments
   :header-rows: 1
   :widths: 25 10 65

   * - Option
     - Short
     - Description

   * - --file
     - -f
     - Manual overwrite for selected dependency file

Listing
-------
   
To list all dependencies execute:

.. code-block:: sh

   dependency-manager list

This can be used to generate a dependency list that is compatible with the package dependency description of your distribution. This list will only contain runtime dependencies

.. code-block:: sh

   dependency-manager list --package deb

Sample output:
   
.. code-block:: sh

   libzip5 (>= 1.0.1), libglib2.0-0 (>= 2.48.2), libgirepository-1.0-1 (>= 1.46.0), libusb-1.0-0 (>= 2:1.0.20), libuuid1 (>= 2.27), libudev1 (>= 229), libgstreamer1.0-0 (>= 1.8.3), gstreamer1.0-plugins-base (>= 1.8.0), gstreamer1.0-plugins-good (>= 1.8.0), gstreamer1.0-plugins-bad (>= 1.8.0), gstreamer1.0-plugins-ugly (>= 1.8.3), libxml2 (>= 2.9.3), libqt5widgets5 (>= 5.9.5), libqt5gui5 (>= 5.9.5)

.. list-table:: list arguments
   :header-rows: 1
   :widths: 25 10 65

   * - Option
     - Short
     - Description

   * - --compilation
     -
     - Install compilation dependencies
   * - --runtime
     -
     - Install runtime dependencies
   * - --modules
     - -m
     - Only use listed modules, if not given all will be used
   * - --package
     -
     - List dependencies compatible with selected package manager. Supported values: `deb`.
      
   
Installation
------------

To install dependencies execute:

.. code-block:: sh

   dependency-manager install

The following options are available for the install command:

.. list-table:: install arguments
   :header-rows: 1
   :widths: 25 10 65

   * - Option
     - Short
     - Description

   * - --compilation
     -
     - Install compilation dependencies
   * - --runtime
     -
     - Install runtime dependencies
   * - --modules
     - -m
     - Only use listed modules, if not given all will be used
   * - --yes
     - -y
     - Assume 'yes' for prompts
   * - --dry-run
     - -s
     - Simulate actions but do not touch the system
   * - --no-update
     -
     - Do not update the package cache before installing

File Format
-----------

The file format the dependency manager uses is JSON.

All dependency descriptions are located in `<tiscamera>/dependencies/`


.. code-block:: json

   {
       "dependencies": [
           {
               "name": "package name",
               "version": "version with modifiers",
               "phase": "runtime/compilation",
               "modules": ["aravis", "base"]
           }
       ]
   }

.. _env_sh:
   
======
env.sh
======

env.sh is a Bourne Again shell script that can be sourced to integrate the build directory
into the current environment.

It will append directories to the PATH and library search path for the dynamic linker
and GStreamer, thus enabling usage of tiscamera resources without installation.

To source it, call the following in the build directory

.. code-block:: sh

   . ./env.sh

Now additional commands like :ref:`tcam_ctrl` or :ref:`tcam_capture` should be available.

.. _tiscamera_env_sh:

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
