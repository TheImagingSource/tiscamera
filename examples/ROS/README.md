# ROS Camera Interface
The TIS Ros Camera interface is a simple interface between the TIS GStreamer
modules and the ROS gscam plugin. It consists of two Python 3 scripts.
`tis_rossarter` is the main program, `tiscamera.py` wraps the GStreamer sources,
camera property handling and rosrun calls.

## ROS Installation
First the ROS gscam plugin is installed. Please refer to http://wiki.ros.org/gscam
for info.

`sudo apt-get install ros-kinetic-gscam`

Since ROS gscam is based on GStreamer 0.10, the GStreamer Base Plugins have to
be installed:

`sudo apt-get install libgstreamer-plugins-base0.10-dev`

After the installation, please also build the TIS GStreamer modules as described
under https://github.com/TheImagingSource/tiscamera.

## Configure your camera

Open `tis_rosstarter` with a text editor. In the following line, select the
camera, video format and frame rate:

```python
Tis = TIS.TIS("17719924",1920, 1080, 30, True, False)
```
The first parameter is the serial number of the camera. The next two are the
width and height of the video format, followed by the frame rate.

The fifth parameter determines whether a color image (true) or a black-and-white
image (false) should be displayed. The last parameter determines whether the
Python script should display its own live video window (True).

The values for the parameters can be determined with the tcam-ctrl program.

The roscore is already running. Now the camera communitcation is started with

`./tis_rosstarter`

The `rqt_image_view` program can be used to check the live video.

`tis_rosstarter` is stopped by hitting the enter key.

In the `tis_rosstarter` source code all camera properties can be set. Sample
code is already included. For example the line executes the Autofocus One Push,
provided that the camera supports this feature:

```python
Tis.PushProperty("Focus Auto")
```
