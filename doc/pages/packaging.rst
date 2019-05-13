
.. _packaging:

#########
Packaging
#########

The build environment allows the creation a binary package for further
distribution of the compiled binaries.

Currently, only the creation of Debian packages is supported.

A release will be published at https://github.com/TheImagingSource/tiscamera/releases .

At this moment only amd64 releases are available.

Naming
======

The naming scheme for the packages follows these rules:

The package is created on the master branch and is thus treated as a release.

`tiscamera_<MAJOR>.<MINOR>.<PATCH>_<ARCHITECTURE>.deb`

Any other branch used will result in the following scheme:

`tiscamera_<MAJOR>.<MINOR>.<PATCH>.<COMMIT_COUNT>~<BRANCH>_<COMMIT_HASH>_<ARCHITECTURE>.deb`

This will be overwritten when a tag is present for the current commit. In this
case, the package naming scheme will be:

`tiscamera_<MAJOR>.<MINOR>.<PATCH>.<COMMIT_COUNT>_<TAG>_<ARCHITECTURE>.deb`

Creation
========

To create a package, simply call the following command in the build directory:

.. code-block:: sh

   make package

This will create a .deb file and a checksum file for the package.

Installation
============

To install the debian package, execute:

.. code-block:: sh

   sudo apt install tiscamera*.deb

All dependencies will automatically be installed by the package manager.

Updating
========

To update the used version, simply follow the install instruction with the new
package. The package manager will detect it as a new version.

For a downgrade, dpkg has to be called directly:

.. code-block:: sh

   sudo dpkg -i tiscamera*.deb

Uninstall
=========

To deinstall the package, execute:

.. code-block:: sh

   sudo apt remove tiscamera
