/*
 * Copyright 2020 The Imaging Source Europe GmbH
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

/* This example will show you how to get/set the JSON property description for a certain camera */

#include <gst/gst.h>

#include <stdio.h> /* printf and putchar */
#include "tcamprop.h" /* gobject introspection interface */



gboolean block_until_playing (GstElement* pipeline)
{
    while (TRUE)
    {
        GstState state;
        GstState pending;

        // wait 0.1 seconds for something to happen
        GstStateChangeReturn ret = gst_element_get_state(pipeline ,&state, &pending, 100000000);

        if (ret == GST_STATE_CHANGE_SUCCESS)
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


int main (int argc, char *argv[])
{
    /* this line sets the gstreamer default logging level
       it can be removed in normal applications
       gstreamer logging can contain verry useful information
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
        GValue val = {};
        g_value_init(&val, G_TYPE_STRING);
        g_value_set_static_string(&val, serial);

        g_object_set_property(G_OBJECT(source), "serial", &val);
    }

    // in the READY state the camera will always be initialized
    // in the PLAYING state additional properties may appear from gstreamer elements
    gst_element_set_state(source, GST_STATE_PLAYING);

    // helper function to ensure we have the right state
    // alternatively wait for the first image
    if (!block_until_playing(pipeline))
    {
        printf("Unable to start pipeline. \n");
    }

    // Device is now in a state for interactions
    GValue state = G_VALUE_INIT;
    g_value_init(&state, G_TYPE_STRING);

    //We print the properties for a before/after comparison,
    g_object_get_property(G_OBJECT(source), "state", &state);

    printf("State of device is:\n%s", g_value_get_string(&state));

    // Change JSON description here
    // not part of this example

    // second print for the before/after comparison
    g_object_set_property(G_OBJECT(source), "state", &state);

    //reread state to see if anything changed
    g_object_get_property(G_OBJECT(source), "state", &state);

    printf("State of device is:\n%s", g_value_get_string(&state));

    // cleanup, reset state
    gst_element_set_state(source, GST_STATE_NULL);

    gst_object_unref(source);
    gst_object_unref(pipeline);
    g_value_unset(&state);

    return 0;
}
