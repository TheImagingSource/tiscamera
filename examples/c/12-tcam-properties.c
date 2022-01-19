/*
 * Copyright 2021 The Imaging Source Europe GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* This example will show you how to get/set the properties through a description string */

#include <gst/gst.h>
#include <stdio.h> /* printf and putchar */


gboolean block_until_state_change_done (GstElement* pipeline)
{
    while (TRUE)
    {
        GstState state;
        GstState pending;

        // wait 0.1 seconds for something to happen
        GstStateChangeReturn ret = gst_element_get_state(pipeline, &state, &pending, 100000000);

        if (ret == GST_STATE_CHANGE_SUCCESS || ret == GST_STATE_CHANGE_NO_PREROLL)
        {
            return TRUE;
        }
        else if (ret == GST_STATE_CHANGE_FAILURE)
        {
            printf("Failed to change state %s %s %s\n",
                   gst_element_state_change_return_get_name(ret),
                   gst_element_state_get_name(state),
                   gst_element_state_get_name(pending));

            return FALSE;
        }
    }
}


static void print_current_properties (GstElement* source)
{
    // Initialize the GValue
    GValue current_properties = G_VALUE_INIT;
    g_value_init(&current_properties, GST_TYPE_STRUCTURE);

    // Get the GObject property
    g_object_get_property(G_OBJECT(source), "tcam-properties", &current_properties);

    // get a string to print the current property state
    char* string = gst_structure_to_string(gst_value_get_structure(&current_properties));
    printf("Current properties:\n%s\n", string );
    g_free(string); // free the string

    g_value_unset( &current_properties );  // free the GstStructure in the GValue
}


int main (int argc, char* argv[])
{
    /* this line sets the gstreamer default logging level
       it can be removed in normal applications
       gstreamer logging can contain very useful information
       when debugging your application
       # see https://gstreamer.freedesktop.org/documentation/tutorials/basic/debugging-tools.html
       for further details
    */
    gst_debug_set_default_threshold(GST_LEVEL_WARNING);

    gst_init(&argc, &argv); // init gstreamer

    GError* err = NULL;

    // this is a placeholder definition
    // normally your pipeline would be defined here
    GstElement* pipeline = gst_parse_launch("tcambin name=source ! fakesink", &err);

    if (pipeline == NULL)
    {
        printf("Unable to create pipeline: %s\n", err->message);
        g_free(err);
        return 1;
    }

    GstElement* source = gst_bin_get_by_name(GST_BIN(pipeline), "source");

    const char* serial = NULL;

    if (serial != NULL)
    {
        GValue val = G_VALUE_INIT;
        g_value_init(&val, G_TYPE_STRING);
        g_value_set_static_string(&val, serial);

        g_object_set_property(G_OBJECT(source), "serial", &val);
    }

    // in the READY state the camera will always be initialized
    gst_element_set_state(source, GST_STATE_READY);

    // helper function to wait for async state change to be done
    if (!block_until_state_change_done(pipeline))
    {
        printf("Unable to start pipeline. \n");
        return 2;
    }

    print_current_properties( source );

    // Create a new structure
    GstStructure* new_property_struct = gst_structure_new_empty("tcam");

    // Change 2 properties so that we can see a 'difference'
    gst_structure_set(new_property_struct,
                      "ExposureAuto", G_TYPE_STRING, "Off",
                      "ExposureTime", G_TYPE_DOUBLE, 35000.0,
                      NULL );

    GValue new_state = G_VALUE_INIT;
    g_value_init(&new_state, GST_TYPE_STRUCTURE);
    gst_value_set_structure( &new_state, new_property_struct);

    // Set the new property settings
    g_object_set_property(G_OBJECT(source), "tcam-properties", &new_state);

    g_value_unset(&new_state);
    gst_structure_free(new_property_struct);

    // Print the property settings after the change above
    print_current_properties(source);

    // cleanup, reset state
    gst_element_set_state(source, GST_STATE_NULL);

    gst_object_unref(source);
    gst_object_unref(pipeline);

    return 0;
}
