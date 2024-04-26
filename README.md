
# The Imaging Source Linux Repository

This repository will give you additional resources to control your TIS camera.

## What do we offer?

* gstreamer elements
* gobject introspection
* uvc extensions
* utilities
* examples on how to interact with your camera
* documentation

## Documentation

You can find an online version of the included user documentation here:

https://www.theimagingsource.com/documentation/tiscamera/

## Precompiled Debian Packages

Ubuntu 18 lts packages are compatible with ubuntu 22 and ubuntu 24.

As of Ubuntu24.04 LTS arm 32-bit is no longer supported.
Please use a supported Release (Ubuntu 18 - Ubuntu 22), or switch to a arm64 release.

## API BREAK

Version 1.0.0 introduces several API changes.

To port your software check [our upgrade guide](doc/upgrade_guide_1.0.md)

The following device types are not supported going forward:

- 23G
- 23U
- 73U
- AFU130

A complete compatibility list can be found here:
https://www.theimagingsource.com/documentation/tiscamera/supported-devices.html

## Dependencies

### Compilation

Compilation dependencies for debian can be found be executing:

    ./scripts/dependency-manager list --compilation

### Runtime

Runtime dependencies for debian can by executing:

    ./scripts/dependency-manager list --runtime


On a Debian / Ubuntu system, the following command line can be used to install all required packages in one go:

```

./scripts/dependency-manager install

```

## Building tiscamera

The following commands will build and install our software with default settings.

```
git clone https://github.com/TheImagingSource/tiscamera.git
cd tiscamera
# only works on Debian based systems like Ubuntu
sudo ./scripts/dependency-manager install
mkdir build
cd build

# With ARAVIS:
cmake -DTCAM_BUILD_ARAVIS=ON ..
# Without ARAVIS
cmake -DTCAM_BUILD_ARAVIS=OFF ..

make
sudo make install
```
The default installation prefix is `/usr`.
Some components have to be installed in `/etc` and `/lib`.
If you want to change the prefix, read the section [installation directories](https://www.theimagingsource.com/documentation/tiscamera/building.html#installation-directories) in our documentation.

#### cmake options
The most important cmake options are:
- **-DTCAM_BUILD_ARAVIS=<ON/OFF>**
Build tiscamera with support for GigE cameras via aravis.

- **-DTCAM_BUILD_TOOLS=<ON/OFF>**
Build additional tools for camera interaction (e.g. firmware tools and tcam-capture).

- **-DTCAM_BUILD_V4L2=<ON/OFF>**
Build tiscamera with support for USB cameras via UVC/V4L2.

- **-DTCAM_BUILD_LIBUSB=<ON/OFF>**
Build tiscamera with support for USB cameras via LibUsb (i.e. AFU420, AFU050, DFK73).

- **-DTCAM_BUILD_DOCUMENTATION=<ON/OFF>**
Build html user documentation.

- **-DCMAKE_INSTALL_PREFIX**
Installation target prefix (defaults to /usr/)

For a complete overview, read the section [cmake options](https://www.theimagingsource.com/documentation/tiscamera/building.html#cmake-options) in out documentation.

### Optional for GigE-Vision devices: Start the tcam-gige-daemon

GigE-Vision cameras have a several seconds long delay before they can be reliably detected on the network.

To speed up this process for applications, a background daemon is built and installed, which detects cameras before an application starts. The following commands will activate the daemon on your system:

```
sudo systemctl daemon-reload                 # make systemd aware of gige-daemon
sudo systemctl enable tcam-gige-daemon.service    # start on every boot
sudo systemctl start tcam-gige-daemon.service     # start the actual daemon
sudo systemctl status tcam-gige-daemon.service    # check if statemd say everything is ok
```

## Where to go from here

After installation you can try one of our examples or directly start with a gstreamer pipeline like:

`gst-launch-1.0 tcambin ! videoconvert ! ximagesink`

alternatively start `tcam-capture`

## Questions, etc.

For questions simply open a ticket or write us a mail at support@theimagingsource.com.

## Licensing

All files are published under the Apache License 2.0, unless otherwise noted.

Included libraries:  
PugiXml 1.6, which is available under the "MIT" license.  
The json library by Niels Lohmann, which is available under the "MIT" license.  
7z, which is published as public domain.  
aravis, which is available under the LGPLv2.  
CLI11, available under the 3-Clause BSD-License  
Catch2, which is published under the Boost Software License, Version 1.0  
spdlog, which is available under the "MIT" license.  
fmt, which is available under the "MIT" license.
