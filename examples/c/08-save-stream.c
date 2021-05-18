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

    const char* serial = NULL; // the serial number of the camera we want to use

    GError* err = NULL;


    GstElement* pipeline = gst_parse_launch("tcambin name=bin"
                                            " ! video/x-raw,format=BGRx,width=640,height=480,framerate=30/1"
                                            " ! tee name=t"
                                            " ! queue"
                                            " ! videoconvert"
                                            " ! ximagesink"
                                            " t."
                                            " ! queue"
                                            " ! videoconvert"
                                            " ! avimux"
                                            " ! filesink name=fsink", &err);
    /*
      to save a video without live view reduce the pipeline to the following:

      GstElement* pipeline = Gst.parse_launch("tcambin name=bin"
                                              " ! video/x-raw,format=BGRx,width=640,height=480,framerate=30/1"
                                              " ! videoconvert"
                                              " ! avimux"
                                              " ! filesink name=fsink", &err);

    */

    if (serial != NULL)
    {
        GstElement* source = gst_bin_get_by_name(GST_BIN(pipeline), "bin");
        GValue val = {};
        g_value_init(&val, G_TYPE_STRING);
        g_value_set_static_string(&val, serial);

        g_object_set_property(G_OBJECT(source), "serial", &val);
        gst_object_unref(source);
    }

    const char* file_location = "/tmp/tiscamera-save-stream.avi";

    GstElement* fsink = gst_bin_get_by_name(GST_BIN(pipeline), "fsink");

    g_object_set(G_OBJECT(fsink), "location", file_location, NULL);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    printf("Press Enter to stop the recording\n");
    getchar();

    // this stops the pipeline and frees all resources
    gst_element_set_state(pipeline, GST_STATE_NULL);

    gst_object_unref( fsink );

    /* the pipeline automatically handles all elements that have been added to it.
       thus they do not have to be cleaned up manually */
    gst_object_unref(pipeline);

    return 0;
}
