
.. _error_handling:

##############
Error Handling
##############

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
