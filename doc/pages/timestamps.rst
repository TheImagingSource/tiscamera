

##########
Timestamps
##########

This page describes the various timestamps that a GstBuffer can be associated with.

PTS
===

The presentation timestamp is a GStreamer internal timestamp and references a gstclock which is associated with the start of a GStreamer stream. These timestamps are automatically created by GStreamer and describe the time the buffer was emitted by the tcamsrc.

Capture Time
============

.. note::
   This requires Gstreamer version >= 1.14.

All GstBuffer contain a capture time that can be retrieved by calling
`gst_buffer_get_meta(buffer, g_type_from_name("TcamStatisticsMetaApi"))` for the associated buffer.
The retrieved timestamp will tell when the backend/driver on the computer captured the image.
This timestamp will always be in nanoseconds.

This frame of reference will depend on the backend.
When unsure about the used backend, call `tcam-ctrl -l` in a terminal and check the 'Type' column.

For an implementation example, see c example `10-metadata.c`

V4L2 Kernel Driver
++++++++++++++++++

The V4L2 backend will deliver a timestamp whose point of reference is the boot time.
Please be aware that suspend/hibernate will stop the clock.

Aravis Userspace Library
++++++++++++++++++++++++

The Aravis backend provides a timestamp (WallClockTime).
This timestamp starts when the first bytes of the image where are received.
The point of reference is January 1, 1970 UTC.

Camera Capture Time
===================

.. note::
   This requires Gstreamer version >= 1.14.

GigE cameras deliver an additional timestamp that describes the time when the camera itself captured the image.
This timestamp can be retrieved by calling `gst_buffer_get_meta(buffer, g_type_from_name("TcamStatisticsMetaApi"))`.

This timestamp will always be in nanoseconds.

The frame of reference is the boot time of the camera.
Manually resetting the frame of reference is possible by calling the property 'TimestampReset'.
