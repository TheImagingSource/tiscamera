.. _logging:

#######
Logging
#######

All tiscamera logging is done via GStreamer

Categories
==========

tiscamera generally writes logs to a category named after the correlating element, e.g. tcamsrc, tcambin, tcamconvert.

Additionally the category `tcam-libtcam` exists. It offers additional logging entries for the backend. 

GStreamer
=========

For a general overview, please refer to the `GStreamer documentation <https://gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer/html/gst-running.html>`_.

All The Imaging Source GStreamer elements are prefixed with 'tcam'.
Enabling logging for all our elements can be carried out like this:

.. code-block:: sh

    gst-launch-1.0 --gst-debug=tcam*:5 ....

or by defining a environment variable via

.. code-block:: sh

    export GST_DEBUG=tcam*:5


This would set the log level of all our GStreamer elements to DEBUG.
For more precise logging, a comma separated list can be used.

.. code-block:: sh

    gst-launch-1.0 --gst-debug=tcamsrc:3,tcambin:5,tcam-libtcam:2 ....

To log to a file set the environment variable `GST_DEBUG_FILE`.

.. code-block:: sh

   export GST_DEBUG_FILE=./gstreamer-output.log
    
When logging to a file, it is generally recommended to disable color output.

.. code-block:: sh

   gst-launch-1.0 --gst-debug-no-color .....


   
Aravis
======

In some cases it might be useful to retrieve log output from aravis itself.

For a description of the details visit the `aravis documentation <https://aravisproject.github.io/docs/aravis-0.8/aravis-building.html>`_.
