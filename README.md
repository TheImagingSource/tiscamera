# The Imaging Source Linux Repository

This repository will give you additional ressources to control your TIS camera.

## What do we offer?

* gstreamer elements
* gobject introspection
* uvc extensions
* firmware update tools
* examples on how to interact with your camera

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

    sudo apt-get install g++ cmake pkg-config libudev-dev libudev1 libtinyxml-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libglib2.0-dev libgirepository1.0-dev libusb-1.0-0-dev libzip-dev uvcdynctrl

If you use GigE cameras you will have to install aravis manually.

To install aravis you require the following additional packages.

    sudo apt-get install libxml2-dev gtk-doc-tools intltool autoconf autoconflibnotify-dev libgtk-3-dev



### Building tiscamera

To build tiscamera follow these steps:

mkdir build
cd build

cmake <OPTIONS> -DCMAKE\_INSTALL\_PREFIX:PATH=/usr ..
The most important options are:

**-DBUILD_ARAVIS=<ON/OFF>**

Build tiscamera with support for aravis devices.

**-DBUILD_GST_1_0=<ON/OFF>**

Build gstreamer 1.0 plugins.

**-DBUILD_TOOLS=<ON/OFF>**

Build additional tools for camera interaction.

**-DBUILD_V4L2=<ON/OFF>**

Build tiscamera with suppoort fpr v4l2 devices.

After configuring the project you can proceed building it via:

make

After make ran successfully your build directory will contain all libraries and executables.

To install them you can call:

sudo make install

**!! IMPORTANT !!**
Currently some features like uvc-extension units for v4l2 cameras or systemd units for GigE cameras require root privileges as no user directories are used for these files. If you wish to install without root privileges you will have to change the paths for udev, uvcdynctrl via -DTCAM\_INSTALL\_UDEV, -DTCAM\_INSTALL\_UVCDYNCTRL, -DTCAM\_INSTALL\_GSTREAMER, -DTCAM\_INSTALL_GIR, -DTCAM_INSTALL_TYPELIB and -DTCAM\_INSTALL\_SYSTEMD

Alternatively you can launch everything from within the build directory.
To ensure that everything is reachable the following environment variables should be set:

To enable the systemd unit you will have to execute:

sudo systemctl daemon-reload                # make systemd aware of gige-daemon
sudo sytemctl enable gige-daemon.service    # start on every boot
sudo sytemctl start gige-daemon.service     # start the actual daemon
sudo sytemctl status gige-daemon.service    # check if statemd say everything is ok

## Don't know where to start?

Take a look at our wiki to see where to begin.

## Questions, etc.

For questions simply open a ticket or write us a mail at support@theimagingsource.com.

## Licensing

All files are published under the Apache License 2.0, unless otherwise noted.
