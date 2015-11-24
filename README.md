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
* -DBUILD_GST_1_0 [default=ON]
Build additional gstreamer-1.0 modules
* -DBUILD_TOOLS [default=ON]
Build additional tools (firmware-tools, ip-configuration)
* -DBUILD_USB [default=ON]
Build against v4l2 to enable support for USB cameras


## Dependencies

To build all options:
g++-4.8 or higher
cmake
pkg-config
libudev-dev
libtinyxml-dev
libgstreamer1.0-dev
libglib2.0-dev
libgirepository1.0-dev
libusb-1.0-0-dev
libzip-dev

### aravis dependencies

To build aravis you require the following packages(assuming you are building all parts):

libxml2-dev
gtk-doc-tools
intltool
autoconf

libgstreamer0.10-dev
libgstreamer-plugins-base0.10-dev
libnotify-dev
libgtk-3-dev


## Don't know where to start?

Take a look at our wiki to see where to begin.

## Questions, etc.

For questions simply open a ticket or write us a mail.

## Licensing

All files are published under the Apache License 2.0, unless otherwise noted.
