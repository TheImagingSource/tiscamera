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

/* This example will show you how to save a videostream into a file */

#include <stdio.h>
#include <string.h>
#include <gst/gst.h>
#include <tcamprop.h>

void print_help ()
{
    printf("Print available gstreamer caps for device.\n"
           "Usage:\n\nsave-video [<serial>]\n"
           "Help options:\n\t-h, --help\t\tPrint this text.\n"
           "\n\n");
}


int main (int argc, char *argv[])
{

    gst_init(&argc, &argv); // init gstreamer

    char* serial = NULL; // the serial number of the camera we want to use

    if (argc > 1)
    {
        if (strcmp("-h", argv[1]) == 0 || strcmp("--help", argv[1]) == 0)
        {
            print_help();
            return 0;
        }
        else
        {
            serial = argv[1];
        }
    }

    GstElement* source = gst_element_factory_make("tcambin", "source");

    if (serial != NULL)
    {
        GValue val = {};
        g_value_init(&val, G_TYPE_STRING);
        g_value_set_static_string(&val, serial);

        g_object_set_property(G_OBJECT(source), "serial", &val);
    }

    GstElement* pipeline = gst_pipeline_new("pipeline");

    GstElement* convert = gst_element_factory_make("videoconvert", "convert");
    GstElement* encoder = gst_element_factory_make("x264enc", "encoder");
    GstElement* mux = gst_element_factory_make("mpegtsmux", "mux");
    GstElement* sink = gst_element_factory_make("filesink", "filesink");

    g_object_set(G_OBJECT(sink), "location", "/tmp/stream.ts");

    gst_bin_add(GST_BIN(pipeline), source);
    gst_bin_add(GST_BIN(pipeline), convert);
    gst_bin_add(GST_BIN(pipeline), encoder);
    gst_bin_add(GST_BIN(pipeline), mux);
    gst_bin_add(GST_BIN(pipeline), sink);

    gst_element_link(source, convert);
    gst_element_link(convert, encoder);
    gst_element_link(encoder, mux);
    gst_element_link(mux, sink);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    printf("Press 'q' then 'enter' to stop the stream.\n");
    while(0 == 0)
    {
        char c = getchar();

        if (c == 'q')
        {
            break;
        }
    }

    // this stops the pipeline and frees all resources
    gst_element_set_state(pipeline, GST_STATE_NULL);

    /* the pipeline automatically handles all elements that have been added to it.
       thus they do not have to be cleaned up manually */
    gst_object_unref(pipeline);

    return 0;
}
