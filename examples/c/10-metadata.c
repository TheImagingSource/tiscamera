/*
 * Copyright 2019 The Imaging Source Europe GmbH
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
#include <gst/video/video.h>
#include <unistd.h>
#include <stdlib.h>
#include <tcamprop.h>

#include "gstmetatcamstatistics.h"


static gboolean meta_struc_print (GQuark field_id,
                           const GValue* value,
                           gpointer user_data)
{
    // GQuark is a gobject internal ID for strings
    // we call the function g_quark_to_string to get the name of the field
    // value is a simple, generic value
    // user_data can contain things you need for further processing

    printf("%s: ", g_quark_to_string(field_id));

    if (G_VALUE_TYPE(value) == G_TYPE_BOOLEAN)
    {
        gboolean val = g_value_get_boolean(value);
        if (val)
        {
            printf(" true\n");
        }
        else
        {
            printf(" false\n");
        }
    }
    else if (G_VALUE_TYPE(value) == G_TYPE_DOUBLE)
    {
        double val = g_value_get_double(value);
        printf("%f\n", val);
    }
    else if (G_VALUE_TYPE(value) == G_TYPE_UINT64)
    {
        guint64 val = g_value_get_uint64(value);
        printf("%lu\n", val);
    }
    else
    {
        printf("value type not implemented\n");
    }

    return TRUE;
}

/*
  This function will be called in a separate thread when our appsink
  says there is data for us. user_data has to be defined when calling g_signal_connect.
 */
static GstFlowReturn callback (GstElement* sink, void* user_data)
{
    printf("new sample\n");

    GstSample* sample = NULL;
    /* Retrieve the buffer */
    g_signal_emit_by_name (sink, "pull-sample", &sample, NULL);

    if (sample)
    {
        static guint framecount = 0;
        int pixel_data = -1;

        framecount++;

        GstBuffer* buffer = gst_sample_get_buffer(sample);
        GstMapInfo info; // contains the actual image
        GstCaps* c = gst_sample_get_caps(sample);

        GstMeta* meta = gst_buffer_get_meta(buffer, g_type_from_name("TcamStatisticsMetaApi"));

        if (meta)
        {
            printf("We have meta\n");
        }
        else
        {
            g_warning("No meta data available\n");
        }

        GstStructure* struc = ((TcamStatisticsMeta*)meta)->structure;

        // this prints all contained fields
        gst_structure_foreach(struc, meta_struc_print, NULL);

        // to only print selective fields
        // read the documentation
        // https://www.theimagingsource.com/documentation/tiscamera/tcam-gstreamer.html#metadata
        // concerning available fields and call them manually by name

        /*
          guint64 frame_count = 0;
          gst_structure_get_uint64(struc, "frame_count", &frame_count);
          printf("frame_count: %ul\n", frame_count);
        */

        // delete our reference so that gstreamer can handle the sample
        gst_sample_unref (sample);
    }
    return GST_FLOW_OK;
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

    gst_init(&argc, &argv);

    const char* serial = NULL; // set this if you do not want the first found device

    GError* err = NULL;

    // some conversion elements will drop the metadata
    // for the sake of this example we will retrieve buffers
    // directly from the src
    const char* pipeline_str = "tcamsrc name=source ! appsink name=sink";

    GstElement* pipeline = gst_parse_launch(pipeline_str, &err);

    /* test for error */
    if (pipeline == NULL)
    {
        printf("Could not create pipeline. Cause: %s\n", err->message);
        return 1;
    }

    if( serial != NULL )
    {
        GstElement* source = gst_bin_get_by_name( GST_BIN( pipeline ), "source" );
        GValue val = {};
        g_value_init( &val, G_TYPE_STRING );
        g_value_set_static_string( &val, serial );

        g_object_set_property( G_OBJECT( source ), "serial", &val );
        gst_object_unref( source );
    }

    /* retrieve the appsink from the pipeline */
    GstElement* sink = gst_bin_get_by_name(GST_BIN(pipeline), "sink");

    // tell appsink to notify us when it receives an image
    g_object_set(G_OBJECT(sink), "emit-signals", TRUE, NULL);

    // tell appsink what function to call when it notifies us
    g_signal_connect(sink, "new-sample", G_CALLBACK(callback), NULL);

    gst_object_unref(sink);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    g_print("Press 'enter' to stop the stream.\n");
    /* wait for user input to end the program */
    getchar();

    // this stops the pipeline and frees all resources
    gst_element_set_state(pipeline, GST_STATE_NULL);

    /*
      the pipeline automatically handles
      all elements that have been added to it.
      thus they do not have to be cleaned up manually
    */
    gst_object_unref(pipeline);

    return 0;
}
