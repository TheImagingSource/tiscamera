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

/* This example will show you how to trigger images */

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
    GstElement* display = gst_element_factory_make("ximagesink", "sink");


    gst_bin_add(GST_BIN(pipeline), source);
    gst_bin_add(GST_BIN(pipeline), convert);
    gst_bin_add(GST_BIN(pipeline), display);

    gst_element_link(source, convert);
    gst_element_link(convert, display);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    GValue trigger_val = {};
    g_value_init(&trigger_val, G_TYPE_BOOLEAN);
    g_value_set_boolean(&trigger_val, TRUE);

    tcam_prop_set_tcam_property(TCAM_PROP(source), "Trigger Mode", &trigger_val);

    printf("Press 'q' then 'enter' to stop the stream.\n");
    printf("Press 'Enter' to trigger a new image.\n");
    while(0 == 0)
    {
        char c = getchar();

        if (c == 'q')
        {
            break;
        }

        gboolean r = tcam_prop_set_tcam_property(TCAM_PROP(source), "Software Trigger", &trigger_val);
        if (!r)
        {
            printf("Could not trigger.\n");
        }
    }

    /* deactivate trigger mode */
    /* this is simply to prevent confusion when the camera ist started without wanting to trigger */
    g_value_set_boolean(&trigger_val, FALSE);
    tcam_prop_set_tcam_property(TCAM_PROP(source), "Trigger Mode", &trigger_val);

    // this stops the pipeline and frees all resources
    gst_element_set_state(pipeline, GST_STATE_NULL);

    /* the pipeline automatically handles all elements that have been added to it.
       thus they do not have to be cleaned up manually */
    gst_object_unref(pipeline);

    return 0;
}
