
# Logging

There are two separate logging functionalities you have to be aware of:
The project internal functionality and the gstreamer owned functionality.

## TCAM_LOG

Our libraries offer logging for debugging purposes.
Currently logging is only possible to the console.
Output redirects to files and user specified callbacks are planned but not implemented.

To enable logging set the environment variable TCAM_LOG.
The following values are possible:
OFF
TRACE
DEBUG
INFO
WARNING
ERROR

Per default all logging is set to OFF.
To disable logging unset the environment variable or set it to OFF.

## GStreamer

For a general overview, please refer to the gstreamer documentation here: https://gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer/html/gst-running.html .

All The Imaging Source gstreamer elements are prefixed with 'tcam'.
Enabling logging for all our elements can thus be done like this:

    gst-launch-1.0 --gst-debug=tcam*:5 ....

This would set the log level of all our gstreamer elements to DEBUG.
For a more precise logging, a comma separated list can be used.

    gst-launch-1.0 --gst-debug=tcamsrc:3,tcambin:5,tcamautofocus:2 ....
