
.. _tcamdeviceprovider:

####################
Tcam Device Provider
####################

The TcamDeviceProvider object provides indexing capabilities for tiscamera devices in a GStreamer context.

The DeviceProvider provides a GstBus message that contains a GstDevice object.  
This object contains all information to uniquely identify a device and to start
gstreamer streams with it.

All devices from `The Imaging Source` source elements are marked as `Video/Source/tcam`.

All devices come with the following properties:

* serial

* type

* model 

For technical details, see `the GStreamer documentation <https://gstreamer.freedesktop.org/documentation/gstreamer/gstdevicemonitor.html>`_.

For a real life example, see `12-device-monitor` in the examples folder.
