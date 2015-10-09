## showcamera

This sample was creating for learing about using V4L2, GStreamer and a little bit QT4. It is a simple camera application, that

 - allows to select a camera from a list of connected cameras
 - select the video format and frame rate
 - show the live video in a QT window 
 - shows a complete, but not nice property dialog

### Prerequisites
The libqt4-dev and libgstreamer-plugins-base0.10-dev are needed. Install them with

```sudo apt-get install libqt4-dev libgstreamer-plugins-base0.10-dev```

Als the TIS GStreamer modules are needed. How to build them is shown at
[GStreamer Modules](https://github.com/TheImagingSource/tiscamera/wiki/Getting-Started-with-USB-2.0-cameras-on-a-Raspberry-PI-or-on-other-Linux-Systems)

### Compiling

Change into the showcamera directory

```cd tiscamera/examples/showcamera```

create a build directory and change into it. Then call camake and make :

```
mkdir build
cd build
cmake ../
make -j
```

Run the program with

``` ./showcamera ```

in this build directory.

### Annotation
Even if this program lists GigE cameras, the handling of them is not implemented. The Imaging Source plans to create a
GStreamer source, that will handle USB and GigE cameras in a coherent way.

## Licensing

All files are published under the Apache License 2.0.