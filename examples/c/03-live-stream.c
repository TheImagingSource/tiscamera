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

/* This example will show you how to start a live stream from your camera */

#include <gst/gst.h>

#include <stdio.h> /* printf and putchar */

#include "tcamprop.h" /* gobject introspection interface */


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

    const char* serial = NULL; // set this if you do not want the first found device

    GError* err = NULL;

    GstElement* pipeline = gst_parse_launch("tcambin name=source ! videoconvert ! ximagesink", &err);

    /* test for error */
    if (pipeline == NULL)
    {
        printf("Could not create pipeline. Cause: %s\n", err->message);
        return 1;
    }

    if (serial != NULL)
    {
        GstElement* source = gst_bin_get_by_name(GST_BIN(pipeline), "source");
        GValue val = {};
        g_value_init(&val, G_TYPE_STRING);
        g_value_set_static_string(&val, serial);

        g_object_set_property(G_OBJECT(source), "serial", &val);

        gst_object_unref( source );
    }

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    printf("Press enter to stop the stream.\n");
    getchar();

    gst_element_set_state(pipeline, GST_STATE_NULL);

    gst_object_unref( pipeline );

    return 0;
}
