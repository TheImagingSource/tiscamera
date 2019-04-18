#######
Scripts
#######

Tiscamera has helper scripts that aim to help with deployment and installation.

=======================
install-dependencies.sh
=======================

install-dependencies.sh is a shell scripts that allows the installation of dependencies on Debian
based systems.
It allows for the installation of compilation and/or runtime dependencies. These can be
selected with --compilation and --runtime.

Additionally the argument --yes can be given to suppress the confirmation prompt of apt-get.

Since the usage of installed packages by other software can not be tracked an uninstall option is not available.

To install all dependencies call

.. code-block:: sh

   scripts/install-dependencies.sh --compilation --runtime

======
env.sh
======

env.sh is a bourne shell script that can be sourced to integrate the build directory
into the current environment.

It will append directories to the PATH and library search path for the dynamic linker
and gstreamer, thus enabling usage of tiscamera resources without installation.

To source it call the following in the build directory

.. code-block:: sh

   . ./env.sh

Now additional commands like :ref:`tcam_ctrl` or :ref:`tcam_capture` should be available.

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
