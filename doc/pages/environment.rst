###########
Environment
###########

This page describes environment variables that might be relevant when dealing with tiscamera.

Tiscamera
=========

The following environment variables are used by tiscamera.

TCAM_LOG
++++++++

.. code-block:: sh

   TCAM_LOG={ERROR|WARNING|INFO|DEBUG|TRACE|OFF}
   # write to file instead
   TCAM_LOG_FILE=/tmp/tcam.log
   # disable log colors
   TCAM_LOG_NO_COLOR=1

TCAM_GIGE_PACKET_SIZE
+++++++++++++++++++++

`TCAM_GIGE_PACKET_SIZE` allows a manual overwrite of the largest allowed packet size for
GigE packages. This value is normally auto-negotiated and does not have to be set.
The value is in bytes.

.. code-block:: sh

   export TCAM_GIGE_PACKET_SIZE=9000

TCAM_ARV_PACKET_REQUEST_RATIO
+++++++++++++++++++++++++++++

`TCAM_ARV_PACKET_REQUEST_RATIO` allows setting the packet resend request limit
as a percentage of frame packet number. See “packet-request-ratio” property of
arvgvstream.
The value has to be between 0.0 and 1.0.

.. code-block:: sh

   export TCAM_ARV_PACKET_REQUEST_RATIO=1.0

.. _env_gstreamer:
 
GStreamer
=========

Please refer to `the GStreamer documentation of environment variables <https://developer.gnome.org/gstreamer/stable/gst-running.html>`_ for a full overview.


The following variables are used to configure the GStreamer-1.0 behavior.

GST_PLUGIN_PATH_1_0
+++++++++++++++++++

To add additional directories to the GStreamer search path, set GST_PLUGIN_PATH_1_0.

.. code-block:: sh
   
   export GST_PLUGIN_PATH_1_0="/home/user/tiscamera/build/src/gstreamer-1.0/:${GST_PLUGIN_SYSTEM_PATH_1_0}"

GST_DEBUG
+++++++++


.. code-block:: sh

   GST_DEBUG=tcam*:5
   # if set output will not contain color codes
   GST_DEBUG_NO_COLOR=1
   # log all output to this file
   GST_DEBUG_FILE=/tmp/tcam-gst.log
   # for separate log files an own handler has to be implemented

GOBJECT
+++++++

To allow for simpler debugging, set the variable `G_DEBUG`. to one of the following values.
For more information, read the `GLib documentation <https://developer.gnome.org/glib/2.28/glib-running.html>`_.

.. code-block:: sh

   export G_DEBUG=fatal-warnings

To index additional directories, set the environment variable `GI_TYPELIB_PATH`.

.. code-block:: sh

   export GI_TYPELIB_PATH=/home/user/tiscamera/build/src/gobject/
