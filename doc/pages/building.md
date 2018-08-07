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
