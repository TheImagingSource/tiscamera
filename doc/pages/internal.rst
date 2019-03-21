########
Internal
########

This section describes the internal project structure.

Folder Structure
================

This is a simplified folder structure of the tiscamera project:


|   tiscamera
|   ├── cmake
|   │   └── modules
|   ├── data - static data that is neither compiled or generated
|   │   ├── bash-completion
|   │   ├── firmware
|   │   │   └── usb2
|   │   ├── images
|   │   ├── systemd
|   │   ├── udev
|   │   └── uvc-extensions
|   ├── dependencies - third party software 
|   │   ├── 7z
|   │   ├── aravis
|   │   ├── catch
|   │   ├── CLI11
|   │   ├── json
|   │   └── PugiXml
|   ├── doc - documentation infrastructure
|   │   ├── images
|   │   ├── pages
|   │   │   ├── images
|   │   │   ├── _static
|   │   │   └── _templates
|   ├── examples - code examples to help with understanding 
|   │   ├── c
|   │   ├── cpp
|   │   ├── gstreamer-1.0
|   │   ├── lua
|   │   ├── python
|   │   └── ROS
|   ├── packaging - resources for the creation of binary distributions
|   │   └── deb
|   ├── scripts - helper scripts, see :any:`scripts`
|   ├── src
|   │   ├── algorithms
|   │   ├── aravis - aravis backend
|   │   ├── gobject - tcamprop property interface
|   │   ├── gstreamer-1.0 - gstreamer modules
|   │   ├── libusb - libusb-1.0 backend
|   │   ├── tcam-network - network helper library
|   │   └── v4l2 - v4l2 backend
|   ├── tests - verification code
|   │   ├── integration
|   │   │   └── start_stop
|   │   ├── release
|   │   └── unit
|   │       ├── algorithms
|   │       ├── gstreamer-1.0
|   │       └── tcam-network
|   └── tools
|       ├── camera-ip-conf
|       ├── dfk73udev
|       ├── firmware-update
|       ├── gige-daemon
|       ├── tcam-capture
|       ├── tcam-ctrl
|       ├── tcam-gigetool
|       └── tcam-uvc-extension-loader

Libraries
=========

This section describes the purpose behind the different libraries.

libtcam
-------

The main library. Device indexing, property mappings, etc. is done here.

libtcam-{aravis, v4l2, libusb}
------------------------------

These libraries represent the different backends. They are dynamically loaded by libtcam.

libtcamprop
-----------

gobject-introspection library. Used by all gstreamer modules.

libtcam-network
---------------

Common network functionality. Used by :any:`gige-daemon`, :any:`camera-ip-conf`, :any:`tcam-gigetool`

libtcam-uvc-extension
---------------------

:ref:`uvc extension<uvc_extensions>` loading functionality. Used by :any:`tcam-uvc-extension-loader`.

libtcamgstbase
--------------

Common functionality that is shared between the tcam gstreamer elements.

libtcam-algorithms
------------------

This library contains all algorithms like auto-exposure, whitebalance and autofocus.

libtcam-dfk73
-------------

Helper library for the correct initialization of DFK73 cameras.
