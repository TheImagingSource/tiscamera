#############
Tcam-Property
#############


This page describes the official gobject-introspection API.

.. note::
   The current version of this API is 1.0

Helper Types
############
   
TcamError
---------

.. c:enum:: TcamError

   Enumeration containing all possible error types tcam-property will return.
            
   .. c:enumerator:: TCAM_ERROR_SUCCESS                     
   .. c:enumerator:: TCAM_ERROR_UNKNOWN                     
   .. c:enumerator:: TCAM_ERROR_PROPERTY_NOT_IMPLEMENTED    
   .. c:enumerator:: TCAM_ERROR_PROPERTY_NOT_AVAILABLE      
   .. c:enumerator:: TCAM_ERROR_PROPERTY_NOT_WRITEABLE

      | The property is either read only or temporarily locked.
      | Call :c:func:`tcam_property_base_is_locked` for verification.
                     
   .. c:enumerator:: TCAM_ERROR_PROPERTY_TYPE_INCOMPATIBLE

      The property is of a different type.
                     
   .. c:enumerator:: TCAM_ERROR_PROPERTY_VALUE_OUT_OF_RANGE 
   .. c:enumerator:: TCAM_ERROR_NO_DEVICE_OPEN
      
      No device has been opened that can offer properties.
      This typically means the GstElement is not in GST_STATE_READY or higher.
      
   .. c:enumerator:: TCAM_ERROR_DEVICE_LOST

      | The device has been lost.
      | This should be considered a fatal, unrecoverable error.
                     
   .. c:enumerator:: TCAM_ERROR_PARAMETER_NULL

      | One of the given arguments is NULL.
      | Are provider/property pointer valid?
      | Is the name a valid string?

   .. c:enumerator:: TCAM_ERROR_PROPERTY_DEFAULT_NOT_AVAILABLE

.. c:enum:: TcamPropertyVisibility

   .. c:enumerator:: \
      TCAM_PROPERTY_VISIBILITY_BEGINNER
      TCAM_PROPERTY_VISIBILITY_EXPERT
      TCAM_PROPERTY_VISIBILITY_GURU
      TCAM_PROPERTY_VISIBILITY_INVISIBLE 

.. c:enum:: TcamPropertyIntRepresentation

   Enumeration describing recommendations on how the property should be represented.

   .. c:enumerator:: \
      TCAM_PROPERTY_INTREPRESENTATION_LINEAR
      TCAM_PROPERTY_INTREPRESENTATION_LOGARITHMIC 
      TCAM_PROPERTY_INTREPRESENTATION_PURENUMBER
      TCAM_PROPERTY_INTREPRESENTATION_HEXNUMBER 
            
.. c:enum:: TcamPropertyFloatRepresentation

   Enumeration describing recommendations on how the property should be represented.

   .. c:enumerator:: \
      TCAM_PROPERTY_FLOATREPRESENTATION_LINEAR
      TCAM_PROPERTY_FLOATREPRESENTATION_LOGARITHMIC
      TCAM_PROPERTY_FLOATREPRESENTATION_PURENUMBER 

.. c:enum:: TcamPropertyType

   Enumeration containing all possible property types.
            
   .. c:enumerator:: \
      TCAM_PROPERTY_TYPE_INTEGER
      TCAM_PROPERTY_TYPE_FLOAT
      TCAM_PROPERTY_TYPE_ENUMERATION
      TCAM_PROPERTY_TYPE_BOOLEAN
      TCAM_PROPERTY_TYPE_COMMAND

   
External Types
##############

All tiscamera gstreamer elements implement the :c:type:`TcamPropertyProvider` interface.
This interface allows access to all properties that the camera and software offer.

.. c:type:: GSList

    In tcamprop this is always a list with element-type utf8 which has to be deallocated via:

    Example:

    .. code-block:: c

        GSList* list = tcam_prop_get_device_serials( self );
        
        // ... do sth with list
        
        g_slist_free_full( list, ::g_free );

.. c:type:: GError

   GObject error reporting mechanism.

   A returned GError has to _always_ be freed by the user with g_error_free().
   The GError will contain a string describing the cause of the error and an error code.
   The message can be accessed through the member variable `message`.
   The error code can be accessed though the member variable `code`.
   The error code will be a :c:enum:`TcamError` enum entry.
        
.. c:type:: GValue

    GObject based variant type, used as arguments.
    
    Note: If you receive out-parameter with this, the caller is responsible of clearing the contents via :cpp:texpr:`g_value_unset( &var )`

    Example:

    .. code-block:: c

        GValue value = G_VALUE_INIT;
        GValue group = G_VALUE_INIT;

        gboolean res = tcam_prop_get_tcam_property( self, "Gain", &value, ..., &group );
        if( res ) {
            // ... stuff
        }
        g_value_unset( &value );
        g_value_unset( &group );

        
TcamPropertyProvider
####################

.. c:type:: TcamPropertyProvider

   This object is typically a converted gstreamer element like tcambin, tcamsrc or tcamdutils.

tcam_property_provider_get_tcam_property_names
----------------------------------------------

.. c:function:: GSList* tcam_property_provider_get_tcam_property_names(TcamPropertyProvider* self, GError** err)

    Retrieve a list of all currently available properties. GstElement must be `GST_STATE_READY` or higher.

    :param self: a TcamPropertyProvider  
    :param err: a :c:type:`GError` pointer, may be NULL

    :returns: (element-type utf8) (transfer full): a #GSList
    :retval GSList*: a single linked list containing strings with property names
    :retval NULL: If an error occurs, NULL will be returned


    .. tabs::

       .. group-tab:: c

          .. code-block:: c

             GstElement* source = gst_element_factory_make("tcambin", "source");

             gst_element_set_state(source, GST_STATE_READY);
             
             GSList* property_names = tcam_property_provider_get_names(TCAM_PROPERTY_PROVIDER(source));

             // free GSList and all contained strings
             g_slist_free_full(property_names, g_free);

             gst_element_set_state(source, GST_STATE_NULL);
             
             // free GstElement
             gst_object_unref(source);

       .. group-tab:: python
             
          .. code-block:: python

             # nothing to do
             # python cleans up automatically

             TcamPropertyBase*   tcam_property_provider_get_tcam_property( TcamPropertyProvider* self, const gchar* name, GError** err );

tcam_property_provider_get_tcam_property
----------------------------------------


.. c:function:: TcamPropertyBase*   tcam_property_provider_get_tcam_property( TcamPropertyProvider* self, const gchar* name, GError** err );
                
   :param self: a TcamPropertyProvider
   :param name: a string pointer, naming the property that shall be set.
   :param err: pointer for error retrieval, may be NULL
   :return: a TcamPropertyBase pointer
   :retval: a valid TcamPropertyBase instance
   :retval: NULL in case of an error. Check err.
               
tcam_property_provider_set_tcam_boolean
---------------------------------------

Convenience function to set the value of a boolean.

For complex applications it is recommended to use a :c:type:`TcamPropertyBoolean` instance instead.
                
.. c:function:: void        tcam_property_provider_set_tcam_boolean( TcamPropertyProvider* self, const gchar* name, gboolean value, GError** err );

   :param self: a TcamPropertyProvider
   :param name: a string pointer, naming the property that shall be set.
   :param value: a boolean with the value that shall be set
   :param err: pointer for error retrieval, may be NULL
                
tcam_property_provider_set_tcam_integer
---------------------------------------

Convenience function to set the value of an integer.

For complex applications it is recommended to use a :c:type:`TcamPropertyInteger` instance instead.

.. c:function:: void        tcam_property_provider_set_tcam_integer( TcamPropertyProvider* self, const gchar* name, gint64 value, GError** err );

   :param self: a TcamPropertyProvider
   :param name: a string pointer, naming the property that shall be set.
   :param value: an integer with the value that shall be set
   :param err: pointer for error retrieval, may be NULL

tcam_property_provider_set_tcam_float
-------------------------------------

Convenience function to set the value of a float.

For complex applications it is recommended to use a :c:type:`TcamPropertyFloat` instance instead.
               
.. c:function:: void        tcam_property_provider_set_tcam_float( TcamPropertyProvider* self, const gchar* name, gdouble value, GError** err );

   :param self: a TcamPropertyProvider
   :param name: a string pointer, naming the property that shall be set.
   :param value: a double with the value that shall be set
   :param err: pointer for error retrieval, may be NULL

tcam_property_provider_set_tcam_enumeration
-------------------------------------------

Convenience function to set the value of an enum.

For complex applications it is recommended to use a :c:type:`TcamPropertyEnumeration` instance instead.
               
.. c:function:: void        tcam_property_provider_set_tcam_enumeration( TcamPropertyProvider* self, const gchar* name, const gchar* value, GError** err );

   :param self: a TcamPropertyProvider
   :param name: a string pointer, naming the property that shall be set.
   :param value: a string with the value that shall be set
   :param err: pointer for error retrieval, may be NULL
                
.. c:function:: void        tcam_property_provider_set_tcam_command( TcamPropertyProvider* self, const gchar* name, GError** err );

.. c:function:: gboolean    tcam_property_provider_get_tcam_boolean( TcamPropertyProvider* self, const gchar* name, GError** err );
.. c:function:: gint64      tcam_property_provider_get_tcam_integer( TcamPropertyProvider* self, const gchar* name, GError** err );
.. c:function:: gdouble     tcam_property_provider_get_tcam_float( TcamPropertyProvider* self, const gchar* name, GError** err );
.. c:function:: gchar*      tcam_property_provider_get_tcam_enumeration( TcamPropertyProvider* self, const gchar* name, GError** err );


TcamPropertyBase
################



.. c:type:: TcamPropertyBase
            
.. py:class:: TcamPropertyBase

   Base class for all properties. Can be cast into different derived classes.
   Check the property type via :c:func:`tcam_property_base_get_property_type` to ensure the correct cast will be used.

.. c:function:: const gchar* tcam_property_base_get_name( TcamPropertyBase* self );

   :param self: Pointer to the property instance
   :returns: Name of the property
   :retval: const gchar*, string containing the name

   The property owns the string. It will be freed once the property is destroyed.
                
.. c:function:: const gchar* tcam_property_base_get_display_name( TcamPropertyBase* self );

   :param self: Pointer to the property instance
   :returns: Name of the property
   :retval: const gchar*, string containing the display name
                
   | The property owns the string. It will be freed once the property is destroyed.
   |
   | The display name is a human readable name intended for GUIs and similar interfaces.

""""""""""""""

.. c:function:: const gchar* tcam_property_base_get_description( TcamPropertyBase* self );

   :param self: Pointer to the property instance
   :returns: Name of the property
   :retval: const gchar*, string containing the description

   The property owns the string. It will be freed once the property is destroyed.

""""""""""""""
   
.. c:function:: const gchar* tcam_property_base_get_category( TcamPropertyBase* self );

   :param self: Pointer to the property instance
   :returns: Name of the property
   :retval: const gchar*, string containing the category

   The property owns the string. It will be freed once the property is destroyed.

^^^^^
   
.. c:function:: TcamPropertyVisibility tcam_property_base_get_visibility( TcamPropertyBase* self );
.. c:function:: TcamPropertyType tcam_property_base_get_property_type( TcamPropertyBase* self );
.. c:function:: gboolean tcam_property_base_is_available( TcamPropertyBase* self, GError** err );
.. c:function:: gboolean tcam_property_base_is_locked( TcamPropertyBase* self, GError** err );

TcamPropertyBoolean
###################

.. c:type:: TcamPropertyBoolean

Inherits from :c:type:`TcamPropertyBase`.
Can be obtained by casting a :c:type:`TcamPropertyBase` with :c:macro:`TCAM_PROPERTY_BOOLEAN(TcamPropertyBase*)`.
                
.. c:function:: gboolean tcam_property_boolean_get_value( TcamPropertyBoolean* self, GError** err );

^^^^^

.. c:function:: void tcam_property_boolean_set_value( TcamPropertyBoolean* self, gboolean value, GError** err );

^^^^^
                
.. c:function:: gboolean tcam_property_boolean_get_default( TcamPropertyBoolean* self, GError** err );

TcamPropertyInteger
###################

.. c:type:: TcamPropertyInteger
            
.. c:function:: gint64 tcam_property_integer_get_value( TcamPropertyInteger* self, GError** err );
.. c:function:: void tcam_property_integer_set_value( TcamPropertyInteger* self, gint64 value, GError** err );
.. c:function:: void tcam_property_integer_get_range( TcamPropertyInteger* self, gint64* min_value, gint64* max_value, gint64* step_value, GError** err );
.. c:function:: gint64 tcam_property_integer_get_default( TcamPropertyInteger* self, GError** err );
.. c:function:: const gchar* tcam_property_integer_get_unit( TcamPropertyInteger* self );
.. c:function:: TcamPropertyIntRepresentation tcam_property_integer_get_representation( TcamPropertyInteger* self );

TcamPropertyFloat
#################

.. c:type:: TcamPropertyFloat

^^^^^
            
.. c:function:: gdouble tcam_property_float_get_value( TcamPropertyFloat* self, GError** err );

^^^^^
                
.. c:function:: void tcam_property_float_set_value( TcamPropertyFloat* self, gdouble value, GError** err );

^^^^^
                
.. c:function:: void tcam_property_float_get_range( TcamPropertyFloat* self, gdouble* min_value, gdouble* max_value, gdouble* step_value, GError** err );

^^^^^
                
.. c:function:: gdouble tcam_property_float_get_default( TcamPropertyFloat* self,GError** err );

^^^^^
                
.. c:function:: const gchar* tcam_property_float_get_unit( TcamPropertyFloat* self );

^^^^^
                
.. c:function:: TcamPropertyFloatRepresentation tcam_property_float_get_representation( TcamPropertyFloat* self );

TcamPropertyEnumeration
#######################

.. c:type:: TcamPropertyEnumeration

^^^^^
            
.. c:function:: gchar* tcam_property_enumeration_get_value( TcamPropertyEnumeration* self, GError** err );

   The caller takes ownership of the returned value.

^^^^^

.. c:function:: void tcam_property_enumeration_set_value( TcamPropertyEnumeration* self, const gchar* value, GError** err );

^^^^^
                
.. c:function:: GSList* tcam_property_enumeration_get_enum_entries( TcamPropertyEnumeration* self, GError** err );

   The caller takes ownership of the returned list and its values.

^^^^^

.. c:function:: gchar* tcam_property_enumeration_get_default( TcamPropertyEnumeration* self, GError** err );

TcamPropertyCommand
###################

.. c:type:: TcamPropertyCommand

^^^^^

.. c:function:: void tcam_property_command_set_command( TcamPropertyCommand* self, GError** err );
