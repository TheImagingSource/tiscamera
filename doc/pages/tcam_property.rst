#############
tcam-property
#############


This page describes the official gobject-introspection API.

.. note::
   The current version of this API is 1.0

.. contents:: Table of Contents
              :depth: 5

   
Helper Types
############

.. _tcamerror:

TcamError
---------

TcamError is the tcam-property enumeration that contains all potential error tcam-property implementations might produce.
This does not mean that other error might not also occur.

.. cpp:enum:: TcamError

   Enumeration containing all possible error types tcam-property will return.
            
   .. cpp:enumerator:: TCAM_ERROR_SUCCESS

      | Should not be encountered.
      | Describes `no error` state.
                       
   .. cpp:enumerator:: TCAM_ERROR_UNKNOWN

      | Catch all error code for things that do not fit other codes.
                       
   .. cpp:enumerator:: TCAM_ERROR_PROPERTY_NOT_IMPLEMENTED    
   .. cpp:enumerator:: TCAM_ERROR_PROPERTY_NOT_AVAILABLE

      | Circumstances prevent this property from being usable.
      | This is typically due to the selected stream format.
      | e.g. BalanceWhite* not being usable when streaming mono.
                       
   .. cpp:enumerator:: TCAM_ERROR_PROPERTY_NOT_WRITEABLE

      | The property is either read only or temporarily locked.
      | Call :ref:`tcam_property_base_is_locked` for verification.
                     
   .. cpp:enumerator:: TCAM_ERROR_PROPERTY_TYPE_INCOMPATIBLE

      The property is of a different type.
                     
   .. cpp:enumerator:: TCAM_ERROR_PROPERTY_VALUE_OUT_OF_RANGE

      | Value is out of bounds.
      | Check the `*_get_range` function for boundaries.
                       
   .. cpp:enumerator:: TCAM_ERROR_NO_DEVICE_OPEN
      
      | No device has been opened that can offer properties.
      | This typically means the GstElement is not in GST_STATE_READY or higher.
      
   .. cpp:enumerator:: TCAM_ERROR_DEVICE_LOST

      | The device has been lost.
      | This should be considered a fatal, unrecoverable error.
                     
   .. cpp:enumerator:: TCAM_ERROR_PARAMETER_NULL

      | One of the given arguments is NULL.
      | Are provider/property pointer valid?
      | Is the name a valid string?

   .. cpp:enumerator:: TCAM_ERROR_PROPERTY_DEFAULT_NOT_AVAILABLE

      | Property offers no default value.

.. _tcampropertyvisibility:
      
TcamPropertyVisibility
----------------------
                       
.. cpp:enum:: TcamPropertyVisibility

   .. cpp:enumerator:: TCAM_PROPERTY_VISIBILITY_BEGINNER

      Should always be displayed.
                       
   .. cpp:enumerator:: TCAM_PROPERTY_VISIBILITY_EXPERT

      Should only be displayed to users, who know what they are doing.
                       
   .. cpp:enumerator:: TCAM_PROPERTY_VISIBILITY_GURU

      Should only be displayed to users, who really know what they are doing.
                          
   .. cpp:enumerator:: TCAM_PROPERTY_VISIBILITY_INVISIBLE

      Should never be displayed.

.. _tcampropertyintrepresentation:
                       
TcamPropertyIntRepresentation
-----------------------------
                       
.. cpp:enum:: TcamPropertyIntRepresentation

   Enumeration describing recommendations on how the property should be represented.

   .. cpp:enumerator:: TCAM_PROPERTY_INTREPRESENTATION_LINEAR

      Property is best displayed with a linear slider.
                       
   .. cpp:enumerator:: TCAM_PROPERTY_INTREPRESENTATION_LOGARITHMIC

      Property is best displayed with a logarithmic slider.
                       
   .. cpp:enumerator:: TCAM_PROPERTY_INTREPRESENTATION_PURENUMBER

      Property is best displayed with an edit box (e.g. QSpinBox, Gtk SpinButton).
                       
   .. cpp:enumerator:: TCAM_PROPERTY_INTREPRESENTATION_HEXNUMBER

      Same as pure number but with hexadecimal values.

.. _tcampropertyfloatrepresentation:
      
TcamPropertyFloatRepresentation
-------------------------------
      
.. cpp:enum:: TcamPropertyFloatRepresentation

   Enumeration describing recommendations on how the property should be represented.

   .. cpp:enumerator:: TCAM_PROPERTY_FLOATREPRESENTATION_LINEAR

      Property is best displayed with a linear slider.
                       
   .. cpp:enumerator:: TCAM_PROPERTY_FLOATREPRESENTATION_LOGARITHMIC

      Property is best displayed with a logarithmic slider.

   .. cpp:enumerator:: TCAM_PROPERTY_FLOATREPRESENTATION_PURENUMBER

      Property is best displayed with an edit box (e.g. QSpinBox, Gtk SpinButton).

.. _tcampropertytype:
      
TcamPropertyType
----------------
      
.. cpp:enum:: TcamPropertyType

   Enumeration containing all possible property types.
            
   .. cpp:enumerator:: TCAM_PROPERTY_TYPE_INTEGER
   .. cpp:enumerator:: TCAM_PROPERTY_TYPE_FLOAT
   .. cpp:enumerator:: TCAM_PROPERTY_TYPE_ENUMERATION
   .. cpp:enumerator:: TCAM_PROPERTY_TYPE_BOOLEAN
   .. cpp:enumerator:: TCAM_PROPERTY_TYPE_COMMAND

   
External Types
##############

All tiscamera gstreamer elements implement the :c:type:`TcamPropertyProvider` interface.
This interface allows access to all properties that the camera and software offer.

.. _gslist:

GSList
------

.. c:type:: GSList

    In tcamprop this is always a list with element-type utf8 which has to be deallocated via:

    Example:

    .. code-block:: c

        GSList* list = tcam_prop_get_device_serials (self);
        
        // ... do sth with list
        
        g_slist_free_full (list, ::g_free);

.. _gerror:
        
GError
------
        
.. c:type:: GError

   GObject error reporting mechanism.

   A returned GError has to _always_ be freed by the user with g_error_free().
   The GError will contain a string describing the cause of the error and an error code.
   The message can be accessed through the member variable `message`.
   The error code can be accessed though the member variable `code`.
   The error code will be a :cpp:enum:`TcamError` enum entry.

.. _tcampropertyprovider:
        
TcamPropertyProvider
####################

This object is typically a casted gstreamer element like :ref:`tcambin`, :ref:`tcamsrc` or :ref:`tcamdutils`.

Properties require the GStreamer element to be at least in the state `GST_STATE_READY`.

| Properties will become invalid once the GStreamer element enters the state `GST_STATE_NULL`.
| In such a case :cpp:enumerator:`TCAM_ERROR_NO_DEVICE_OPEN` will be returned.

.. _tcam_property_provider_get_tcam_property_names:
   
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

         source = Gst.ElementFactory.make("tcambin", "source")
         source.set_state(Gst.State.READY)

         try:
             names = source.get_tcam_property_names()
         except GLib.Error as err:
             # error handling
                          
         # nothing to do for memory management
         # python cleans up automatically



         
.. _tcam_property_provider_get_tcam_property:
             
tcam_property_provider_get_tcam_property
----------------------------------------


.. c:function:: TcamPropertyBase*   tcam_property_provider_get_tcam_property (TcamPropertyProvider* self, const gchar* name, GError** err);
                
:param self: a TcamPropertyProvider
:param name: a string pointer, naming the property that shall be set.
:param err: pointer for error retrieval, may be NULL
:return: a TcamPropertyBase pointer
:retval: a valid TcamPropertyBase instance
:retval: NULL in case of an error. Check err.


.. tabs::

   .. group-tab:: c

      .. code-block:: c

         GstElement* tcambin = ....
         GError* err = NULL;

         TcamPropertyBase* base_property = tcam_property_provider_get_tcam_property(TCAM_PROPERTY_PROVIDER(tcambin), "name", &err);

         if (!base_provider)
         {
             if (err)
             {
                 // error handling
             }
         }

         // no error
         // do property handling
            
         if (base_property)
         {
             g_object_unref(base_property);
             base_property = NULL;
         }

   .. group-tab:: python

      .. code-block:: python
                     
         tcambin = ....
         try:
             property = tcambin.get_tcam_property("name")
         except GLib.Error as err:
             # error handling





             
.. _tcam_property_provider_set_tcam_boolean:
                
tcam_property_provider_set_tcam_boolean
---------------------------------------

Convenience function to set the value of a boolean.

For complex applications it is recommended to use a :c:type:`TcamPropertyBoolean` instance instead.
                
.. c:function:: void tcam_property_provider_set_tcam_boolean (TcamPropertyProvider* self, const gchar* name, gboolean value, GError** err);

:param self: a TcamPropertyProvider
:param name: a string pointer, naming the property that shall be set.
:param value: a boolean with the value that shall be set
:param err: pointer for error retrieval, may be NULL

.. tabs::

   .. group-tab:: c

      .. code-block:: c

         GstElement* tcambin = ....
         GError* err = NULL;
         gboolean value = TRUE;

         tcam_property_provider_set_tcam_boolean(TCAM_PROPERTY_PROVIDER(tcambin), "name", value, &err);

         if (err)
         {
             // error handling
         }

   .. group-tab:: python

      .. code-block:: python
                  
         tcambin = ....
         value = True
            
         try:
             tcambin.set_tcam_boolean("name", value)
         except GLib.Error as err:
             # error handling


             

.. _tcam_property_provider_set_tcam_integer:
                
tcam_property_provider_set_tcam_integer
---------------------------------------

Convenience function to set the value of an integer.

For complex applications it is recommended to use a :c:type:`TcamPropertyInteger` instance instead.

.. c:function:: void tcam_property_provider_set_tcam_integer (TcamPropertyProvider* self, const gchar* name, gint64 value, GError** err);

:param self: a TcamPropertyProvider
:param name: a string pointer, naming the property that shall be set.
:param value: an integer with the value that shall be set
:param err: pointer for error retrieval, may be NULL

.. tabs::

   .. group-tab:: c

      .. code-block:: c

         GstElement* tcambin = ....
         GError* err = NULL;
         int value = 500;

         tcam_property_provider_set_tcam_integer(TCAM_PROPERTY_PROVIDER(tcambin), "name", value, &err);
         
         if (err)
         {
             // error handling
         }

   .. group-tab:: python

      .. code-block:: python
      
         tcambin = ....
         value = 500
            
         try:
             tcambin.set_tcam_integer("name", value)
         except GLib.Error as err:
             # error handling

.. _tcam_property_provider_set_tcam_float:
                
tcam_property_provider_set_tcam_float
-------------------------------------

Convenience function to set the value of a float.

For complex applications it is recommended to use a :c:type:`TcamPropertyFloat` instance instead.
               
.. c:function:: void tcam_property_provider_set_tcam_float (TcamPropertyProvider* self, const gchar* name, gdouble value, GError** err);

:param self: a TcamPropertyProvider
:param name: a string pointer, naming the property that shall be set.
:param value: a double with the value that shall be set
:param err: pointer for error retrieval, may be NULL

.. tabs::

   .. group-tab:: c

      .. code-block:: c

         GstElement* tcambin = ....
         GError* err = NULL;
         double value = 3000.0;
         
         tcam_property_provider_set_tcam_float(TCAM_PROPERTY_PROVIDER(tcambin), "name", value, &err);

         if (err)
         {
             // error handling
         }

   .. group-tab:: python

      .. code-block:: python
                  
         tcambin = ....
         value = 3000.0
         try:
             tcambin.set_tcam_float("name", value)
         except GLib.Error as err:
             # error handling



             
.. _tcam_property_provider_set_tcam_enumeration:
                
tcam_property_provider_set_tcam_enumeration
-------------------------------------------

Convenience function to set the value of an enum.

For complex applications it is recommended to use a :c:type:`TcamPropertyEnumeration` instance instead.



.. c:function:: void tcam_property_provider_set_tcam_enumeration (TcamPropertyProvider* self, const gchar* name, const gchar* value, GError** err);

:param self: a TcamPropertyProvider
:param name: a string pointer, naming the property that shall be set.
:param value: a string with the value that shall be set
:param err: pointer for error retrieval, may be NULL

.. tabs::

   .. group-tab:: c

      .. code-block:: c

         GstElement* tcambin = ....
         GError* err = NULL;
         const char* value = "entry";

         tcam_property_provider_set_tcam_enumeration(TCAM_PROPERTY_PROVIDER(tcambin), "name", value, &err);

         if (err)
         {
             // error handling
         }

   .. group-tab:: python

      .. code-block:: python
                  
         tcambin = ....
         value = "entry"
            
         try:
             tcambin.set_tcam_enumeration("name", value)
         except GLib.Error as err:
             # error handling




.. _tcam_property_provider_set_tcam_command:
                
tcam_property_provider_set_tcam_command
---------------------------------------
                
.. c:function:: void tcam_property_provider_set_tcam_command (TcamPropertyProvider* self, const gchar* name, GError** err);

:param self: a TcamPropertyProvider
:param name: a string pointer, naming the property that shall be set.
:param err: pointer for error retrieval, may be NULL
                
.. tabs::

   .. group-tab:: c

      .. code-block:: c

         GstElement* tcambin = ....
         GError* err = NULL;

         tcam_property_provider_set_tcam_command(TCAM_PROPERTY_PROVIDER(tcambin), "name", &err);

         if (err)
         {
             // error handling
         }

   .. group-tab:: python

      .. code-block:: python
                  
         tcambin = ....

         try:
             tcambin.set_tcam_command("name")
         except GLib.Error as err:
             # error handling


.. _tcam_property_provider_get_tcam_boolean:
                
tcam_property_provider_get_tcam_boolean
---------------------------------------
                
.. c:function:: gboolean tcam_property_provider_get_tcam_boolean (TcamPropertyProvider* self, const gchar* name, GError** err);

:param self: a TcamPropertyProvider
:param name: a string pointer, naming the property that shall be queried.
:param err: pointer for error retrieval, may be NULL
:returns: value of the boolean property
:retval: gboolean
                
.. tabs::

   .. group-tab:: c

      .. code-block:: c

         GstElement* tcambin = ....
         GError* err = NULL;

         gboolean value = tcam_property_provider_get_tcam_boolean(TCAM_PROPERTY_PROVIDER(tcambin), "name", &err);

         if (err)
         {
            // error handling
         }

   .. group-tab:: python

      .. code-block:: python
                  
         tcambin = ....
         value = True
         try:
             value = tcambin.get_tcam_boolean("name")
         except GLib.Error as err:
             # error handling

                
.. _tcam_property_provider_get_tcam_integer:
                
tcam_property_provider_get_tcam_integer
---------------------------------------
                
.. c:function:: gint64 tcam_property_provider_get_tcam_integer (TcamPropertyProvider* self, const gchar* name, GError** err);

:param self: a TcamPropertyProvider
:param name: a string pointer, naming the property that shall be queried.
:param err: pointer for error retrieval, may be NULL
:returns: value of the integer property
:retval: gint64
                
.. tabs::

   .. group-tab:: c

      .. code-block:: c

         GstElement* tcambin = ....
         GError* err = NULL;

         gint64 value = tcam_property_provider_get_tcam_integer(TCAM_PROPERTY_PROVIDER(tcambin), "name", &err);

         if (err)
         {
             // error handling
         }

   .. group-tab:: python

      .. code-block:: python
      
         tcambin = ....
         
         try:
             value = tcambin.get_tcam_integer("name")
         except GLib.Error as err:
             # error handling   

.. _tcam_property_provider_get_tcam_float:
                
tcam_property_provider_get_tcam_float
-------------------------------------
                
.. c:function:: gdouble tcam_property_provider_get_tcam_float (TcamPropertyProvider* self, const gchar* name, GError** err);
                
:param self: Pointer to the TcamPropertyProvider instance
:param name: String containing the name of the double that shall be queried
:param err: Pointer to a GError* variable that will filled if an error occurs. May be `NULL`.
:returns: double containing the currently property value
:retval: double
         
.. tabs::

   .. group-tab:: c

      .. code-block:: c

         GstElement* tcambin = ....
         GError* err = NULL;

         double value = tcam_property_provider_get_tcam_float(TCAM_PROPERTY_PROVIDER(tcambin), "name", &err);

         if (err)
         {
            // error handling
         }

   .. group-tab:: python

      .. code-block:: python
                  
         tcambin = ....

         try:
             value = tcambin.get_tcam_float("name")
         except GLib.Error as err:
             # error handling




.. _tcam_property_provider_get_tcam_enumeration:
                
tcam_property_provider_get_tcam_enumeration
-------------------------------------------
                
.. c:function:: const char* tcam_property_provider_get_tcam_enumeration (TcamPropertyProvider* self, const gchar* name, GError** err);

:param self: Pointer to the TcamPropertyProvider instance
:param name: String containing the name of the enumeration that shall be queried
:param err: Pointer to a GError* variable that will filled if an error occurs. May be `NULL`.
:returns: String containing the currently selected enum entry
:retval: const char*
                
.. tabs::

   .. group-tab:: c

      .. code-block:: c

         GstElement* tcambin = ....
         GError* err = NULL;

         const char* value = tcam_property_provider_get_tcam_enumeration(TCAM_PROPERTY_PROVIDER(tcambin), "name", &err);

         if (err)
         {
             // error handling
         }

   .. group-tab:: python

      .. code-block:: python
                  
         tcambin = ....

         try:
             value = tcambin.get_tcam_enumeration("name")
         except GLib.Error as err:
             # error handling

.. _tcampropertybase:
                
TcamPropertyBase
################

            
.. py:class:: TcamPropertyBase

   Base class for all properties. Can be cast into different derived classes.
   Check the property type via :c:func:`tcam_property_base_get_property_type` to ensure the correct cast will be used.

   Python users will have to do nothing.

   Retrieval of properties is done by calling :c:func:`tcam_property_provider_get_tcam_property`.






.. _tcam_property_base_get_name:
   
tcam_property_base_get_name
---------------------------



.. c:function:: const gchar* tcam_property_base_get_name(TcamPropertyBase* self);

:param self: Pointer to the property instance
:returns: Name of the property
:retval: const gchar*, string containing the name

The property owns the string. It will be freed once the property is destroyed.

.. tabs::

   .. group-tab:: c

      .. code-block:: c

         TcamPropertyBase* base_property = ....

         const char* name = tcam_property_base_get_name(base_property);

   .. group-tab:: python

      .. code-block:: python

         name = base_property.get_name()
         



.. _m_property_base_get_display_name:
   
tcam_property_base_get_display_name
-----------------------------------
   
.. c:function:: const gchar* tcam_property_base_get_display_name (TcamPropertyBase* self);

:param self: Pointer to the property instance
:returns: Name of the property
:retval: const gchar*, string containing the display name
                
| The property owns the string. It will be freed once the property is destroyed.
|
| The display name is a human readable name intended for GUIs and similar interfaces.

.. tabs::

   .. group-tab:: c

      .. code-block:: c

         TcamPropertyBase* base_property = ....

         const char* display_name = tcam_property_base_get_display_name(base_property);

   .. group-tab:: python

      .. code-block:: python

         display_name = base_property.get_display_name()
         

   

.. _tcam_property_base_get_description:
   
tcam_property_base_get_description
----------------------------------

.. c:function:: const gchar* tcam_property_base_get_description (TcamPropertyBase* self);

:param self: Pointer to the property instance
:returns: Name of the property
:retval: const gchar*, string containing the description
         
The property owns the string. It will be freed once the property is destroyed.


.. tabs::

   .. group-tab:: c

      .. code-block:: c

         TcamPropertyBase* base_property = ....

         const char* description = tcam_property_base_get_description(base_property);

   .. group-tab:: python

      .. code-block:: python

         description = base_property.get_description()
                               
   

.. _tcam_property_base_get_category:
   
tcam_property_base_get_category
-------------------------------

.. c:function:: const gchar* tcam_property_base_get_category (TcamPropertyBase* self);

   :param self: Pointer to the property instance
   :returns: Name of the property
   :retval: const gchar*, string containing the category

   The property owns the string. It will be freed once the property is destroyed.


.. tabs::

   .. group-tab:: c

      .. code-block:: c

         TcamPropertyBase* base_property = ....

         const char* category = tcam_property_base_get_category(base_property);

   .. group-tab:: python

      .. code-block:: python

         category = base_property.get_category()
         

.. _tcam_property_base_get_visibility:
   
tcam_property_base_get_visibility
---------------------------------
   
   
.. cpp:function:: TcamPropertyVisibility tcam_property_base_get_visibility (TcamPropertyBase* self);

                  
.. tabs::

   .. group-tab:: c

      .. code-block:: c

         TcamPropertyBase* base_property = ...

         TcamPropertyVisibility = tcam_property_base_get_visibility(base_property);

                               
   .. group-tab:: python

      .. code-block:: python

         visibility = base_property.get_visibility()
                  

.. _tcam_property_base_get_property_type:
                  
tcam_property_base_get_property_type
------------------------------------

The property owns the string. It will be freed once the property is destroyed.


.. tabs::

   .. group-tab:: c

      .. cpp:function:: TcamPropertyType tcam_property_base_get_property_type (TcamPropertyBase* self);

         :param self: TcamPropertyBase instance that shall be queried
         :returns: the actual type of the property
         :retval: a TcamPropertyType entry

                     
      .. code-block:: c
                                  
         if (tcam_property_base_is_locked(base_property))
         {
             // property is locked and cannot be changed
         }

   .. group-tab:: python

      .. py:method:: Tcam.PropertyType  get_property_type()
                        
         :param self: TcamPropertyBase instance that shall be queried
         :returns: the actual type of the property
         :retval: a TcamPropertyType entry

      .. code-block:: python
                  
         if base_property.is_locked():
             # property is locked and cannot be changed
         



.. _tcam_property_base_is_available:
                    
tcam_property_base_is_available
-------------------------------
.. c:function:: gboolean tcam_property_base_is_available (TcamPropertyBase* self, GError** err);


.. tabs::
      
   .. group-tab:: c

      .. code-block:: c
                                  
         if (tcam_property_base_is_locked(base_property, &err))
         {
             // property is locked and cannot be changed
         }
            
         if (err)
         {
             // error handling
         }

   .. group-tab:: python

      .. code-block:: python
                  
         try:
             if base_property.is_locked():
                 # property is locked and cannot be changed
             except GLib.Error as err:
                 # error handling
   




.. _tcam_property_base_is_locked:
                    
tcam_property_base_is_locked
----------------------------
                
.. c:function:: gboolean tcam_property_base_is_locked (TcamPropertyBase* self, GError** err);

:param self:
:param err: Pointer to GError pointer to be used in case of error. Can be `NULL`.
:returns: Bool describing of property is locked.
:retval: gboolean

.. tabs::

   .. group-tab:: c

      .. code-block:: c
                         
         if (tcam_property_base_is_locked(base_property, &err))
         {
             // property is locked and cannot be changed
         }

         if (err)
         {
             // error handling
         }

   .. group-tab:: python

      .. code-block:: python
                     
         try:
             if base_property.is_locked():
                 # property is locked and cannot be changed
         except GLib.Error as err:
             # error handling
      

.. _TcamPropertyBoolean:
                
TcamPropertyBoolean
###################
.. c:type:: TcamPropertyBoolean

Property representing a bool value.
An instance can be retrieved by casting a :ref:`TcamPropertyBase` pointer.
`TCAM_PROPERTY_BOOLEAN(TcamPropertyBase*)`

Upon cleanup `g_object_unref` has to be called on the property.


Inherits from :c:type:`TcamPropertyBase`.
Can be obtained by casting a :c:type:`TcamPropertyBase` with `TCAM_PROPERTY_BOOLEAN(TcamPropertyBase*)`.

.. _tcam_property_boolean_get_value:

tcam_property_boolean_get_value
-------------------------------

.. c:function:: gboolean tcam_property_boolean_get_value (TcamPropertyBoolean* self, GError** err);

:param self:
:param err: Pointer to GError pointer to be used in case of error. Can be `NULL`.
:returns: Bool describing of property value.
:retval: gboolean


.. tabs::

   .. group-tab:: c

      .. code-block:: c

         TcamPropertyBoolean* bool_property = TCAM_PROPERTY_BOOLEAN(base_property);
         GError* err = NULL;

         bool current_value = tcam_property_boolean_get_value(bool_property, &err);

         if (err)
         {
             // error handling
         }
                  
   .. group-tab:: python

      .. code-block:: python

         try:
             current_value = base_property.get_value()
         except GLib.Error as err:
             # error handling
         
.. _tcam_property_boolean_set_value:
                
tcam_property_boolean_set_value
-------------------------------

.. c:function:: void tcam_property_boolean_set_value (TcamPropertyBoolean* self, gboolean value, GError** err);

:param self:
:param err: Pointer to GError pointer to be used in case of error. Can be `NULL`.
:param value: value that shall be set.


.. tabs::

   .. group-tab:: c

      .. code-block:: c

         TcamPropertyBoolean* bool_property = TCAM_PROPERTY_BOOLEAN(base_property);
         GError* err = NULL;

         
         bool new_value = true;
         tcam_property_boolean_set_value(bool_property, new_value, &err);

         if (err)
         {
             // error handling
         }
                               
   .. group-tab:: python

      .. code-block:: python

         try:
             new_value = True
             base_property.set_value(new_value)
         except GLib.Error as err:
             # error handling


              
              
.. _tcam_property_boolean_get_default:
                
tcam_property_boolean_get_default
---------------------------------                
                
.. c:function:: gboolean tcam_property_boolean_get_default (TcamPropertyBoolean* self, GError** err);

:param self:
:param err: Pointer to GError pointer to be used in case of error. Can be `NULL`.
:returns: Bool describing the property default.
:retval: gboolean



.. tabs::

   .. group-tab:: c

      .. code-block:: c

         TcamPropertyBoolean* bool_property = TCAM_PROPERTY_BOOLEAN(base_property);
         GError* err = NULL;

         bool default_value = tcam_property_boolean_get_default(bool_property, &err);

         if (err)
         {
             // error handling
         }
                               
   .. group-tab:: python

      .. code-block:: python

         try:
             current_value = base_property.get_default()
         except GLib.Error as err:
             # error handling
         
.. _TcamPropertyInteger:
                
TcamPropertyInteger
###################


Property representing an integer value.
An instance can be retrieved by casting a :ref:`TcamPropertyBase` pointer.
`TCAM_PROPERTY_INTEGER(TcamPropertyBase*)`

Upon cleanup `g_object_unref` has to be called on the property.


.. c:type:: TcamPropertyInteger





.. _tcam_property_integer_get_value:
            
tcam_property_integer_get_value
-------------------------------
            
.. cpp:function:: gint64 tcam_property_integer_get_value (TcamPropertyInteger* self, GError** err);

:param self: property instance
:param err: Pointer to GError pointer to be used in case of error. Can be `NULL`.
:returns: int64 describing the property value.
:retval: gint64


.. tabs::

   .. group-tab:: c

      .. code-block:: c

         TcamPropertyInteger* int_property = TCAM_PROPERTY_INTEGER(base_property);
         GError* err = NULL;

         int64 current_value = tcam_property_integer_get_value(int_property, &err);

         if (err)
         {
             // error handling
         }
                  
   .. group-tab:: python

      .. code-block:: python

         try:
             current_value = base_property.get_value()
         except GLib.Error as err:
             # error handling


         
.. _tcam_property_integer_set_value:
                  
tcam_property_integer_set_value
-------------------------------

.. cpp:function:: void tcam_property_integer_set_value (TcamPropertyInteger* self, gint64 value, GError** err);

:param self: property instance
:param value: int64 value that shall be set.
:param err: Pointer to GError pointer to be used in case of error. Can be `NULL`.


.. tabs::

   .. group-tab:: c

      .. code-block:: c

         TcamPropertyInteger* int_property = TCAM_PROPERTY_INTEGER(base_property);
         GError* err = NULL;

         int64 new_value = 500;
         tcam_property_integer_set_value(int_property, new_value, &err);

         if (err)
         {
             // error handling
         }
                               
   .. group-tab:: python

      .. code-block:: python

         try:
             new_value = 500
             base_property.set_value(new_value)
         except GLib.Error as err:
             # error handling

            
.. _tcam_property_integer_get_range:
                  
tcam_property_integer_get_range
-------------------------------

.. cpp:function:: void tcam_property_integer_get_range (TcamPropertyInteger* self, gint64* min_value, gint64* max_value, gint64* step_value, GError** err);

:param self: property instance
:param min_value: out value. pointer to a int64 that will be filled with the minimal value the property can have. May be `NULL`.
:param max_value: out value. pointer to a int64 that will be filled with the maximum value the property can have. May be `NULL`.
:param step_value: out value. pointer to a int64 that will be filled with the step size between values. May be `NULL`.
:param err: Pointer to GError pointer to be used in case of error. Can be `NULL`.


.. tabs::

   .. group-tab:: c

      .. code-block:: c

         TcamPropertyInteger* int_property = TCAM_PROPERTY_INTEGER(base_property);
         GError* err = NULL;
         int64 min_value;
         int64 max_value;
         int64 step_value;
         tcam_property_integer_get_representation(int_property,
                                                  &min_value,
                                                  &max_value,
                                                  &step_value,
                                                  &err);

         if (err)
         {
             // error handling
         }
                  
   .. group-tab:: python

      .. code-block:: python

         try:
             min_value, max_value, step_value = base_property.get_range()
         except GLib.Error as err:
             # error handling

            

.. _tcam_property_integer_get_default:
                  
tcam_property_integer_get_default
---------------------------------

.. cpp:function:: gint64 tcam_property_integer_get_default (TcamPropertyInteger* self, GError** err);

:param self: property instance
:param err: Pointer to GError pointer to be used in case of error. Can be `NULL`.
:returns: int64 describing the property default value.
:retval: gint64


.. tabs::

   .. group-tab:: c

      .. code-block:: c

         TcamPropertyInteger* int_property = TCAM_PROPERTY_INTEGER(base_property);
         GError* err = NULL;
         int64 default_value = tcam_property_integer_get_default(int_property, &err);

         if (err)
         {
             // error handling
         }
                               
   .. group-tab:: python

      .. code-block:: python

         try:
             default_value = base_property.get_default()
         except GLib.Error as err:
             # error handling


         
.. _tcam_property_integer_get_unit:
                  
tcam_property_integer_get_unit
------------------------------

.. cpp:function:: const gchar* tcam_property_integer_get_unit (TcamPropertyInteger* self);

:param self: property instance
:param err: Pointer to GError pointer to be used in case of error. Can be `NULL`.
:returns: string describing the property unit. Can be an empty string.
:retval: const char*


.. tabs::

   .. group-tab:: c

      .. code-block:: c

         TcamPropertyInteger* int_property = TCAM_PROPERTY_INTEGER(base_property);
         GError* err = NULL;
         const char* unit = tcam_property_integer_get_unit(int_property, &err);

         if (!unit)
         {
             if (err)
             {
                 // error handling
             }
         }
                  
   .. group-tab:: python

      .. code-block:: python

         try:
             unit = base_property.get_unit()
         except GLib.Error as err:
             # error handling
         

.. _tcam_property_integer_get_representation:
                  
tcam_property_integer_get_representation
----------------------------------------

.. cpp:function:: TcamPropertyIntRepresentation tcam_property_integer_get_representation (TcamPropertyInteger* self);


:param self: property instance
:param err: Pointer to GError pointer to be used in case of error. Can be `NULL`.
:returns: TcamPropertyIntRepresentation describing the recommended way of displaying the property.
:retval: :ref:`TcamPropertyIntRepresentation`


.. tabs::

   .. group-tab:: c

      .. code-block:: c

         TcamPropertyInteger* int_property = TCAM_PROPERTY_INTEGER(base_property);
         GError* err = NULL;
         TcamPropertyIntRepresentation representation = tcam_property_integer_get_representation(int_property, &err);

         if (err)
         {
             // error handling
         }
                               
   .. group-tab:: python

      .. code-block:: python

         try:
             representation = base_property.get_representation()
         except GLib.Error as err:
             # error handling
         

.. _TcamPropertyFloat:
                  
TcamPropertyFloat
#################

Property representing a floating point value.
An instance can be retrieved by casting a :ref:`TcamPropertyBase` pointer.
`TCAM_PROPERTY_FLOAT(TcamPropertyBase*)`

Upon cleanup `g_object_unref` has to be called on the property.


.. c:type:: TcamPropertyFloat


.. _tcam_property_float_get_value:
            
tcam_property_float_get_value
-----------------------------
            
.. c:function:: gdouble tcam_property_float_get_value (TcamPropertyFloat* self, GError** err);

:param self: property instance
:param err: Pointer to GError pointer to be used in case of error. Can be `NULL`.
:returns: double describing the property value.
:retval: double

.. tabs::

   .. group-tab:: c

      .. code-block:: c

         TcamPropertyFloat* float_property = TCAM_PROPERTY_FLOAT(base_property);
         GError* err = NULL;

         double current_value = tcam_property_float_get_value(float_property, &err);

         if (err)
         {
             // error handling
         }
                               
   .. group-tab:: python

      .. code-block:: python

         try:
             current_value = base_property.get_value()
         except GLib.Error as err:
             # error handling

         
.. _tcam_property_float_set_value:
                
tcam_property_float_set_value
-----------------------------
                
.. c:function:: void tcam_property_float_set_value (TcamPropertyFloat* self, gdouble value, GError** err);

:param self: property instance
:param value: double value that shall be set.
:param err: Pointer to GError pointer to be used in case of error. Can be `NULL`.

.. tabs::

   .. group-tab:: c

      .. code-block:: c

         TcamPropertyFloat* float_property = TCAM_PROPERTY_FLOAT(base_property);
         GError* err = NULL;

         double new_value = 30000.0;
         tcam_property_float_set_value(float_property, new_value, &err);

         if (err)
         {
             // error handling
         }
                      
   .. group-tab:: python

      .. code-block:: python

         try:
             new_value = 30000.0
             base_property.set_value(new_value)
         except GLib.Error as err:
             # error handling


             
.. _tcam_property_float_get_range:
                
tcam_property_float_get_range
-----------------------------
                
.. c:function:: void tcam_property_float_get_range (TcamPropertyFloat* self, gdouble* min_value, gdouble* max_value, gdouble* step_value, GError** err);

:param self: property instance
:param min_value: out value. pointer to a double that will be filled with the minimal value the property can have. May be `NULL`.
:param max_value: out value. pointer to a double that will be filled with the maximum value the property can have. May be `NULL`.
:param step_value: out value. pointer to a double that will be filled with the step size between values. May be `NULL`.
:param err: Pointer to GError pointer to be used in case of error. Can be `NULL`.


.. tabs::

   .. group-tab:: c

      .. code-block:: c

         TcamPropertyFloat* float_property = TCAM_PROPERTY_FLOAT(base_property);
         GError* err = NULL;
         double min_value;
         double max_value;
         double step_value;
         tcam_property_float_get_representation(float_property,
                                                &min_value,
                                                &max_value,
                                                &step_value,
                                                &err);

         if (err)
         {
             // error handling
         }
                               
   .. group-tab:: python

      .. code-block:: python

         try:
             min_value, max_value, step_value = base_property.get_range()
         except GLib.Error as err:
             # error handling


             
.. _tcam_property_float_get_default:
                
tcam_property_float_get_default
-------------------------------

.. c:function:: gdouble tcam_property_float_get_default (TcamPropertyFloat* self, GError** err);


:param self: property instance
:param err: Pointer to GError pointer to be used in case of error. Can be `NULL`.
:returns: double describing the property default value.
:retval: double

.. tabs::

   .. group-tab:: c

      .. code-block:: c

         TcamPropertyFloat* float_property = TCAM_PROPERTY_FLOAT(base_property);
         GError* err = NULL;
         double default_value = tcam_property_float_get_default(float_property, &err);

         if (err)
         {
             // error handling
         }
                      
   .. group-tab:: python

      .. code-block:: python

         try:
             default_value = base_property.get_default()
         except GLib.Error as err:
             # error handling

.. _tcam_property_float_get_unit:
                
tcam_property_float_get_unit
----------------------------
                
.. c:function:: const gchar* tcam_property_float_get_unit (TcamPropertyFloat* self);


:param self: property instance
:param err: Pointer to GError pointer to be used in case of error. Can be `NULL`.
:returns: string describing the property unit. Can be an empty string.
:retval: const char*

.. tabs::

   .. group-tab:: c

      .. code-block:: c

         TcamPropertyFloat* float_property = TCAM_PROPERTY_FLOAT(base_property);
         GError* err = NULL;
         const char* unit = tcam_property_float_get_unit(float_property, &err);

         if (!unit)
         {
             if (err)
             {
                 // error handling
             }
         }
                               
   .. group-tab:: python

      .. code-block:: python

         try:
             unit = base_property.get_unit()
         except GLib.Error as err:
             # error handling

.. _tcam_property_float_get_representation:
                
tcam_property_float_get_representation
--------------------------------------
                
.. c:function:: TcamPropertyFloatRepresentation tcam_property_float_get_representation (TcamPropertyFloat* self);



:param self: property instance
:param err: Pointer to GError pointer to be used in case of error. Can be `NULL`.
:returns: TcamPropertyFloatRepresentation describing the recommended way of displaying the property.
:retval: :ref:`TcamPropertyFloatRepresentation`
                                                            

.. tabs::

   .. group-tab:: c

      .. code-block:: c

         TcamPropertyFloat* float_property = TCAM_PROPERTY_FLOAT(base_property);
         GError* err = NULL;
         TcamPropertyFloatRepresentation representation = tcam_property_float_get_representation(float_property, &err);

         if (err)
         {
             // error handling
         }
                      
   .. group-tab:: python

      .. code-block:: python

         try:
             representation = base_property.get_representation()
         except GLib.Error as err:
             # error handling

.. _TcamPropertyEnumeration:
                
TcamPropertyEnumeration
#######################

Property representing an enumeration/menu value.
An instance can be retrieved by casting a :ref:`TcamPropertyBase` pointer.
`TCAM_PROPERTY_ENUMERATION(TcamPropertyBase*)`

Upon cleanup `g_object_unref` has to be called on the property.


.. c:type:: TcamPropertyEnumeration



.. _tcam_property_enumeration_get_value:
            
tcam_property_enumeration_get_value
-----------------------------------

.. c:function:: const gchar* tcam_property_enumeration_get_value (TcamPropertyEnumeration* self, GError** err);

   The caller does **NOT** take ownership of the returned value.

:param self: property instance
:param err: Pointer to GError pointer to be used in case of error. Can be `NULL`.
:returns: string describing the properties current value.  `NULL` on error.
:retval: const char*

.. tabs::

   .. group-tab:: c

      .. code-block:: c

         TcamPropertyEnumeration* enumeration = TCAM_PROPERTY_ENUMERATION(base_property);
         GError* err = NULL;
         const char* current_value = tcam_property_enumeration_get_value(enumeration, &err);

         if (!current_value)
         {
             if (err)
             {
                 // error handling
             }
         }
                      
   .. group-tab:: python

      .. code-block:: python

         try:
             current_value = base_property.get_value()
         except GLib.Error as err:
             # error handling
         
.. _tcam_property_enumeration_set_value:
   
tcam_property_enumeration_set_value
-----------------------------------

.. c:function:: void tcam_property_enumeration_set_value (TcamPropertyEnumeration* self, const gchar* value, GError** err);


:param self: property instance
:param value: entry string that shall be set.
:param err: Pointer to GError pointer to be used in case of error. Can be `NULL`.

.. tabs::

   .. group-tab:: c

      .. code-block:: c

         TcamPropertyEnumeration* enumeration = TCAM_PROPERTY_ENUMERATION(base_property);
         GError* err = NULL;
         const char* new_value = "entry";
         tcam_property_enumeration_set_value(enumeration, new_value, &err);

         if (err)
         {
             // error handling
         }
                               
   .. group-tab:: python

      .. code-block:: python

         try:
             new_value = "entry"
             base_property.set_value(new_value)
         except GLib.Error as err:
             # error handling             

.. _tcam_property_enumeration_get_enum_entries:
                
tcam_property_enumeration_get_enum_entries
------------------------------------------
                
.. c:function:: GSList* tcam_property_enumeration_get_enum_entries (TcamPropertyEnumeration* self, GError** err);

The caller takes ownership of the returned list and its values.
Call `g_slist_free_full(enum_entries, g_free)` when not done.

:param self: property instance
:param err: Pointer to GError pointer to be used in case of error. Can be `NULL`.
:returns: A GSList containing string values describing all possible property values. `NULL` on error.
:retval: GSList*

.. tabs::

   .. group-tab:: c

      .. code-block:: c

         TcamPropertyEnumeration* enumeration = TCAM_PROPERTY_ENUMERATION(base_property);
         GError* err = NULL;
         GSList* entry_list = tcam_property_enumeration_get_enum_entries(enumeration, &err);

         if (!entry_list)
         {
             if (err)
             {
                 // error handling
             }
         }

         // when done call
         g_slist_free_full(entry_list, g_free);

   .. group-tab:: python

      .. code-block:: python   

         try:
             entry_list = base_property.get_enum_entries()
         except GLib.Error as err:
             # error handling


.. _tcam_property_enumeration_get_default:
   
tcam_property_enumeration_get_default
-------------------------------------

.. c:function:: const gchar* tcam_property_enumeration_get_default (TcamPropertyEnumeration* self, GError** err);

:param self: property instance
:param err: Pointer to GError pointer to be used in case of error. Can be `NULL`.
:returns: string describing the property default value. `NULL` on error.
:retval: const char* 

.. tabs::

   .. group-tab:: c

      .. code-block:: c

         TcamPropertyEnumeration* enumeration = TCAM_PROPERTY_ENUMERATION(base_property);
         GError* err = NULL;
         const char* default_value = tcam_property_enumeration_get_default(enumeration, &err);

         if (!default_value)
         {
             if (err)
             {
                 // error handling
             }
         }
                      
   .. group-tab:: python

      .. code-block:: python

         try:
             default_value = base_property.get_default()
         except GLib.Error as err:
             # error handling


.. _TcamPropertyCommand:
                
TcamPropertyCommand
###################

Property representing a command/button value.
An instance can be retrieved by casting a :ref:`TcamPropertyBase` pointer.
`TCAM_PROPERTY_COMMAND(TcamPropertyBase*)`

Upon cleanup `g_object_unref` has to be called on the property.

.. _tcam_property_command_set_command:

tcam_property_command_set_command
---------------------------------

Execute the command.

.. c:function:: void tcam_property_command_set_command (TcamPropertyCommand* self, GError** err);

:param self: property instance
:param err: Pointer to GError pointer to be used in case of error. Can be `NULL`.

.. tabs::

   .. group-tab:: c

      .. code-block:: c

         TcamPropertyCommand* command = TCAM_PROPERTY_COMMAND(base_property);
         GError* err = NULL;
         tcam_property_command_set_command(command, &err);

         if (err)
         {
             // error handling
         }

   .. group-tab:: python
         
      .. code-block:: python

         try:
             base_property.set_command()
         except GLib.Error as err:
             # error handling
