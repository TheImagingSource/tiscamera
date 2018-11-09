
# Timestamps {#timestamps}

This page describes the various timestamps that a GstBuffer you receive can be associated with.

## PTS

The presentation timestamp is a gstreamer internal timestamp that references a gstclock which is associated with the start of a gstreamer stream. These timestamps are automatically created by gstreamer and describe the time the buffer was emitted by the tcamsrc.

## Capture Time

This requires Gstreamer version >= 1.14.

All GstBuffer contain a capture time that can be retrieved by calling [gst_buffer_get_reference_timestamp_meta ()](https://gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer/html/GstBuffer.html#gst-buffer-get-reference-timestamp-meta) for the associated buffer. The retrieved timestamp will tell you when the backend/driver on your computer captured the image.  
This timestamp will always be in nanoseconds.

This frame of reference will depend on the backend.  
If you are unsure which backend your camera is using, call `tcam-ctrl -l` in your terminal and check the 'Type' column.

#### V4L2 kernel driver

The V4L2 backend will deliver a timestamp which's point of reference is the boot time.  
Please be aware that suspend/hibernate will cause the clock to not run.

#### Aravis userspace library

The Aravis backend delivers the system wall-clock time of then the first bytes of the image where received.  
The point of reference if January 1, 1970 UTC.

## Camera Capture Time

This requires Gstreamer version >= 1.14.

GigE cameras deliver an additional timestamp that describes the time when the camera itself captured the image. You can retrieve this timestamp by calling [gst_buffer_get_reference_timestamp_meta ()](https://gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer/html/GstBuffer.html#gst-buffer-get-reference-timestamp-meta) with the required GstCaps reference set to NULL.

This timestamp will always be in nanoseconds.

The frame of reference is the boot time of the camera.  
Manually resetting the frame of reference is possible by calling the property 'TimestampReset'.
