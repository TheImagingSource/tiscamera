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
|   │   ├── images - logos, icons, camera images, etc.
|   │   ├── systemd - systemd units
|   │   ├── udev - udev rules for usb cameras, see :doc:`udev`
|   │   └── uvc-extensions - description files for UVC extension units, see :doc:`uvc`
|   ├── doc - documentation infrastructure
|   │   ├── images
|   │   ├── pages
|   │   │   ├── images
|   │   │   ├── _static
|   │   │   └── _templates
|   ├── examples - code examples to help with understanding 
|   │   ├── c
|   │   └── python
|   ├── external - third party software 
|   │   ├── aravis
|   │   ├── catch - C++ test framework
|   │   ├── CLI11
|   │   ├── fmt-7.1.3
|   │   ├── json
|   │   ├── outcome
|   │   └── PugiXml
|   │   ├── spdlog
|   ├── libs - collection of helper libraries, not external projects
|   │   ├── dutils_image - simplified version of tiscamera-dutils algorithms
|   │   ├── gst-helper - helper libraries for gstreamer elements
|   │   └── tcam-property - tcam-property GObject library
|   ├── packaging - resources for the creation of binary distributions
|   │   └── deb
|   ├── scripts - helper scripts, see :any:`scripts`
|   ├── src - general source directory
|   │   ├── aravis - aravis backend
|   │   ├── gstreamer-1.0 - gstreamer modules
|   │   │   ├── tcamconvert - files related to the tcamconvert gstreamer plugin
|   │   │   ├── tcamgstbase - collection of shared gstreamer functionality
|   │   │   └── tcamsrc - files related to the tcamconvert gstreamer plugin
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
|       ├── :ref:`tcam-capture<tcam_capture>`
|       ├── :ref:`tcam-ctrl<tcam_ctrl>`
|       ├── :ref:`tcam-gige-daemon<tcam_gige_daemon>`
|       ├── :ref:`tcam-gigetool<tcam_gigetool>`
|       └── :ref:`tcam-uvc-extension-loader<tcam_uvc_extension_loader>`

Libraries
=========

This section describes the purpose behind the different libraries.

libtcam
-------

The main library. Device indexing, property mappings, etc. is done here.
The backends are also contained in this library.

libtcam-property
----------------

gobject-introspection library. Used by all gstreamer modules.

libtcam-network
---------------

Common network functionality.
Used by :ref:`tcam-gige-daemon<tcam_gige_daemon>` and :ref:`tcam-gigetool<tcam_gigetool>`

libtcam-uvc-extension
---------------------

:ref:`uvc extension<uvc_extensions>` loading functionality. Used by :ref:`tcam-uvc-extension-loader<tcam_uvc_extension_loader>`.

libtcamgstbase
--------------

Common functionality that is shared between the tcam gstreamer elements.

Threading
=========

Tiscamera will start multiple threads in the background.
The following is an overview over these.

- Indexing
  An internal device indexing thread will run periodically to ensure
  all potential devices are known and accessible to the user.
- Internal capture thread
  This thread is responsible for image acquisition. It will call the auto algorithms for further processing.
- Auto algorithms  
  This thread checks for exposure corrections, white balance and other before passing the image buffer to gstreamer.
- Streaming thread
  This is the thread tcamsrc and internal elements reside in.
  It will wait for a new image buffer to be given and pass it to downstream elements.
- GStreamer thread
  The main gstreamer thread. Responsible for message handling and state changes.
  Typically implicitly started by the user application.
- Device lost thread
  Security thread for the propagation of device lost events. Idle unless such an event occurs.
  
