# The Imaging Source Linux Repository

This repository will give you additional ressources to control your TIS camera.

## What do we offer?

* C/C++ library for image retrieval
* gstreamer elements
* uvc extensions
* firmware update tools
* examples on how to interact with your camera
* GUI application that allows straight forward camera interaction

## How to install?

mkdir build
cd build
cmake ..
make
sudo make install

## Available options

* -DBUILD_ARAVIS [default=OFF]
Build against aravis to enable support for GigE cameras
* -DBUILD_DOC [default=OFF]
BUILD doxygen and other project documentations
-DBUILD_FIREWIRE [default=OFF]
Build against 1394 libraries to enable support for Firewire cameras
-DBUILD_GST_0_10 [default=OFF]
Build additional gstreamer-0.10 modules
-DBUILD_GST_1_0 [default=OFF]
Build additional gstreamer-1.0 modules
-DBUILD_TOOLS [default=OFF]
Build additional tools (firmware-tools, ip-configuration)
-DBUILD_USB [default=ON]
Build against v4l2 to enable support for USB cameras


## Dependencies

cmake
libudev

### Additional aravis dependencies

aravis-0.4
libxml2
libgobject

### Additional gstreamer dependencies

libgstreamer-0.10
libglib-2.0

### Qt4 dependencies



## Don't know where to start?

Take a look at our wiki to see where to begin.

## Questions, etc.

For questions simply open a ticket or write us a mail.

## Licensing

All files are published under the Apache License 2.0.

