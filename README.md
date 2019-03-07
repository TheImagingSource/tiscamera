# The Imaging Source Linux Repository

This repository will give you additional ressources to control your TIS camera.

## What do we offer?

* gstreamer elements
* gobject introspection
* uvc extensions
* firmware update tools
* examples on how to interact with your camera


## Important Note

The instructions in this manual will install the software in the system folders located under /usr. If you want to install the software to a different location (eg. /usr/local/), you need to change the path specifications in the respective instructions by yourself. Please note that in this case you may need to take additional steps to allow your system to locate the installed libraries and other components.

## Dependencies

The following packages are required to build our software:

git
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
python3-setuptools

For aravis:

libxml2
libaudit
libpcap0.8

## Runtime Dependencies

The following packages are required to execute the compiled binaries for our software:

gstreamer-1.0
libusb-1.0
libglib2.0
libgirepository1.0
libudev
libtinyxml
libzip
libnotify

For aravis:

libxml2-dev
libaudit-dev
libpcap-dev
libnotify-dev

For our GUI applications:

python3-gi
python3-pyqt5


On a Debian / Ubuntu system, the following command line could be used to install all required packages in one go:

```
# Build dependencies
sudo apt-get install git g++ cmake pkg-config libudev-dev libudev1 libtinyxml-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libglib2.0-dev libgirepository1.0-dev libusb-1.0-0-dev libzip-dev uvcdynctrl python-setuptools libxml2-dev libpcap-dev libaudit-dev libnotify-dev autoconf intltool gtk-doc-tools python3-setuptools

# Runtime dependencies
sudo apt-get install gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly libxml2 libpcap0.8 libaudit1 libnotify4 python3-pyqt5 python3-gi
```

### aravis installation

We support GigE cameras through an aravis git submodule. A systemwide installation can be done but does not affect tiscamera.
If you want to build against an external aravis version set TCAM\_INTERNAL\_ARAVIS=OFF

If you only use USB cameras you can build without aravis.

### Building tiscamera

The following commands will build and install our software with default settings. A brief reference of compile time options could be found at the end of this document.

```
git clone --recursive https://github.com/TheImagingSource/tiscamera.git
cd tiscamera
mkdir build
cd build

# With ARAVIS:
cmake -DBUILD_ARAVIS=ON -DBUILD_GST_1_0=ON -DBUILD_TOOLS=ON -DBUILD_V4L2=ON -DCMAKE_INSTALL_PREFIX=/usr ..
# Without ARAVIS
cmake -DBUILD_ARAVIS=OFF -DBUILD_GST_1_0=ON -DBUILD_TOOLS=ON -DBUILD_V4L2=ON -DCMAKE_INSTALL_PREFIX=/usr ..

make
sudo make install
```

### Optional for GigE-Vision devices: Start the gige-daemon

GigE-Vision cameras have a several seconds long delay before they could be reliably detected on the network. To speed up this process for applications, a background daemon is build and installed which detects cameras before an application starts. The following commands will activate the daemon on your system:

```
sudo systemctl daemon-reload                 # make systemd aware of gige-daemon
sudo systemctl enable gige-daemon.service    # start on every boot
sudo systemctl start gige-daemon.service     # start the actual daemon
sudo systemctl status gige-daemon.service    # check if statemd say everything is ok
```


## cmake options

- **-DBUILD_ARAVIS=<ON/OFF>**

  Build tiscamera with support for aravis devices.

- **-DBUILD_GST_1_0=<ON/OFF>**

  Build gstreamer 1.0 plugins.

- **-DBUILD_TOOLS=<ON/OFF>**

  Build additional tools for camera interaction.

- **-DBUILD_V4L2=<ON/OFF>**

  Build tiscamera with suppoort for v4l2 devices.

- **-DBUILD_LIBUSB=<ON/OFF>**

  Build tiscamera with suppoort for v4l2 devices.

- **-DCMAKE_INSTALL_PREFIX**

  Installation target prefix (defaults to /usr/local)


**!! IMPORTANT !!**
Currently some features like uvc-extension units for v4l2 cameras or systemd units for GigE cameras require root privileges as no user directories are used for these files. If you wish to install without root privileges you will have to change the paths for udev, uvcdynctrl via -DTCAM\_INSTALL\_UDEV, -DTCAM\_INSTALL\_UVCDYNCTRL, -DTCAM\_INSTALL\_GSTREAMER, -DTCAM\_INSTALL_GIR, -DTCAM_INSTALL_TYPELIB and -DTCAM\_INSTALL\_SYSTEMD

## Where to go from here

After installation you could try one of our examples or directly start with a gstreamer pipeline like:

`gst-launch-1.0 tcambin ! videoconvert ! ximagesink`

## Questions, etc.

For questions simply open a ticket or write us a mail at support@theimagingsource.com.

## Licensing

All files are published under the Apache License 2.0, unless otherwise noted.

Included libraries:
PugiXml 1.6, which is available under a "MIT" license.
The json library by Niels Lohmann, which is available under the "MIT" license.
7z, which is published as public domain.
aravis, which is available under the LGPLv2.
CLI11 available under the 3-Clause BSD-License
Catch2 which is published under the Boost Software License, Version 1.0
