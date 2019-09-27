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

   * - -DBUILD_ARAVIS
     - Build tiscamera with support for aravis devices.
     - OFF
   * - -DBUILD_GST_1_0
     - Build gstreamer 1.0 plugins.
     - ON
   * - -DBUILD_TOOLS
     - Build additional tools for camera interaction.
     - OFF
   * - -DBUILD_V4L2
     - Build tiscamera with support for V4L2 devices.
     - ON

   * - -DBUILD_LIBUSB
     - Build tiscamera with support for USB devices that that have no native driver.
     - ON

   * - -DBUILD_DOCUMENTATION
     - Build html documentation. This requires Sphinx Version >= 1.4
     - OFF
       
   * - -DBUILD_TESTS
     - Build unit/integration tests.
     - OFF

   * - -DCMAKE_INSTALL_PREFIX
     - Installation target prefix
     - /usr

   * - -DCMAKE_RELEASE_TYPE
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

   * - TCAM_VERSION
     - TCAM release version
     - Current version string

   * - TCAM_GI_API_VERSION
     - Version the gobject introspection tiscamera has.
     - 0.1 
       
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
   * - Binaries
     - TCAM_INSTALL_BIN
     - /usr/bin
     - Directory for executables
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
     - /usr/share/tiscamera
     -
   * - UDev
     - TCAM_INSTALL_UDEV
     - /etc/udev/rules.d/
     - Directory for :any:`udev`
   * - UVC Extensions
     - TCAM_INSTALL_UVC_EXTENSION
     - /usr/share/theimagingsource/tiscamera/
     - 
   * - Pkgconfig Files
     - TCAM_INSTALL_PKGCONFIG
     - /usr/lib/pgkconfig
     -
   * - Desktop Files
     - TCAM_INSTALL_DESKTOP_FILES
     - /usr/share/applications
     - 
