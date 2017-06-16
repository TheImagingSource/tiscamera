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

/* This example will show you how to receive data from gstreamer in your application
   and how to get the actual iamge data */

#include <stdio.h>
#include <string.h>
#include <gst/gst.h>
#include <tcamprop.h>

void print_help ()
{
    printf("Print available gstreamer caps for device.\n"
           "Usage:\n\trigger-images [<serial>]\n"
           "Help options:\n\t-h, --help\t\tPrint this text.\n"
           "\n\n");
}


/*
  This function will be called in a separate thread when our appsink
  says there is data for us. user_data has to be defined when calling g_signal_connect.
 */
GstFlowReturn callback (GstElement* sink, void* user_data)
{
    GstSample* sample = NULL;
    /* Retrieve the buffer */
    g_signal_emit_by_name (sink, "pull-sample", &sample, NULL);

    if (sample)
    {
        g_print ("*");

        /* if you want to have information about the format the image has
           you can look at the caps */
        /* GstCaps* caps gst_sample_get_caps(sample) */

        GstBuffer* buffer = gst_sample_get_buffer(sample);
        GstMapInfo info; // contains the actual image
        if (gst_buffer_map(buffer, &info, GST_MAP_READ))
        {
            unsigned char* data = info.data;

            /* do things here */

            gst_buffer_unmap(buffer, &info);
        }
        // delete our reference so that gstreamer can handle the sample
        gst_sample_unref (sample);
    }
    return GST_FLOW_OK;
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
    GstElement* sink = gst_element_factory_make("appsink", "sink");

    // tell appsink to notify us when it receives an image
    g_object_set(G_OBJECT(sink), "emit-signals", TRUE);

    g_signal_connect(sink, "new-sample", G_CALLBACK(callback), NULL);

    gst_bin_add(GST_BIN(pipeline), source);
    gst_bin_add(GST_BIN(pipeline), convert);
    gst_bin_add(GST_BIN(pipeline), sink);

    gst_element_link(source, convert);
    gst_element_link(convert, sink);

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
