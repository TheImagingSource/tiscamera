#############
tcam-property
#############


This page describes the official gobject-introspection API.

.. note::
   The current version of this API is 1.0

.. contents:: Table of Contents
              :depth: 5

.. _tcampropertyprovider:
        
TcamPropertyProvider
####################

This object is typically a casted gstreamer element like :ref:`tcambin`, :ref:`tcamsrc` or :ref:`tcamdutils`.

Properties require the GStreamer element to be at least in the state `GST_STATE_READY`.

| Properties will become invalid once the GStreamer element enters the state `GST_STATE_NULL`.
| In such a case :cpp:enumerator:`TCAM_ERROR_DEVICE_NOT_OPENED` will be returned.

.. _tcam_property_provider_get_tcam_property_names:
   
tcam_property_provider_get_tcam_property_names
----------------------------------------------

Retrieve a list of all currently available properties. GstElement must be `GST_STATE_READY` or higher.

.. tabs::

   .. group-tab:: c

      .. c:function:: GSList* tcam_property_provider_get_tcam_property_names (TcamPropertyProvider* self, GError** err)

         :param self: :ref:`TcamPropertyProvider` instance that shall be queried
         :param err: A :c:type:`GError` pointer, may be NULL
         :returns: (element-type utf8) (transfer full): A list of property names supported by this object
         :retval GSList*: A single linked list containing strings with property names
         :retval NULL: If an error occurs, NULL will be returned

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

      .. py:method:: Tcam.PropertyProvider.get_tcam_property_names()
         
         :exception: May raise an `GLib.Error` when communication with a device fails
         :returns: A list of property names supported by this object
         :rtype: List of strings

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

Retrieve a specific property instance.

Property has to be unreferenced after usage.

Instances will return a :ref:`GError` containing ref:`TCAM_ERROR_NO_DEVICE_OPEN` when the providing device is closed or lost.

.. tabs::

   .. group-tab:: c

       .. c:function:: GSList* tcam_property_provider_get_tcam_property (TcamPropertyProvider* self, const char* name, GError** err)

         :param self: :ref:`TcamPropertyProvider` instance that shall be queried
         :param name: A string pointer, naming the property which instance shall be returned.
         :param err: A :c:type:`GError` pointer, may be NULL
         :return: (transfer full): A :ref:`TcamPropertyBase` object. 
         :retval NULL: If an error occurs, NULL will be returned. Check err
         :retval: a valid :ref:`TcamPropertyProvider` instance

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

       .. py:method:: Tcam.PropertyProvider.get_tcam_property( name )
         
         :param name: Name of the property to return
         :exception: May raise an `GLib.Error` when communication with a device fails
         :returns: A instance of a :ref:`TcamPropertyBase` derived object

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

.. tabs::

   .. group-tab:: c

      .. c:function:: void tcam_property_provider_set_tcam_boolean (TcamPropertyProvider* self, const char* name, gboolean value, GError** err)

         :param self: The :ref:`TcamPropertyProvider` instance
         :param name: A string, naming the property that shall be set.
         :param value: The value to set.
         :param err: A :c:type:`GError` pointer, may be NULL

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
      
       .. py:method:: Tcam.PropertyProvider.set_tcam_boolean(name, value)
         
         :param name: Name of the property
         :param value: New value to set
         :exception: May raise an `GLib.Error` when setting the property fails

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

.. tabs::

   .. group-tab:: c

      .. c:function:: void tcam_property_provider_set_tcam_integer (TcamPropertyProvider* self, const char* name, gint64 value, GError** err)

         :param self: The :ref:`TcamPropertyProvider` instance
         :param name: A string, naming the property that shall be set.
         :param value: The value to set.
         :param err: A :c:type:`GError` pointer, may be NULL

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
   
      .. py:method:: Tcam.PropertyProvider.set_tcam_integer(name, value)
         
         :param name: Name of the property
         :param value: New value to set
         :exception: May raise an `GLib.Error` when setting the property fails

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
               
.. tabs::

   .. group-tab:: c

      .. c:function:: void tcam_property_provider_set_tcam_float (TcamPropertyProvider* self, const char* name, gdouble value, GError** err)

         :param self: The :ref:`TcamPropertyProvider` instance
         :param name: A string, naming the property that shall be set.
         :param value: The value to set.
         :param err: A :c:type:`GError` pointer, may be NULL

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

       .. py:method:: Tcam.PropertyProvider.set_tcam_float(name, value)
         
         :param name: Name of the property
         :param value: New value to set
         :exception: May raise an `GLib.Error` when setting the property fails

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

.. tabs::

   .. group-tab:: c

      .. c:function:: void tcam_property_provider_set_tcam_enumeration (TcamPropertyProvider* self, const char* name, const char* value, GError** err)

         :param self: The :ref:`TcamPropertyProvider` instance
         :param name: A string, naming the property that shall be set.
         :param value: The string of the enumeration entry to set.
         :param err: A :c:type:`GError` pointer, may be NULL

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
   
       .. py:method:: Tcam.PropertyProvider.set_tcam_enumeration(name, value)
         
         :param name: Name of the property
         :param value: New value to set
         :exception: May raise an `GLib.Error` when setting the property fails

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

Convenience function to execute a command.

For complex applications it is recommended to use a :ref:`TcamPropertyCommand` instance instead.

.. tabs::

   .. group-tab:: c

      .. c:function:: void tcam_property_provider_set_tcam_command (TcamPropertyProvider* self, const char* name, GError** err)

         :param self: The :ref:`TcamPropertyProvider` instance
         :param name: A string, naming the property that shall be set.
         :param err: A :c:type:`GError` pointer, may be NULL

      .. code-block:: c

         GstElement* tcambin = ....
         GError* err = NULL;

         tcam_property_provider_set_tcam_command(TCAM_PROPERTY_PROVIDER(tcambin), "name", &err);

         if (err)
         {
             // error handling
         }

   .. group-tab:: python
      
       .. py:method:: Tcam.PropertyProvider.set_tcam_command(name)
         
         :param name: Name of the property
         :exception: May raise an `GLib.Error` when setting the property fails

      .. code-block:: python
                  
         tcambin = ....

         try:
             tcambin.set_tcam_command("name")
         except GLib.Error as err:
             # error handling


.. _tcam_property_provider_get_tcam_boolean:
                
tcam_property_provider_get_tcam_boolean
---------------------------------------
                
Convenience function to retrieve the value of a boolean property.

For complex applications it is recommended to use a :ref:`TcamPropertyBoolean` instance instead.

.. tabs::

   .. group-tab:: c
       
       .. c:function:: gboolean tcam_property_provider_get_tcam_boolean (TcamPropertyProvider* self, const char* name, GError** err)

         :param self: The :ref:`TcamPropertyProvider` instance
         :param name: A string, naming the property that shall be retrieved.
         :param err: A :c:type:`GError` pointer, may be NULL
         :returns: The value of the property or on error an unspecified value.

      .. code-block:: c

         GstElement* tcambin = ....
         GError* err = NULL;

         gboolean value = tcam_property_provider_get_tcam_boolean(TCAM_PROPERTY_PROVIDER(tcambin), "name", &err);

         if (err)
         {
            // error handling
         }

   .. group-tab:: python
      
       .. py:method:: Tcam.PropertyProvider.get_tcam_boolean(name)
         
         :param name: Name of the property
         :exception: May raise an `GLib.Error` when setting the property fails
         :returns: The value of the property
   
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

Convenience function to retrieve the value of an integer property.

For complex applications it is recommended to use a :ref:`TcamPropertyInteger` instance instead.
                
.. tabs::

   .. group-tab:: c
       
       .. c:function:: gint64 tcam_property_provider_get_tcam_integer (TcamPropertyProvider* self, const char* name, GError** err)

         :param self: The :ref:`TcamPropertyProvider` instance
         :param name: A string, naming the property that shall be retrieved.
         :param err: A :c:type:`GError` pointer, may be NULL
         :returns: The value of the property or on error an unspecified value.

      .. code-block:: c

         GstElement* tcambin = ....
         GError* err = NULL;

         gint64 value = tcam_property_provider_get_tcam_integer(TCAM_PROPERTY_PROVIDER(tcambin), "name", &err);

         if (err)
         {
             // error handling
         }

   .. group-tab:: python

       .. py:method:: Tcam.PropertyProvider.get_tcam_integer(name)
         
         :param name: Name of the property
         :exception: May raise an `GLib.Error` when setting the property fails
         :returns: The value of the property

      .. code-block:: python
      
         tcambin = ....
         
         try:
             value = tcambin.get_tcam_integer("name")
         except GLib.Error as err:
             # error handling   

.. _tcam_property_provider_get_tcam_float:
                
tcam_property_provider_get_tcam_float
-------------------------------------
                
Convenience function to retrieve the value of a float property.

For complex applications it is recommended to use a :ref:`TcamPropertyFloat` instance instead.
         
.. tabs::

   .. group-tab:: c
       
       .. c:function:: gdouble tcam_property_provider_get_tcam_float (TcamPropertyProvider* self, const char* name, GError** err)

         :param self: The :ref:`TcamPropertyProvider` instance
         :param name: A string, naming the property that shall be retrieved.
         :param err: A :c:type:`GError` pointer, may be NULL
         :returns: The value of the property or on error an unspecified value.

      .. code-block:: c

         GstElement* tcambin = ....
         GError* err = NULL;

         double value = tcam_property_provider_get_tcam_float(TCAM_PROPERTY_PROVIDER(tcambin), "name", &err);

         if (err)
         {
            // error handling
         }

   .. group-tab:: python

      .. py:method:: Tcam.PropertyProvider.get_tcam_float(name)
         
         :param name: Name of the property
         :exception: May raise an `GLib.Error` when setting the property fails
         :returns: The value of the property

      .. code-block:: python
                  
         tcambin = ....

         try:
             value = tcambin.get_tcam_float("name")
         except GLib.Error as err:
             # error handling




.. _tcam_property_provider_get_tcam_enumeration:
                
tcam_property_provider_get_tcam_enumeration
-------------------------------------------
                
Convenience function to retrieve the value of an enumeration property.

For complex applications it is recommended to use a :ref:`TcamPropertyEnumeration` instance instead.
                
.. tabs::

   .. group-tab:: c
       
       .. c:function:: const char* tcam_property_provider_get_tcam_enumeration (TcamPropertyProvider* self, const char* name, GError** err)

         :param self: The :ref:`TcamPropertyProvider` instance
         :param name: A string, naming the property that shall be retrieved.
         :param err: A :c:type:`GError` pointer, may be NULL
         :returns: (transfer none): The current enumeration entry string of the property or on error an unspecified value.

      .. code-block:: c

         GstElement* tcambin = ....
         GError* err = NULL;

         const char* value = tcam_property_provider_get_tcam_enumeration(TCAM_PROPERTY_PROVIDER(tcambin), "name", &err);

         if (err)
         {
             // error handling
         }

   .. group-tab:: python
   
      .. py:method:: Tcam.PropertyProvider.get_tcam_enumeration(name)
         
         :param name: Name of the property
         :exception: May raise an `GLib.Error` when setting the property fails
         :returns: The value of the property

      .. code-block:: python
                  
         tcambin = ....

         try:
             value = tcambin.get_tcam_enumeration("name")
         except GLib.Error as err:
             # error handling

.. _tcampropertybase:
                
TcamPropertyBase
################

Base class for all properties. Can be cast into different derived classes.

Check the property type via :ref:`tcam_property_base_get_property_type` to ensure the correct cast will be used.

Python users will have to do nothing.

Retrieval of properties is done by calling :ref:`tcam_property_provider_get_tcam_property`.

.. _tcam_property_base_get_name:
   
tcam_property_base_get_name
---------------------------

Fetches the name of this property.

.. tabs::

   .. group-tab:: c
       
       .. c:function:: const char* tcam_property_base_get_name (TcamPropertyBase* self)

         :param self: The :ref:`TcamPropertyBase` instance
         :returns: (transfer none): The name of the property. This is valid and will not change until this property is released.
         :retval NULL: This is only NULL if the passed in instance is not a :ref:`TcamPropertyBase`.

      .. code-block:: c

         TcamPropertyBase* base_property = ....

         const char* name = tcam_property_base_get_name(base_property);

   .. group-tab:: python
      
      .. py:method:: Tcam.PropertyBase.get_name()
         
         :returns: The name of this property

      .. code-block:: python

         name = base_property.get_name()
         

.. _tcam_property_base_get_display_name:
   
tcam_property_base_get_display_name
-----------------------------------
   
The display name is a human readable name intended for GUIs and similar interfaces.

.. tabs::

   .. group-tab:: c
       
       .. c:function:: const gchar* tcam_property_base_get_display_name (TcamPropertyBase* self)

         :param self: The :ref:`TcamPropertyBase` instance
         :returns: (transfer none): The display name of the property. This is valid and will not change until this property is released.
         :retval NULL: Maybe NULL if no display name is available for this property.

      .. code-block:: c

         TcamPropertyBase* base_property = ....

         const char* display_name = tcam_property_base_get_display_name(base_property);

   .. group-tab:: python
         
      .. py:method:: Tcam.PropertyBase.get_display_name()
         
         :returns: The display name of the property. 

      .. code-block:: python

         display_name = base_property.get_display_name()
   

.. _tcam_property_base_get_description:
   
tcam_property_base_get_description
----------------------------------

Description of the property.

.. tabs::

   .. group-tab:: c
       
       .. c:function:: const gchar* tcam_property_base_get_description (TcamPropertyBase* self)

         :param self: The :ref:`TcamPropertyBase` instance
         :returns: (transfer none): The description of the property. This is valid and will not change until this property is released.
         :retval NULL: Maybe NULL if no description is available for this property.

      .. code-block:: c

         TcamPropertyBase* base_property = ....

         const char* description = tcam_property_base_get_description(base_property);

   .. group-tab:: python

      .. py:method:: Tcam.PropertyBase.get_description()
         
         :returns: The description of the property. 

      .. code-block:: python

         description = base_property.get_description()
                               
   

.. _tcam_property_base_get_category:
   
tcam_property_base_get_category
-------------------------------

Category string for simple property organization.

.. tabs::

   .. group-tab:: c
   
      .. c:function:: const gchar* tcam_property_base_get_category (TcamPropertyBase* self)

         :param self: The :ref:`TcamPropertyBase` instance
         :returns: (transfer none): The name of the category this property is associated with. This is valid and will not change until this property is released.
         :retval NULL: Maybe NULL if no category is available for this property.

      .. code-block:: c

         TcamPropertyBase* base_property = ....

         const char* category = tcam_property_base_get_category(base_property);

   .. group-tab:: python
   
      .. py:method:: Tcam.PropertyBase.get_category()
         
         :returns: The name of the category this property is associated with.

      .. code-block:: python

         category = base_property.get_category()

.. _tcam_property_base_get_access:
   
tcam_property_base_get_access
---------------------------------
   
Specifies the :ref:`TcamPropertyAccess` for the property.
                  
.. tabs::

   .. group-tab:: c

      .. c:function:: TcamPropertyAccess tcam_property_base_get_access (TcamPropertyBase* self)

         :param self: The :ref:`TcamPropertyBase` instance
         :returns: The :ref:`TcamPropertyAccess` of the property

      .. code-block:: c

         TcamPropertyBase* base_property = ...

         TcamPropertyAccess access = tcam_property_base_get_access(base_property);

                               
   .. group-tab:: python

      .. py:method:: Tcam.TcamPropertyAccess  Tcam.PropertyBase.get_access()
                        
         :returns: the :ref:`TcamPropertyAccess` of the property

      .. code-block:: python

         access_level = base_property.get_access()


.. _tcam_property_base_get_visibility:
   
tcam_property_base_get_visibility
---------------------------------
   
Specifies a :ref:`TcamPropertyVisibility` for the property, providing a recommended visibility level for applications.
                  
.. tabs::

   .. group-tab:: c

      .. c:function:: TcamPropertyVisibility tcam_property_base_get_visibility (TcamPropertyBase* self)

         :param self: The :ref:`TcamPropertyBase` instance
         :returns: The :ref:`TcamPropertyVisibility` of the property

      .. code-block:: c

         TcamPropertyBase* base_property = ...

         TcamPropertyVisibility vis = tcam_property_base_get_visibility(base_property);

                               
   .. group-tab:: python

      .. py:method:: Tcam.PropertyVisibility  Tcam.PropertyBase.get_visibility()
                        
         :returns: the :ref:`TcamPropertyVisibility` of the property

      .. code-block:: python

         visibility = base_property.get_visibility()
                  

.. _tcam_property_base_get_property_type:
                  
tcam_property_base_get_property_type
------------------------------------

A :ref:`TcamPropertyType` describing the specific property type of the TcamPropertyBase instance.

Cast the TcamPropertyBase instance into a derived type to access more functions.

.. tabs::

   .. group-tab:: c

      .. c:function:: TcamPropertyType tcam_property_base_get_property_type (TcamPropertyBase* self)

         :param self: The :ref:`TcamPropertyBase` instance
         :returns: The :ref:`TcamPropertyType` of the property
                     
      .. code-block:: c

         TcamPropertyType type = tcam_property_base_get_property_type(base_property);

   .. group-tab:: python

      .. py:method:: Tcam.PropertyType  Tcam.PropertyBase.get_property_type()
                        
         :returns: the :ref:`TcamPropertyType` of the property

      .. code-block:: python
         
         property_type = base_property.get_property_type()


.. _tcam_property_base_is_available:
                    
tcam_property_base_is_available
-------------------------------

| Check if property is currently available.
| If the property is not available it means that a stream setting is preventing usage.
| A typical example would be BalanceWhiteAuto being not available while streaming `video/x-raw,format=GRAY8`.


.. tabs::
      
   .. group-tab:: c
      
      .. c:function:: gboolean tcam_property_base_is_available (TcamPropertyBase* self, GError** err)

         :param self: The :ref:`TcamPropertyBase` instance
         :param err: A :c:type:`GError` pointer, may be NULL
         :returns: Returns :c:type:`TRUE` if the property is currently available, otherwise false.

      .. code-block:: c
                                  
         if (tcam_property_base_is_available(base_property, &err))
         {
             // property is locked and cannot be changed
         }
            
         if (err)
         {
             // error handling
         }

   .. group-tab:: python
   
      .. py:method:: Tcam.PropertyBase.is_available()
                        
         :exception: May raise an `GLib.Error` when fetching the property fails
         :returns: Returns if this property is available.

      .. code-block:: python
                  
         try:
             if base_property.is_available():
                 # property is locked and cannot be changed
             except GLib.Error as err:
                 # error handling


.. _tcam_property_base_is_locked:
                    
tcam_property_base_is_locked
----------------------------
                
| Check if property is currently locked.
| If the property is locked it means that no write actions are possible, due to another property preventing such actions.
| A typical example would be ExposureAuto locking ExposureTime.

.. tabs::

   .. group-tab:: c
         
      .. c:function:: gboolean tcam_property_base_is_locked (TcamPropertyBase* self, GError** err)

         :param self: The :ref:`TcamPropertyBase` instance
         :param err: A :c:type:`GError` pointer, may be NULL
         :returns: Returns :c:type:`TRUE` if the property is currently locked, otherwise false.

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
   
      .. py:method:: Tcam.PropertyBase.is_locked()
                        
         :exception: May raise an `GLib.Error` when fetching the property fails
         :returns: Returns if this property is locked.

      .. code-block:: python
                     
         try:
             if base_property.is_locked():
                 # property is locked and cannot be changed
         except GLib.Error as err:
             # error handling
      

.. _TcamPropertyBoolean:
                
TcamPropertyBoolean
###################

| Property representing a bool value.
| Can be obtained by casting a :c:type:`TcamPropertyBase` with `TCAM_PROPERTY_BOOLEAN(TcamPropertyBase*)`.
| Inherits from :ref:`TcamPropertyBase`.
| Upon cleanup `g_object_unref` has to be called on the property.

.. _tcam_property_boolean_get_value:

tcam_property_boolean_get_value
-------------------------------

.. tabs::

   .. group-tab:: c
         
      .. c:function:: gboolean tcam_property_boolean_get_value (TcamPropertyBoolean* self, GError** err)

         :param self: The :ref:`TcamPropertyBoolean` instance
         :param err: A :c:type:`GError` pointer, may be NULL
         :returns: Returns the current value of the property.

      .. code-block:: c

         TcamPropertyBoolean* bool_property = TCAM_PROPERTY_BOOLEAN(base_property);
         GError* err = NULL;

         gboolean current_value = tcam_property_boolean_get_value(bool_property, &err);

         if (err)
         {
             // error handling
         }
                  
   .. group-tab:: python

      .. py:method:: Tcam.PropertyBoolean.get_value()
                        
         :exception: May raise an `GLib.Error` when fetching the property fails
         :returns: Returns the value of this property

      .. code-block:: python

         try:
             current_value = base_property.get_value()
         except GLib.Error as err:
             # error handling
         
.. _tcam_property_boolean_set_value:
                
tcam_property_boolean_set_value
-------------------------------

.. tabs::

   .. group-tab:: c

      .. c:function:: void tcam_property_boolean_set_value (TcamPropertyBoolean* self, gboolean value, GError** err)

         :param self: The :ref:`TcamPropertyBoolean` instance
         :param value: The new value to set.
         :param err: A :c:type:`GError` pointer, may be NULL

      .. code-block:: c

         TcamPropertyBoolean* bool_property = TCAM_PROPERTY_BOOLEAN(base_property);
         GError* err = NULL;

         
         gboolean new_value = TRUE;
         tcam_property_boolean_set_value(bool_property, new_value, &err);

         if (err)
         {
             // error handling
         }
                               
   .. group-tab:: python

      .. py:method:: Tcam.PropertyBoolean.set_value(value)
                        
         :param value: The new value to set.
         :exception: May raise an `GLib.Error` when setting the property fails

      .. code-block:: python

         try:
             new_value = True
             base_property.set_value(new_value)
         except GLib.Error as err:
             # error handling


              
              
.. _tcam_property_boolean_get_default:
                
tcam_property_boolean_get_default
---------------------------------                

| Query for the default value of a boolean property.
| This might fail with `TCAM_ERROR_PROPERTY_DEFAULT_NOT_AVAILABLE` if no default value is available for this property.

.. tabs::

   .. group-tab:: c
         
      .. c:function:: gboolean tcam_property_boolean_get_default (TcamPropertyBoolean* self, GError** err)

         :param self: The :ref:`TcamPropertyBoolean` instance
         :param err: A :c:type:`GError` pointer, may be NULL
         :returns: Returns the default value of the property

      .. code-block:: c

         TcamPropertyBoolean* bool_property = TCAM_PROPERTY_BOOLEAN(base_property);
         GError* err = NULL;

         bool default_value = tcam_property_boolean_get_default(bool_property, &err);

         if (err)
         {
             // error handling
         }
                               
   .. group-tab:: python

      .. py:method:: Tcam.PropertyBoolean.get_default()
                        
         :exception: May raise an `GLib.Error` when fetching the property fails
         :returns: Returns the default value of the property

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


.. tabs::

   .. group-tab:: c

      .. c:function:: gint64 tcam_property_integer_get_value (TcamPropertyInteger* self, GError** err)

         :param self: The :ref:`TcamPropertyInteger` instance
         :param err: A :c:type:`GError` pointer, may be NULL
         :returns: Returns the current value of the property

      .. code-block:: c

         TcamPropertyInteger* int_property = TCAM_PROPERTY_INTEGER(base_property);
         GError* err = NULL;

         gint64 current_value = tcam_property_integer_get_value(int_property, &err);

         if (err)
         {
             // error handling
         }
                  
   .. group-tab:: python
   
      .. py:method:: Tcam.PropertyInteger.get_value()
                        
         :exception: May raise an `GLib.Error` when fetching the property fails
         :returns: Returns the value of the property

      .. code-block:: python

         try:
             current_value = base_property.get_value()
         except GLib.Error as err:
             # error handling


         
.. _tcam_property_integer_set_value:
                  
tcam_property_integer_set_value
-------------------------------

.. tabs::

   .. group-tab:: c

      .. c:function:: void tcam_property_integer_set_value (TcamPropertyInteger* self, gint64 value, GError** err)

         :param self: The :ref:`TcamPropertyInteger` instance
         :param value: The new value to set.
         :param err: A :c:type:`GError` pointer, may be NULL

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
   
      .. py:method:: Tcam.PropertyInteger.set_value(value)
                        
         :param value: The new value to set.
         :exception: May raise an `GLib.Error` when setting the property fails

      .. code-block:: python

         try:
             new_value = 500
             base_property.set_value(new_value)
         except GLib.Error as err:
             # error handling

            
.. _tcam_property_integer_get_range:
                  
tcam_property_integer_get_range
-------------------------------

.. tabs::

   .. group-tab:: c

      .. c:function:: void tcam_property_integer_get_range (TcamPropertyInteger* self, gint64* min_value, gint64* max_value, gint64* step_value, GError** err)

         :param self: The :ref:`TcamPropertyInteger` instance
         :param min_value: out value. pointer to a int64 that will be filled with the minimal value the property may have. May be `NULL`.
         :param max_value: out value. pointer to a int64 that will be filled with the maximum value the property may have. May be `NULL`.
         :param step_value: out value. pointer to a int64 that will be filled with the step size between values. May be `NULL`.
         :param err: A :c:type:`GError` pointer, may be NULL

      .. code-block:: c

         TcamPropertyInteger* int_property = TCAM_PROPERTY_INTEGER(base_property);
         GError* err = NULL;
         int64 min_value;
         int64 max_value;
         int64 step_value;
         tcam_property_integer_get_range(int_property,
                                                  &min_value,
                                                  &max_value,
                                                  &step_value,
                                                  &err);

         if (err)
         {
             // error handling
         }
                  
   .. group-tab:: python
      
      .. py:method:: [min,max,step] = Tcam.PropertyInteger.get_range()
                        
         :exception: May raise an `GLib.Error` when fetching the property fails
         :returns: Returns the range and step of the property
         :retval min: The minimum for this property
         :retval max: The maximum for this property
         :retval step: The step for this property

      .. code-block:: python

         try:
             min_value, max_value, step_value = base_property.get_range()
         except GLib.Error as err:
             # error handling

            

.. _tcam_property_integer_get_default:
                  
tcam_property_integer_get_default
---------------------------------

.. tabs::

   .. group-tab:: c

      .. c:function:: gint64 tcam_property_integer_get_default (TcamPropertyInteger* self, GError** err)

         :param self: The :ref:`TcamPropertyInteger` instance
         :param err: A :c:type:`GError` pointer, may be NULL
         :returns: Returns the default value of the property

      .. code-block:: c

         TcamPropertyInteger* int_property = TCAM_PROPERTY_INTEGER(base_property);
         GError* err = NULL;
         int64 default_value = tcam_property_integer_get_default(int_property, &err);

         if (err)
         {
             // error handling
         }
                               
   .. group-tab:: python
         
      .. py:method:: Tcam.PropertyInteger.get_default()
                        
         :exception: May raise an `GLib.Error` when fetching the property fails
         :returns: Returns the default value of the property

      .. code-block:: python

         try:
             default_value = base_property.get_default()
         except GLib.Error as err:
             # error handling


         
.. _tcam_property_integer_get_unit:
                  
tcam_property_integer_get_unit
------------------------------

.. tabs::

   .. group-tab:: c
   
      .. c:function:: const gchar* tcam_property_integer_get_unit (TcamPropertyInteger* self);

         :param self: The :ref:`TcamPropertyInteger` instance
         :returns: (transfer none): String describing the property unit. The string is valid until the instance is released.
         :retval NULL: If the unit is not available, NULL is returned.

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
            
      .. py:method:: Tcam.PropertyInteger.get_unit()
                        
         :exception: May raise an `GLib.Error` when fetching the property fails
         :returns: Returns the unit name for this property

      .. code-block:: python

         try:
             unit = base_property.get_unit()
         except GLib.Error as err:
             # error handling
         

.. _tcam_property_integer_get_representation:
                  
tcam_property_integer_get_representation
----------------------------------------

.. tabs::

   .. group-tab:: c
            
      .. c:function:: TcamPropertyIntRepresentation tcam_property_integer_get_representation (TcamPropertyInteger* self)

         :param self: The :ref:`TcamPropertyInteger` instance
         :returns: :ref:`TcamPropertyIntRepresentation` describing the recommended way of displaying the property.

      .. code-block:: c

         TcamPropertyInteger* int_property = TCAM_PROPERTY_INTEGER(base_property);
         GError* err = NULL;
         TcamPropertyIntRepresentation representation = tcam_property_integer_get_representation(int_property, &err);

         if (err)
         {
             // error handling
         }
                               
   .. group-tab:: python

      .. py:method:: Tcam.PropertyInteger.get_representation()
                        
         :exception: May raise an `GLib.Error` when fetching the property fails
         :returns: Returns the :ref:`TcamPropertyIntRepresentation` for this property

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

.. tabs::

   .. group-tab:: c
   
      .. c:function:: gdouble tcam_property_float_get_value (TcamPropertyFloat* self, GError** err)

         :param self: The :ref:`TcamPropertyFloat` instance
         :param err: A :c:type:`GError` pointer, may be NULL
         :returns: Returns the current value of the property

      .. code-block:: c

         TcamPropertyFloat* float_property = TCAM_PROPERTY_FLOAT(base_property);
         GError* err = NULL;

         double current_value = tcam_property_float_get_value(float_property, &err);

         if (err)
         {
             // error handling
         }
                               
   .. group-tab:: python

      .. py:method:: Tcam.PropertyFloat.get_value()
                        
         :exception: May raise an `GLib.Error` when fetching the property fails
         :returns: Returns the current value for this property

      .. code-block:: python

         try:
             current_value = base_property.get_value()
         except GLib.Error as err:
             # error handling

         
.. _tcam_property_float_set_value:
                
tcam_property_float_set_value
-----------------------------

.. tabs::

   .. group-tab:: c


      .. c:function:: void tcam_property_float_set_value (TcamPropertyFloat* self, gdouble value, GError** err)

         :param self: The :ref:`TcamPropertyFloat` instance
         :param value: The new value to set.
         :param err: A :c:type:`GError` pointer, may be NULL
         
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
   
      .. py:method:: Tcam.PropertyFloat.set_value(value)
                        
         :param value: The new value to set.
         :exception: May raise an `GLib.Error` when fetching the property fails

      .. code-block:: python

         try:
             new_value = 30000.0
             base_property.set_value(new_value)
         except GLib.Error as err:
             # error handling


             
.. _tcam_property_float_get_range:
                
tcam_property_float_get_range
-----------------------------

.. tabs::

   .. group-tab:: c

      .. c:function:: void tcam_property_float_get_range (TcamPropertyFloat* self, gdouble* min_value, gdouble* max_value, gdouble* step_value, GError** err);

         :param self: The :ref:`TcamPropertyFloat` instance
         :param min_value: out value. pointer to a gdouble that will be filled with the minimal value the property may have. May be `NULL`.
         :param max_value: out value. pointer to a gdouble that will be filled with the maximum value the property may have. May be `NULL`.
         :param step_value: out value. pointer to a gdouble that will be filled with the step size between values. May be `NULL`.
         :param err: A :c:type:`GError` pointer, may be NULL

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
         
      .. py:method:: [min,max,step] = Tcam.PropertyFloat.get_range()
                        
         :exception: May raise an `GLib.Error` when fetching the property fails
         :returns: Returns the range and step of the property
         :retval min: The minimum for this property
         :retval max: The maximum for this property
         :retval step: The step for this property

      .. code-block:: python

         try:
             min_value, max_value, step_value = base_property.get_range()
         except GLib.Error as err:
             # error handling


             
.. _tcam_property_float_get_default:
                
tcam_property_float_get_default
-------------------------------

.. tabs::

   .. group-tab:: c
   
      .. c:function:: gdouble tcam_property_float_get_default (TcamPropertyFloat* self, GError** err);

         :param self: The :ref:`TcamPropertyFloat` instance
         :param err: A :c:type:`GError` pointer, may be NULL
         :returns: Returns the default value of the property

      .. code-block:: c

         TcamPropertyFloat* float_property = TCAM_PROPERTY_FLOAT(base_property);
         GError* err = NULL;
         double default_value = tcam_property_float_get_default(float_property, &err);

         if (err)
         {
             // error handling
         }
                      
   .. group-tab:: python

       .. py:method:: Tcam.PropertyFloat.get_default()
                        
         :exception: May raise an `GLib.Error` when fetching the property fails
         :returns: Returns the default value of the property

      .. code-block:: python

         try:
             default_value = base_property.get_default()
         except GLib.Error as err:
             # error handling

.. _tcam_property_float_get_unit:
                
tcam_property_float_get_unit
----------------------------

.. tabs::

   .. group-tab:: c

      .. c:function:: const gchar* tcam_property_float_get_unit (TcamPropertyFloat* self);

         :param self: The :ref:`TcamPropertyFloat` instance
         :returns: (transfer none): String describing the property unit. The string is valid until the instance is released.
         :retval NULL: If the unit is not available, NULL is returned.

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
   
       .. py:method:: Tcam.PropertyFloat.get_unit()
                        
         :exception: May raise an `GLib.Error` when fetching the property fails
         :returns: Returns the unit name of the property

      .. code-block:: python

         try:
             unit = base_property.get_unit()
         except GLib.Error as err:
             # error handling

.. _tcam_property_float_get_representation:
                
tcam_property_float_get_representation
--------------------------------------

.. tabs::

   .. group-tab:: c

      .. c:function:: TcamPropertyFloatRepresentation tcam_property_float_get_representation (TcamPropertyFloat* self);
      
         :param self: The :ref:`TcamPropertyFloat` instance
         :returns: :ref:`TcamPropertyFloatRepresentation` describing the recommended way of displaying the property.

      .. code-block:: c

         TcamPropertyFloat* float_property = TCAM_PROPERTY_FLOAT(base_property);
         GError* err = NULL;
         TcamPropertyFloatRepresentation representation = tcam_property_float_get_representation(float_property, &err);

         if (err)
         {
             // error handling
         }
                      
   .. group-tab:: python
   
       .. py:method:: Tcam.PropertyFloat.get_unit()
                        
         :exception: May raise an `GLib.Error` when fetching the property fails
         :returns: Returns the :ref:`TcamPropertyFloatRepresentation` of the property

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

.. tabs::

   .. group-tab:: c

      .. c:function:: const gchar* tcam_property_enumeration_get_value (TcamPropertyEnumeration* self, GError** err);

         :param self: The :ref:`TcamPropertyEnumeration` instance
         :param err: A :c:type:`GError` pointer, may be NULL
         :returns: (transfer none): Returns the current value of the property. `NULL` on error. The string is valid until the instance is released.

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
   
       .. py:method:: Tcam.PropertyEnumeration.get_value()
                        
         :exception: May raise an `GLib.Error` when fetching the property fails
         :returns: Returns current value of the property

      .. code-block:: python

         try:
             current_value = base_property.get_value()
         except GLib.Error as err:
             # error handling
         
.. _tcam_property_enumeration_set_value:
   
tcam_property_enumeration_set_value
-----------------------------------

.. tabs::

   .. group-tab:: c
   
      .. c:function:: void tcam_property_enumeration_set_value (TcamPropertyEnumeration* self, const gchar* value, GError** err);

         :param self: The :ref:`TcamPropertyEnumeration` instance
         :param value: The new value to set
         :param err: A :c:type:`GError` pointer, may be NULL

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
      
       .. py:method:: Tcam.PropertyEnumeration.set_value(value)
                        
         :param value: The new value to set
         :exception: May raise an `GLib.Error` when fetching the property fails

      .. code-block:: python

         try:
             new_value = "entry"
             base_property.set_value(new_value)
         except GLib.Error as err:
             # error handling             

.. _tcam_property_enumeration_get_enum_entries:
                
tcam_property_enumeration_get_enum_entries
------------------------------------------
                


The caller takes ownership of the returned list and its values.
Call `g_slist_free_full(enum_entries, g_free)` when not done.

:param self: property instance
:param err: Pointer to GError pointer to be used in case of error. Can be `NULL`.
:returns: A GSList containing string values describing all possible property values. `NULL` on error.
:retval: GSList*

.. tabs::

   .. group-tab:: c

      .. c:function:: GSList* tcam_property_enumeration_get_enum_entries (TcamPropertyEnumeration* self, GError** err);

         :param self: The :ref:`TcamPropertyEnumeration` instance
         :param err: A :c:type:`GError` pointer, may be NULL
         :returns: (transfer full): A list of enumeration entries. All entries and the list itself must be freed by the caller.
         :retval NULL: Returns NULL on error.

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
   
       .. py:method:: Tcam.PropertyEnumeration.get_enum_entries()
                        
         :exception: May raise an `GLib.Error` when fetching the property fails
         :returns: Returns a list of strings for the enumeration entries

      .. code-block:: python   

         try:
             entry_list = base_property.get_enum_entries()
         except GLib.Error as err:
             # error handling


.. _tcam_property_enumeration_get_default:
   
tcam_property_enumeration_get_default
-------------------------------------

.. tabs::

   .. group-tab:: c
      
      .. c:function:: const gchar* tcam_property_enumeration_get_default (TcamPropertyEnumeration* self, GError** err);

         :param self: The :ref:`TcamPropertyEnumeration` instance
         :param err: A :c:type:`GError` pointer, may be NULL
         :returns: (transfer none): The default value for this property. The string is valid until this instance is released.
         :retval NULL: Returns `NULL` on error.

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
   
       .. py:method:: Tcam.PropertyEnumeration.get_default()
                        
         :exception: May raise an `GLib.Error` when fetching the property fails
         :returns: Returns the defaulkt value for this property.

      .. code-block:: python

         try:
             default_value = base_property.get_default()
         except GLib.Error as err:
             # error handling


.. _TcamPropertyCommand:
                
TcamPropertyCommand
###################

Property representing a command/button value.

An instance can be retrieved by casting a :ref:`TcamPropertyBase` pointer. `TCAM_PROPERTY_COMMAND(TcamPropertyBase*)`

Upon cleanup `g_object_unref` has to be called on the property.

.. _tcam_property_command_set_command:

tcam_property_command_set_command
---------------------------------

Execute the command.

.. tabs::

   .. group-tab:: c

      .. c:function:: void tcam_property_command_set_command (TcamPropertyCommand* self, GError** err);
         
         :param self: The :ref:`TcamPropertyInteger` instance
         :param err: A :c:type:`GError` pointer, may be NULL

      .. code-block:: c

         TcamPropertyCommand* command = TCAM_PROPERTY_COMMAND(base_property);
         GError* err = NULL;
         tcam_property_command_set_command(command, &err);

         if (err)
         {
             // error handling
         }

   .. group-tab:: python
            
       .. py:method:: Tcam.PropertyCommand.set_command()
                        
         :exception: May raise an `GLib.Error` when fetching the property fails

      .. code-block:: python

         try:
             base_property.set_command()
         except GLib.Error as err:
             # error handling

.. _TcamPropertyString:
                
TcamPropertyString
##################

| Property representing a string value.
| Inherits from :ref:`TcamPropertyBase`.
| Can be obtained by casting a :c:type:`TcamPropertyBase` with `TCAM_PROPERTY_STRING(TcamPropertyBase*)`.
| Upon cleanup `g_object_unref` has to be called on the property.

.. _tcam_property_string_get_value:

tcam_property_string_get_value
-------------------------------

.. tabs::

   .. group-tab:: c
         
      .. c:function:: char* tcam_property_string_get_value (TcamPropertyString* self, GError** err)

         :param self: The :ref:`TcamPropertyString` instance
         :param err: A :c:type:`GError` pointer, may be NULL
         :returns: (transfer full): Returns the current value of the property. This string must be freed by the caller

      .. code-block:: c

         TcamPropertyString* property = TCAM_PROPERTY_STRING(base_property);
         GError* err = NULL;

         char* current_value = tcam_property_string_get_value(property, &err);

         if (err)
         {
             // error handling
         }

         g_free(current_value);
                  
   .. group-tab:: python

      .. py:method:: Tcam.PropertyString.get_value()
                        
         :exception: May raise an `GLib.Error` when fetching the property fails
         :returns: Returns the value of this property

      .. code-block:: python

         try:
             current_value = base_property.get_value()
         except GLib.Error as err:
             # error handling
         

.. _tcam_property_string_set_value:
                
tcam_property_string_set_value
-------------------------------

.. tabs::

   .. group-tab:: c

      .. c:function:: void tcam_property_string_set_value (TcamPropertyString* self, const char* value, GError** err)

         :param self: The :ref:`TcamPropertyString` instance
         :param value: The new value to set.
         :param err: A :c:type:`GError` pointer, may be NULL

      .. code-block:: c

         TcamPropertyString* property = TCAM_PROPERTY_STRING(base_property);
         GError* err = NULL;

         const char* new_value = "test";
         tcam_property_string_set_value(property, new_value, &err);

         if (err)
         {
             // error handling
         }
                               
   .. group-tab:: python

      .. py:method:: Tcam.PropertyString.set_value(value)
                        
         :param value: The new value to set.
         :exception: May raise an `GLib.Error` when setting the property fails

      .. code-block:: python

         try:
             new_value = True
             base_property.set_value(new_value)
         except GLib.Error as err:
             # error handling


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
                    
.. cpp:enumerator:: TCAM_ERROR_TIMEOUT

                    | A function failed with a timeout.

.. cpp:enumerator:: TCAM_ERROR_UNKNOWN

                    | Catch all error code for things that do not fit other codes.
                    
.. cpp:enumerator:: TCAM_ERROR_NOT_IMPLEMENTED

                    | Generic not implemented error value.

.. cpp:enumerator:: TCAM_ERROR_PARAMETER_INVALID

                    | A parameter was not valid.

.. cpp:enumerator:: TCAM_ERROR_PROPERTY_NOT_IMPLEMENTED
.. cpp:enumerator:: TCAM_ERROR_PROPERTY_NOT_AVAILABLE

                    | Circumstances prevent this property from being usable.
                    | This is typically due to the selected stream format.
                    | e.g. BalanceWhite* not being usable when streaming mono.
                    
.. cpp:enumerator:: TCAM_ERROR_PROPERTY_NOT_WRITEABLE

                    | The property is either read only or temporarily locked.
                    | Call :ref:`tcam_property_base_is_locked` for verification.
                    
.. cpp:enumerator:: TCAM_ERROR_PROPERTY_VALUE_OUT_OF_RANGE

                    | Value is out of bounds.
                    | Check the `*_get_range` function for boundaries.

.. cpp:enumerator:: TCAM_ERROR_PROPERTY_DEFAULT_NOT_AVAILABLE

                    | The property has no default value.

.. cpp:enumerator:: TCAM_ERROR_PROPERTY_TYPE_INCOMPATIBLE

                    | The property is of a different type.

.. cpp:enumerator:: TCAM_ERROR_DEVICE_NOT_OPENED
                    
                    | No device has been opened that can offer properties.
                    | This typically means the GstElement is not in GST_STATE_READY or higher.
                    
.. cpp:enumerator:: TCAM_ERROR_DEVICE_LOST

                    | The device has been lost.
                    | This should be considered a fatal, unrecoverable error.
                    
.. cpp:enumerator:: TCAM_ERROR_DEVICE_NOT_ACCESSIBLE

                    | The property cannot query the device for data.


.. _tcampropertyaccess:
                    
TcamPropertyAccess
------------------

Indicates a static property access mode.

.. cpp:enum:: TcamPropertyAccess

.. cpp:enumerator:: TCAM_PROPERTY_ACCESS_RW

                    Read and write operations are available.

.. cpp:enumerator:: TCAM_PROPERTY_ACCESS_RO

                    Only read operations are available.

.. cpp:enumerator:: TCAM_PROPERTY_ACCESS_WO

                    Only write operations are available.

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

| A returned GError has to _always_ be freed by the user with g_error_free().
| The GError will contain a string describing the cause of the error and an error code.
| The message can be accessed through the member variable `message`.
| The error code can be accessed though the member variable `code`.
| The error code will be a :cpp:enum:`TcamError` enum entry.
