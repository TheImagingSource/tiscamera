########################
Property-loading/saving
########################

To allow for an easy serializing/desrializing of gstreamer filter state :ref:`tcambin` and :ref:`tcamsrc` have gobject properties
`tcam-properties` and `tcam-properties-json`.

You can use these properties to load/save the current tcam-properties state.

----------------
tcam-properties
----------------

The property `tcam-properties` allows for easy initialization at the command line. 

E.g.:

.. code-block:: sh

    gst-launch-1.0 tcambin tcam-properties=tcam,ExposureAuto=Off,ExposureTime=33333 ! ...

`tcam-properties` is of type `GstStructure`. You can read/write this structure via the gobject interface.

Setting several properties via `GstStructure`:

.. code-block:: c

    GstElement* src = ...;

    // Create a new structure
    GstStructure* new_property_struct = gst_structure_new_empty( "tcam" );

    // Change 2 properties so that we can see a 'difference'
    gst_structure_set( new_property_struct, 
                        "ExposureAuto", G_TYPE_STRING, "Off", 
                        "ExposureTime", G_TYPE_DOUBLE, 35000., 
                        NULL );

    GValue new_state = G_VALUE_INIT;
    g_value_init( &new_state, GST_TYPE_STRUCTURE );
    gst_value_set_structure( &new_state, new_property_struct );

    // Set the new property settings
    g_object_set_property(G_OBJECT(source), "tcam-properties", &new_state);

Reading the `GstStructure` of a opened device:

.. code-block:: c

    GstElement* src = ...;

    // Initialize the GValue
    GValue current_properties = G_VALUE_INIT;
    g_value_init(&current_properties, GST_TYPE_STRUCTURE);

    // Get the GObject property
    g_object_get_property(G_OBJECT(source), "tcam-properties", &current_properties);

    // get a string to print the current property state
    char* string = gst_structure_to_string( gst_value_get_structure(&current_properties) );
    printf("Current properties:\n%s\n", string );
    g_free( string ); // free the string

    g_value_unset( &current_properties );  // free the Gststurcture in the GValue

Note:
* If a property is locked when loading it, writing to the property is retried after all other properties are written. (This circumvents the problem of property order for e.g. "ExposureTime" and "ExposureAuto")
* Failed writing/reading of properties gets logged to the gstreamer log.
* Writing to the property in `GST_STATE_NULL` sets an internal cache which gets applied in the state transition to `GST_STATE_READY`
    
---------------------
tcam-properties-json
---------------------

The property `tcam-properties-json` provides the current device properties as a json string.

A simple state dump would look like this:

.. code-block:: json
                
    {
        "ExposureTime": 35000.0,
        "ExposureAuto": "Off"
    }

A Property consists out of the fields 'name' and 'value'.

- name is a string containing the property identifier.
- value contains the actual value. The interpretation of this field
  is done automatically.

In the case of an error a message will the written to TCAM_LOG.
The property will be ignored.

Properties of the type 'button' are not added to a state description.