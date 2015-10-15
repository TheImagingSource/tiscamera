# The Imaging Source Linux Repository

This repository will give you additional ressources to control your TIS camera.

## What do we offer?

* gstreamer elements
* uvc extensions
* firmware update tools
* examples on how to interact with your camera

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
* -DBUILD_GST_0_10 [default=OFF]
Build additional gstreamer-0.10 modules
* -DBUILD_GST_1_0 [default=ON]
Build additional gstreamer-1.0 modules
* -DBUILD_TOOLS [default=ON]
Build additional tools (firmware-tools, ip-configuration)
* -DBUILD_V4L2 [default=ON]
Build against v4l2 to enable support for USB cameras


## Dependencies

cmake
libudev
tinyxml
libgstreamer-1.0
libglib-2.0
libgobject

### Additional aravis dependencies

aravis-0.4
libxml2

## Don't know where to start?

Take a look at our wiki to see where to begin.

## Questions, etc.

For questions simply open a ticket or write us a mail.

## Licensing

All files are published under the Apache License 2.0, unless otherwise noted.
