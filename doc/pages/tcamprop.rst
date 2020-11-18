########
TcamProp
########


This page describes the official gobject-introspection API.

.. note::
   The current version of this API is 0.1

Base Object
###########

All tiscamera gstreamer elements implement the TcamProp interface.
This interface allows access to all properties that the camera and software offer,
as well as possibilities for device discovery.

.. c:type:: TcamProp

    This object is typically a converted gstreamer element like tcambin or tcamautoexposure.

.. c:type:: GSList

    In tcamprop this is always a list with element-type utf8 which has to be deallocated via:

    Example:

    .. code-block:: c

        GSList* list = tcam_prop_get_device_serials( self );
        
        // ... do sth with list
        
        g_slist_free_full( list, ::g_free );

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


Device Discovery
################

The following functions can be used for device discovery.
Only tcamsrc and tcambin implement these.

.. c:function:: GSList* tcam_prop_get_device_serials(TcamProp* self)
                
   @self: a #TcamProp
  
   Retrieve a list of all connected device serial numbers
   
   Returns: (element-type utf8) (transfer full): a #GSList
   
        
.. c:function:: gboolean tcam_prop_get_device_info (TcamProp* self, const char* serial, char** name, char** identifier, char** connection_type)
                
   | @self: a #TcamProp
   | @serial: (in): serial number of camera to query
   | @name: (out) (optional): location to store an allocated string.
   |                          Use g_free() to free the returned string
   | @identifier: (out) (optional): location to store an allocated string.
   |                                Use g_free() to free the returned string
   | @connection_type: (out) (optional): location to store an allocated string.
   |                                     Use g_free() to free the returned string
                
   Get details of a given camera.

   Returns: True on success

.. c:function:: GSList* tcam_prop_get_device_serials_backend (TcamProp* self)

   @self: a #TcamProp

   Retrieve a list of all connected device serial numbers with the backend appended

   | Retrieved serials may appear multiple times but with different backends.
   | The format will always be `<serial>-<backend>`.
   | The contained strings will have to be freed by the user.
   | Call `g_slist_free_full(<list_var>, ::g_free)` to clear the list and the contained strings.

   Returns: (element-type utf8) (transfer full): a #GSList

Property I/O
############

tcam_prop_get_tcam_property_names
---------------------------------

.. c:function:: GSList* tcam_prop_get_tcam_property_names(TcamProp* self)

    Retrieve a list of all currently available properties.

    @self: a #TcamProp

    Returns: (element-type utf8) (transfer full): a #GSList


tcam_prop_get_tcam_property
---------------------------

.. c:function:: gboolean tcam_prop_get_tcam_property(TcamProp* self, const gchar* name, GValue* value, GValue* min, GValue* max, GValue* def, GValue* step, GValue* type, GValue* flags, GValue* category, GValue* group)

    Queries the specified property for its value, range, etc.

    @self: Pointer to #TcamProp instance that shall be queried.

    @name: Name of the property to query.

    Returns: A gboolean. TRUE if query could be answered and values filled.

.. list-table:: Arguments
   :header-rows: 1
   :widths: 10 20 50 10 10

   * - Name
     - Type
     - Description
     - In/Out
     - Optional
   * - self
     - TcamProp
     -
     - In
     - No
   * - name
     - const char*
     - identifying the property to query
     - In
     - No
   * - value
     - GValue*
     -
     - Out
     - Yes
   * - min
     - GValue*
     -
     - Out
     - Yes
   * - max
     - GValue*
     -
     - Out
     - Yes
   * - def
     - GValue*
     -
     - Out
     - Yes
   * - step
     - GValue*
     -
     - Out
     - Yes
   * - type
     - GValue*
     -
     - Out
     - Yes
   * - flags
     - GValue*
     -
     - Out
     - Yes
   * - category
     - GValue*
     -
     - Out
     - Yes
   * - group
     - GValue*
     -
     - Out
     - Yes

tcam_prop_get_tcam_menu_entries
-------------------------------

.. c:function:: GSList* tcam_prop_get_tcam_menu_entries( TcamProp* self, const gchar* name )

    If the specified property is a string list, this returnes the available string entries.

    @self: Pointer to #TcamProp instance that shall be queried.

    @name: Name of the property for which the menu entries shall be returned.

    Returns: A pointer to a #GSList containing the c-strings of all entries. (element-type utf8) (transfer full): a #GSList


tcam_prop_set_tcam_property
---------------------------

.. c:function:: gboolean tcam_prop_set_tcam_property( TcamProp* self, const gchar* name, const GValue* value )

    @self: Pointer to #TcamProp instance that shall be queried.

    @name: Name of the property.

    @value: Value that shall be set

    Returns: TRUE if setting this property was successful, otherwise FALSE.


tcam_prop_get_tcam_property_type
--------------------------------

.. c:function:: const gchar* tcam_prop_get_tcam_property_type( TcamProp* self, const gchar* name )

    Deprecated!

    Returns the 'type' of the specified property.

    @self: Pointer to #TcamProp instance that shall be queried.

    @name: Name of the property.

    Returns: A pointer to a c-string containing the type of the requested property. Returns NULL when property does not exist.
