## showcamera

This sample was created for learning about using V4L2, GStreamer and a little bit QT4. It is a simple camera application, that

 - allows to select a camera from a list of connected cameras
 - allows to select the video format and frame rate
 - shows the live video in a QT window 
 - shows a complete, but not nice property dialog
 - saves jpeg imags on demand.
 
It shows, how to enumerate cameras, their video formats and the camera properties.

### Prerequisites
The libqt4-dev and libgstreamer-plugins-base0.10-dev are needed. Install them with

```sudo apt-get install libqt4-dev libgstreamer-plugins-base0.10-dev```

The TIS GStreamer modules are needed. How to build them is shown at
[GStreamer Modules](https://github.com/TheImagingSource/tiscamera/wiki/Getting-Started-with-USB-2.0-cameras-on-a-Raspberry-PI-or-on-other-Linux-Systems)

### Compiling

Change into the showcamera directory

```cd tiscamera/examples/showcamera```

Create a ```build``` directory and change into it. Then call camake and make :

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

Even if this sample program lists GigE cameras, handling them is not implemented. The Imaging Source plans to create a
GStreamer source, that will handle USB and GigE cameras in a coherent way.

## Licensing

All files are published under the Apache License 2.0.