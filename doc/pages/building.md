# Configuration and Building {#building}

## Getting the code

To get the code execute:

```
git clone --recursive https://github.com/TheImagingSource/tiscamera.git
```

This requires that git is installed.

## cmake options

- **-DBUILD_ARAVIS=<ON/OFF>**

    Build tiscamera with support for aravis devices.

    _Default:_ OFF

- **-DBUILD_GST_1_0=<ON/OFF>**

    Build gstreamer 1.0 plugins.

    _Default:_ ON

- **-DBUILD_TOOLS=<ON/OFF>**

    Build additional tools for camera interaction.

    _Default:_ OFF

- **-DBUILD_V4L2=<ON/OFF>**

    Build tiscamera with support for v4l2 devices.

    _Default:_ ON

- **-DBUILD_LIBUSB=<ON/OFF>**

    Build tiscamera with support for usb devices that that have no native driver.

    _Default:_ ON

- **-DCMAKE_INSTALL_PREFIX**

    Installation target prefix

    _Default:_ /usr

- **-DCMAKE_RELEASE_TYPE**

    Release type to be built.
    Set this to `Debug` to enable debug symbols.

    _Default:_ Release


If you wish for a more interactive solution you can use the program `cmake-gui` to interactively configure your tiscamera checkout.  
Under debian/ubuntu you can install it with `sudo apt install cmake-qt-gui`

## installation directories

This is a list of all installation directories that are used.  
All directories can be changed independently of each other when configuring the project with cmake.  
Paths may change when CMAKE_INSTALL_PREFIX is set.

Binaries: /usr/bin

Libraries: /usr/lib

Header: /usr/include

GStreamer: /usr/lib/x86_64-linux-gnu/gstreamer-1.0 This path may vary depending on your system

Bash Completion: /usr/share/bash-completion/completions

Systemd Units: /lib/systemd/system

Static Data: /usr/share/tiscamera

UDev: /etc/udev/rules.d/

UVC Extensions: /usr/share/uvcdynctrl/data/199e

Pkgconfig Files: /usr/share/applications

Desktop Files: /usr/share/applications
