.. _logging:

#######
Logging
#######

There are two separate logging functionalities one should be aware of:
The project internal functionality (TCAM_LOG) and the GStreamer-owned functionality.

TCAM_LOG
========

Our libraries offer logging for debugging purposes.
Currently, logging is only possible to the console.
Output redirects to files and user-specified callbacks
are planned but have not yet been implemented.

To enable logging, set the environment variable TCAM_LOG.
The following values are possible:

.. list-table:: Logging States
   :header-rows: 1
   :widths: 10 90

   * - Name
     - Description
   * - OFF
     - No logging
   * - TRACE
     - Gives as much information as possible.
   * - DEBUG
     - Information that is potentially useful to understand/debug a problem.
   * - INFO
     - Information that may be useful to the user under certain circumstances( e.g. Disabling a software feature because the used camera already offers a similar feature).
   * - WARNING
     - Events that might cause problems and are noticeable to the user but do not affect the general streaming capabilities of the camera.
   * - ERROR
     - Events that may cause a stream to end and other critical failures. The tcamsrc will see these as reasons to end streaming.

Per default, all logging is set to OFF.

To disable logging, unset the environment variable or set it to OFF.

GStreamer
=========

For a general overview, please refer to the `GStreamer documentation <https://gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer/html/gst-running.html>`_.

All The Imaging Source GStreamer elements are prefixed with 'tcam'.
Enabling logging for all our elements can be carried out like this:

.. code-block:: sh

    gst-launch-1.0 --gst-debug=tcam*:5 ....

This would set the log level of all our GStreamer elements to DEBUG.
For more precise logging, a comma separated list can be used.

.. code-block:: sh

    gst-launch-1.0 --gst-debug=tcamsrc:3,tcambin:5,tcamautofocus:2 ....

When logging to a file, it is generally recommended to disable color output.

.. code-block:: sh

   gst-launch-1.0 --gst-debug-no-color .....
