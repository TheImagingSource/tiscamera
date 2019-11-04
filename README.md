
# The Imaging Source Linux Repository

This repository will give you additional resources to control your TIS camera.

## What do we offer?

* gstreamer elements
* gobject introspection
* uvc extensions
* utilities
* examples on how to interact with your camera

https://www.theimagingsource.com/documentation/tiscamera/

## Dependencies

### Compilation

Compilation dependencies for debian can be found in:

dependencies-debian-compilation.txt

### Runtime

Runtime dependencies for debian can be found in:

dependencies-debian-runtime.txt

On a Debian / Ubuntu system, the following command line could be used to install all required packages in one go:

```
# Build dependencies
sudo ./scripts/install-dependencies.sh --compilation --runtime

# Runtime dependencies
sudo ./scripts/install-dependencies.sh --runtime
```

### Building tiscamera

The following commands will build and install our software with default settings. A brief reference of compile time options could be found at the end of this document.

```
git clone https://github.com/TheImagingSource/tiscamera.git
cd tiscamera
# only works on Debian based systems like Ubuntu
sudo ./scripts/install-dependencies.sh --compilation --runtime
mkdir build
cd build

# With ARAVIS:
cmake -DBUILD_ARAVIS=ON ..
# Without ARAVIS
cmake -DBUILD_ARAVIS=OFF ..

make
sudo make install
```
The default installation prefix is `/usr`.
Some components have to be installed in `/etc` and `/lib`.
If you want to change it, read the section [installation directories](https://www.theimagingsource.com/documentation/tiscamera/building.html#installation-directories) in our documentation.

#### cmake options
The most important cmake options are:
- **-DBUILD_ARAVIS=<ON/OFF>**
Build tiscamera with support for aravis devices.

- **-DBUILD_TOOLS=<ON/OFF>**
Build additional tools for camera interaction.

- **-DBUILD_V4L2=<ON/OFF>**
Build tiscamera with suppoort for v4l2 devices.

- **-DBUILD_LIBUSB=<ON/OFF>**
Build tiscamera with suppoort for v4l2 devices.

- **-DBUILD_DOCUMENTATION=<ON/OFF>**
Build html user documentation.

- **-DCMAKE_INSTALL_PREFIX**
Installation target prefix (defaults to /usr/local)

For a complete overview, read the section [cmake options](https://www.theimagingsource.com/documentation/tiscamera/building.html#cmake-options) in out documentation.

### Optional for GigE-Vision devices: Start the gige-daemon

GigE-Vision cameras have a several seconds long delay before they could be reliably detected on the network. To speed up this process for applications, a background daemon is build and installed which detects cameras before an application starts. The following commands will activate the daemon on your system:

```
sudo systemctl daemon-reload                 # make systemd aware of gige-daemon
sudo systemctl enable gige-daemon.service    # start on every boot
sudo systemctl start gige-daemon.service     # start the actual daemon
sudo systemctl status gige-daemon.service    # check if statemd say everything is ok
```

## Where to go from here

After installation you could try one of our examples or directly start with a gstreamer pipeline like:

`gst-launch-1.0 tcambin ! videoconvert ! ximagesink`

alternatively start `tcam-capture`

## Documentation

You can find an online version of the included user documentation here:

https://www.theimagingsource.com/documentation/tiscamera/

## Questions, etc.

For questions simply open a ticket or write us a mail at support@theimagingsource.com.

## Licensing

All files are published under the Apache License 2.0, unless otherwise noted.

Included libraries:
PugiXml 1.6, which is available under the "MIT" license.
The json library by Niels Lohmann, which is available under the "MIT" license.
7z, which is published as public domain.
aravis, which is available under the LGPLv2.
CLI11 available under the 3-Clause BSD-License
Catch2 which is published under the Boost Software License, Version 1.0
