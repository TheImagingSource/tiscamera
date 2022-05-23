
.. _error_handling:

##############
Error Handling
##############

This is a small overview as to how error handling might be handled.

GStreamer
#########

gst_element_set_state() will return a `GstStateChangeReturn <https://gstreamer.freedesktop.org/documentation/gstreamer/gstelement.html?gi-language=c#GstStateChangeReturn>`_ value.

.. tabs::

   .. group-tab:: c

      .. code-block:: c

         GstPipeline* pipeline = ...
         GstStateChangeReturn ret = gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING);

         if (ret == GST_STATE_CHANGE_SUCCESS)
         {
             // everything is wonderful
         }
         else if (ret == GST_STATE_CHANGE_ASYNC)
         {
             // elements are still working
             // call gst_element_get_state()
             // to get a final result
         }
         else if (ret == GST_STAT_CHANGE_FAILURE)
         {
             // The pipeline did not start
             // increase logging to find out what is happening
             // typical causes can be
             // - wrong caps
             // - elements cannot work with each other
             // - device cannot be opened
         }
         else
         {
             // this means a GST_STATE_CHANGE_NO_PREROLL happens.
             // This case can be ignored.
         }

   .. group-tab:: python

      .. code-block:: python

         pipeline = ...
         change_ret = pipeline.set_state(Gst.State.PLAYING)

         if change_ret == Gst.StateChange.SUCCESS:
             # everything is wonderful
             pass
         elif change_ret == Gst.StateChange.ASYNC:
             # elements are still working
             # call gst_element_get_state()
             # to get a final result 
             # to get a final result
             pass
         elif ret == GST_STAT_CHANGE_FAILURE:
             # The pipeline did not start
             # increase logging to find out what is happening
             # typical causes can be
             # - wrong caps
             # - elements cannot work with each other
             # - device cannot be opened
             pass
         else:
             # this means a Gst.StateChange.NO_PREROLL happens.
             # This case can be ignored.
             pass
         
tcam-property
#############

tcam-property handles error by returning a `GError <https://docs.gtk.org/glib/struct.Error.html>`_ to the user.
All possible error types are defined in the enum :ref:`tcamerror`

.. tabs::

   .. group-tab:: c

      .. code-block:: c

         // explicitly initialize to NULL
         // do this so that we can identify
         // error
         GError* err = NULL;

         // some tcam-property API call

         if (err) // an error occurred
         {
             if (err->domain != tcam_error_quark())
             {
                 // Not an error caused by tiscamera
                 // handling is outside of the scope of this example
                 return;
             }
         
             // err->message contains a human readable error description
             switch (err->code) // err->code contains the TcamError value
             {
                 case TCAM_ERROR_UNKNOWN:
                 {
                     break;
                 }
                 case TCAM_ERROR_PROPERTY_NOT_WRITEABLE:
                 {
                     break;
                 }
                 case TCAM_ERROR_NO_DEVICE_OPEN:
                 {
                     //
                     break;
                 }
                 case TCAM_ERROR_DEVICE_LOST:
                 {
                     printf("error: Device lost %s\n", err->message);
                     // stop playback and other things
                     break;
                 }
                 default:
                 {
                     printf("error: %s\n", err->message);
                     break;
                 }
             }
         }

   .. group-tab:: python
                  
      .. code-block:: python

         try:

             # some tcam-property call

         except GLib.Error as err:

             if err.code == Tcam.Error.DEVICE_LOST:
                 # stop playback and other things
                 
             print("Error: {}".format(err.message))

