/*
 * Copyright 2016 The Imaging Source Europe GmbH
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

/* This example will show you how to list the formats your device offers */

#include <gst/gst.h>

#include <stdio.h> /* printf and putchar */

#include "tcamprop.h" /* gobject introspection interface */


int main (int argc, char *argv[])
{
    gst_init(&argc, &argv); // init gstreamer

    char* serial = NULL; // set this if you do not want the first found device

    /* create a tcambin to retrieve device information */
    GstElement* source = gst_element_factory_make("tcambin", "source");

    if (serial != NULL)
    {
        GValue val = {};
        g_value_init(&val, G_TYPE_STRING);
        g_value_set_static_string(&val, serial);

        g_object_set_property(G_OBJECT(source), "serial", &val);
    }

    /* Setting the state to ready ensures that all resources are initialized
       and that we really get all format capabilities */
    gst_element_set_state(source, GST_STATE_READY);

    GstPad* pad = gst_element_get_static_pad(source, "src");

    GstCaps* caps = gst_pad_query_caps(pad, NULL);

    for (unsigned int i = 0; i < gst_caps_get_size(caps); ++i)
    {
        GstStructure* structure = gst_caps_get_structure(caps, i);

        printf("%s\n", gst_structure_to_string(structure));
    }

    gst_object_unref(source);

    return 0;
}
