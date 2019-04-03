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
|   │   ├── firmware
|   │   │   └── usb2
|   │   ├── images - logos, icons, camera images, etc.
|   │   ├── systemd - systemd units
|   │   ├── udev - udev rules for usb cameras, see :doc:`udev`
|   │   └── uvc-extensions - description files for UVC extension units, see :doc:`uvc`
|   ├── dependencies - third party software 
|   │   ├── 7z
|   │   ├── aravis
|   │   ├── catch - C++ test framework
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
|   │   └── python
|   ├── packaging - resources for the creation of binary distributions
|   │   └── deb
|   ├── scripts - helper scripts, see :any:`scripts`
|   ├── src - general source directory
|   │   ├── algorithms - autofocus, whitebalance, etc.
|   │   ├── aravis - aravis backend
|   │   ├── gobject - tcamprop property interface
|   │   ├── gstreamer-1.0 - gstreamer modules
|   │   ├── libusb - libusb-1.0 backend
|   │   ├── tcam-network - network helper library
|   │   └── v4l2 - v4l2 backend
|   ├── tests - verification code, see :doc:`tests`
|   │   ├── integration
|   │   │   └── start_stop
|   │   └── unit
|   │       ├── gstreamer-1.0
|   │       └── tcam-network
|   └── tools - directory for applications 
|       ├── :ref:`camera-ip-conf<camera_ip_conf>`
|       ├── dfk73udev
|       ├── :ref:`firmware-update<firmware_update>`
|       ├── :ref:`gige-daemon<gige_daemon>`
|       ├── :ref:`tcam-capture<tcam_capture>`
|       ├── :ref:`tcam-ctrl<tcam_ctrl>`
|       ├── :ref:`tcam-gigetool<tcam_gigetool>`
|       └── :ref:`tcam-uvc-extension-loader<tcam_uvc_extension_loader>`

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

Common network functionality.
Used by :ref:`gige-daemon<gige_daemon>`, :ref:`camera-ip-conf<camera_ip_conf>`,
:ref:`tcam-gigetool<tcam_gigetool>`

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

