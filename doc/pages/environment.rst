.. _environment:

###########
Environment
###########

This page describes environment variables that might be relevant when dealing with tiscamera.

TCAM_GIGE_PACKET_SIZE
+++++++++++++++++++++

`TCAM_GIGE_PACKET_SIZE` allows a manual overwrite of the largest allowed packet size for
GigE packages. This value is normally auto-negotiated and does not have to be set.
The value is in bytes.

.. code-block:: sh

   export TCAM_GIGE_PACKET_SIZE=9000

TCAM_GIGE_HEARTBEAT_MS
++++++++++++++++++++++

`TCAM_GIGE_HEARTBEAT_MS` allows manual overwrite of the periodic lifetime checks for GigE cameras.
The default is 3 seconds. Is should only be set for debugging purposes.
The value is in milliseconds.

.. code-block:: sh

   # Set timeout to 10 seconds
   export TCAM_GIGE_HEARTBEAT_MS=10000
   
TCAM_ARV_STREAM_OPTIONS
+++++++++++++++++++++++
`TCAM_ARV_STREAM_OPTIONS` allows setting all options for the arvstream object.

For an overview over available options, please look at `the official aravis documentation <https://aravisproject.github.io/docs/aravis-0.8/ArvGvStream.html>`_.

.. code-block:: sh

   export TCAM_ARV_STREAM_OPTIONS=packet-resend-ratio=0.8,packet-timeout=20000,packet-resend=ARV_GV_STREAM_PACKET_RESEND_NEVER

Enumerations use the complete enumeration value.
   
TCAM_UVC_EXTENSION_DIR
++++++++++++++++++++++

Define additional directories where tcam-uvc-extension-loader should look to extension unit description files.

.. code-block:: sh

   export TCAM_UVC_EXTENSION_DIR=/home/user/share/uvc-extensions/

TCAM_DISABLE_DEVICE_BLACKLIST
+++++++++++++++++++++++++++++

When set internal device filtering for legacy devices will be disabled.  
Usage at own risk.   
Devices enabled through this method are not tested and unsupported.

.. code-block:: sh

   export TCAM_DISABLE_DEVICE_BLACKLIST=1

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
