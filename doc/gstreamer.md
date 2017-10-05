
# Gstreamer
@addtogroup gstreamer

Gstreamer is a general purpose multimedia framework. It is the recommended way
to interact with your device.

## Imaging Source Gstreamer Plugins

#### tcamsrc

Source elements that retrieves images from you device.

#### tcamautoexposure

Automatically adjust exposure and gain to reach the wished image brightness.

#### tcamwhitebalance

Color correction for bayer images.

#### tcamautofocus

Allows for cameras with focus elements to automatically adjust focus.

#### tcambin

Wrapper around all the previous elements, allowing for an easy all in one handling.

## Useful modules

#### clockovlerlay

Allows overlays of timestamps or duration's.

#### capssetter

Capssetter allows you to overwrite existing caps or to add information to existing ones.

#### tee

Tee allows you to send a buffer to multiple modules at once.
A typical example would be to send a videostream to a window for live preview
and to additionally process it in the background.

After adding tee always use a queue to ensure that the following pipeline runs
in its own thread.

For further info please consult the
[gstreamer documentation](https://gstreamer.freedesktop.org/data/doc/gstreamer/head/manual/html/section-threads-uses.html).

#### valve


#### videorate

Change the framerate by duplicating or dropping frames.


## Debugging

For further info please consult the
[gstreamer documentation](https://gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer/html/gst-running.html).

## Environment

To add additional directories to the gstreamer search path you need to set
GST_PLUGIN_SYSTEM_PATH_1_0.

    export GST_PLUGIN_SYSTEM_PATH_1_0="/home/user/tiscamera/build/src/gstreamer-1.0/:${GST_PLUGIN_SYSTEM_PATH_1_0}"
