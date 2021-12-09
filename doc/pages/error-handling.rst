
.. _error_handling:

##############
Error Handling
##############

tiscamera


GStreamer
#########

gst_element_set_state() will return a GstStateChangeReturn value.

.. todo::

   add gstreamer documentation links

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

.. todo:: add link to tcamerror

tcam-property handles error by returning a GError to the user.
All possible error types are defined in the enum TcamError

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
             // err->message contains a human readable error description
             switch (err->code) // err->code contains the TcamError value
             {
             case TCAM_ERROR_UNKNOWN:
             {
                 break;
             }
             case TCAM_ERROR_PROPERTY_NOT_IMPLEMENTED:
             {
                 break;
             }
             case TCAM_ERROR_PROPERTY_NOT_AVAILABLE:
             {
                 break;
             }
             case TCAM_ERROR_PROPERTY_NOT_WRITEABLE:
             {
                 break;
             }
             case TCAM_ERROR_PROPERTY_TYPE_INCOMPATIBLE:
             {
                 break;
             }
             case TCAM_ERROR_PROPERTY_VALUE_OUT_OF_RANGE:
             {
                 break;
             }
             case TCAM_ERROR_NO_DEVICE_OPEN:
             {
             break;
             }
             case TCAM_ERROR_DEVICE_LOST:
             {
             break;
             }
             case TCAM_ERROR_PARAMETER_NULL:
             {
             break;
             }
             case TCAM_ERROR_PROPERTY_DEFAULT_NOT_AVAILABLE:
             {
             break;
             }
         }
         }

   .. group-tab:: python
                  
      .. code-block:: python

         try:

             # some tcam-property call

         except GLib.Error as err:

             err.code
             print("Error for {}: {}".format(name, err.message))


         
typedef enum {
TCAM_ERROR_SUCCESS                      = 0,
TCAM_ERROR_UNKNOWN                      = 1,
TCAM_ERROR_PROPERTY_NOT_IMPLEMENTED     = 2,
TCAM_ERROR_PROPERTY_NOT_AVAILABLE       = 3,
TCAM_ERROR_PROPERTY_NOT_WRITEABLE       = 4,
TCAM_ERROR_PROPERTY_TYPE_INCOMPATIBLE   = 5,
TCAM_ERROR_PROPERTY_VALUE_OUT_OF_RANGE  = 6,
TCAM_ERROR_NO_DEVICE_OPEN               = 7,
TCAM_ERROR_DEVICE_LOST                  = 8,
TCAM_ERROR_PARAMETER_NULL               = 9,
TCAM_ERROR_PROPERTY_DEFAULT_NOT_AVAILABLE = 10,
} TcamError;
