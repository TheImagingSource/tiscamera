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



int main (int argc, char *argv[])
{
    gst_init(&argc, &argv); // init gstreamer

    /* create a tcambin to retrieve device information */
    GstElement* source = gst_element_factory_make("tcambin", "source");

    const char* serial = NULL;

    if (serial != NULL)
    {
        GValue val = {};
        g_value_init(&val, G_TYPE_STRING);
        g_value_set_static_string(&val, serial);

        g_object_set_property(G_OBJECT(source), "serial", &val);
    }

    /* in the READY state the camera will always be initialized */
    gst_element_set_state(source, GST_STATE_READY);

    /* Device is now in a state for interactions */

    GValue state = G_VALUE_INIT;
    /*
      We print the properties for a before/after comparison,
    */
    g_object_get_property(G_OBJECT(source), "state", &state);

    printf("State of device is:\n%s", g_value_get_string(&state));
    /*
      Change JSON description here
    */
    // not part of this example

    /*
      second print for the before/after comparison
    */
    g_object_set_property(G_OBJECT(source), "state", &state);

    /* cleanup, reset state */
    gst_element_set_state(source, GST_STATE_NULL);

    gst_object_unref(source);
    g_value_unset(&state);

    return 0;
}
