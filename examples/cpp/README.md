tcamgstcamera: C++ class library for accessing The Imaging Source cameras

To build this library:

1. Checkout and build the latest version of the tiscamera software from this location:

   https://github.com/theimagingsource/tiscamera

2. In addition to the dependencies of the tiscamera software, the following
   dependencies must be installed (development versions):

   - gtkmm >= 3.22

3. From the directory containing this README.md file, run the following commands:

   ```
   mkdir build
   cd build
   cmake ..
   make
   ```


This will create the following files/directories:

common/libtcamgstcamera.a   <<The c++ class library>>

examples/tcam_demo          <<Small getting started example>>

tcam_view/tcam_view         <<Graphical application to show the video stream and camera parameters>>

