##########################
Configuration and Building
##########################

================
Getting the code
================

To get the code, execute:

```
git clone --recursive https://github.com/TheImagingSource/tiscamera.git
```

This requires that git be installed.

.. _configuring:

===========
Configuring
===========


cmake options
=============

.. list-table:: cmake options
   :header-rows: 1

   * - option
     - description
     - default

   * - TCAM_BUILD_ARAVIS
     - Build tiscamera with support for aravis devices.
     - ON
       
   * - TCAM_BUILD_GST_1_0
     - Build gstreamer 1.0 plugins.
     - ON
       
   * - TCAM_BUILD_TOOLS
     - Build additional tools for camera interaction.
     - ON
       
   * - TCAM_BUILD_V4L2
     - Build tiscamera with support for V4L2 devices.
     - ON

   * - TCAM_BUILD_LIBUSB
     - Build tiscamera with support for USB devices that that have no native driver.
     - ON

   * - TCAM_BUILD_DOCUMENTATION
     - Build html documentation. This requires Sphinx Version >= 1.4
     - ON
       
   * - TCAM_BUILD_TESTS
     - Build unit/integration tests.
     - OFF

   * - CMAKE_INSTALL_PREFIX
     - Installation target prefix
     - /usr

   * - CMAKE_RELEASE_TYPE
     - Release type to be built.
       Set this to `Debug` to enable debug symbols.
     - Release

   * - TCAM_INTERNAL_ARAVIS
     - To reduce required steps for customers it was decided to ship an internal aravis version.
       This version will be downloaded, compiled and statically linked during the build process (libtcam-aravis.so).
       The current minimal version of aravis that is supported is '0.6'.

       When switching to an external version the following steps are recommended:
       
       - deleting the existing build directory
         CMake may retain build artifacts that may prevent a clean compilation.
       - Configuration of header/library search paths.
         Tiscamera tries to automatically determine these values through pkg-config.
         To manually configure the aravis installation define the variables 'aravis_INCLUDE_DIR' and 'aravis_LIBRARIES'.
         The only cmake file that uses aravis is 'src/aravis/CMakeLists.txt'
         
     - ON

   * - TCAM_ARAVIS_USB_VISION
     - Enables USB3Vision via aravis.
       When v4l2 and Usb3Vision are enabled cameras will be listed twice but with different bookends.
       To select Usb3Vision either set the `type` argument to `aravis`,
       or set the serial to `<serial>-aravis`.
     - ON
       
   * - TCAM_VERSION
     - TCAM release version
     - Current version string

   * - TCAM_GI_API_VERSION
     - Version the gobject introspection tiscamera has.
     - 1.0

   * - TCAM_DOWNLOAD_MESON
     - Download a local meson version to compile aravis.
     - ON

   * - TCAM_BUILD_WITH_GUI
     - Enable/Disable GUI parts of tiscamera. Currently only involves tcam-capture.
     - ON
       
Installation Directories
========================

This is a list of all installation directories used.
All directories can be changed independently of each other when configuring the project with cmake.
Paths may change when CMAKE_INSTALL_PREFIX is set.

.. list-table:: Default Installation Directories
   :header-rows: 1

   * - Name
     - Variable
     - Default Directory
     - Description
   * - bash completions
     - TCAM_INSTALL_BASH_COMPLETION
     - /usr/share/bash-completion/completions/
     - Directory for bash completions
   * - binaries
     - TCAM_INSTALL_BIN
     - /usr/bin
     - Directory for executables
   * - desktop files
     - TCAM_INSTALL_DESKTOP_FILES
     - /usr/share/applications
     - 
   * - documentation
     - TCAM_INSTALL_DOCUMENTATION
     - /usr/share/theimagingsource/tiscamera/doc/
     - location of html documentation
   * -
     - 
     -
     - 
   * - Libraries
     - TCAM_INSTALL_LIB
     - /usr/lib
     - Directory for libraries
   * - Header
     - TCAM_INSTALL_INCLUDE
     - /usr/include
     - Directory for header
   * - GStreamer
     - TCAM_INSTALL_GST_1_0
     - /usr/lib/x86_64-linux-gnu/gstreamer-1.0
     - This path may vary
   * - Systemd Units
     - TCAM_INSTALL_SYSTEMD
     - /lib/systemd/system
     - 
   * - Static Data
     - TCAM_INSTALL_IMAGE_DIR
     - /usr/share/theimagingsource/tiscamera/images/
     -
   * - UDev
     - TCAM_INSTALL_UDEV
     - /etc/udev/rules.d/
     - Directory for :ref:`udev`
   * - UVC Extensions
     - TCAM_INSTALL_UVC_EXTENSION
     - /usr/share/theimagingsource/tiscamera/uvc-extension/
     - 
   * - Pkgconfig Files
     - TCAM_INSTALL_PKGCONFIG
     - /usr/lib/x86_64-linux-gnu/pgkconfig
     - This path may vary

========
Building
========

The following build targets are available.
Call `make <target>` or `ninja <target>` to build.

.. list-table:: Build Targets
   :header-rows: 1

   * - Target
     - Description
   * - all
     - Default Target. Builds all selected components.
   * - clean
     - Remove all build objects. Reverts to state after cmake has been called.
   * - package
     - Build the debian package.
   * - install
     - Install compiled files into the system.
   * - uninstall
     - Remove installed files.
   * - test
     - Run unit tests.
       
