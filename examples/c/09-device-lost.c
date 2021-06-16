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

#include <unistd.h> /* usleep */

#include "tcamprop.h" /* gobject introspection interface */


static gboolean stop_program = FALSE;
static GMainLoop* loop;


static gboolean starts_with (const char* a, const char* b)
{
    if (strncmp(a, b, strlen(b)) == 0)
    {
        return 1;
    }
    return 0;
}


static gboolean bus_callback (GstBus* bus,
                              GstMessage* message,
                              gpointer data)
{
    g_print("Got %s message\n", GST_MESSAGE_TYPE_NAME(message));

    switch (GST_MESSAGE_TYPE (message))
    {
        case GST_MESSAGE_ERROR:
        {
            GError* err;
            gchar* debug;

            gst_message_parse_error (message, &err, &debug);
            g_print ("Error: %s \n", err->message);

            const char* source_name = gst_object_get_name(message->src);

            // if you use tcamsrc directly this will be the name you give to the element
            // if (strcmp(source_name, "tcamsrc0") == 0)
            if (strcmp(source_name, "tcambin-source") == 0)
            {
                if (starts_with(err->message, "Device lost ("))
                {
                    char* s_str = strstr(err->message, "(");
                    const char* serial = strtok(s_str, "()");
                    printf("Device lost came from device with serial = %s\n", serial);
                }
            }

            g_error_free (err);
            g_free (debug);
            // device lost handling should be initiated here
            // this example simply stops plaback
            g_main_loop_quit(loop);
            break;
        }
        case GST_MESSAGE_INFO:
        {
            break;
        }
        case GST_MESSAGE_EOS:
        {
            /* end-of-stream */
            g_main_loop_quit(loop);
            break;
        }
        default:
        {
            /* unhandled message */
            break;
        }
    }
    return TRUE;
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
        gst_object_unref(source);
    }

    GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));

    int bus_watch_id = gst_bus_add_watch(bus, bus_callback, NULL);

    gst_object_unref(bus);


    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    printf("Disconnect your camera to trigger a device lost or press enter to stop the stream.\n");

    // we work with a event loop to be automatically
    // notified when a new messages occur.
    loop = g_main_loop_new (NULL, FALSE);
    g_main_loop_run (loop);

    g_main_loop_unref(loop);
    gst_element_set_state(pipeline, GST_STATE_NULL);

    gst_object_unref( pipeline );

    return 0;
}
