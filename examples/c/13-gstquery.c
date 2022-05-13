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

/*
  This example will show you how to query the tcamsrc
  - to verify potential framerates
  - to verify potential caps
 */

#include <gst/gst.h>
#include <stdio.h> /* printf and putchar */


int main(int argc, char* argv[])
{
    /* this line sets the gstreamer default logging level
       it can be removed in normal applications
       gstreamer logging can contain verry useful information
       when debugging your application
       see https://gstreamer.freedesktop.org/documentation/tutorials/basic/debugging-tools.html
       for further details
    */
    gst_debug_set_default_threshold(GST_LEVEL_WARNING);

    gst_init(&argc, &argv); // init gstreamer

    GError* err = NULL;
    const char* pipeline_desc = "tcambin name=bin ! videoconvert ! ximagesink";

    GstElement* pipeline = gst_parse_launch(pipeline_desc, &err);

    /* test for error */
    if (pipeline == NULL)
    {
        printf("Could not create pipeline. Cause: %s\n", err->message);
        return 1;
    }
    GstElement* tcambin = gst_bin_get_by_name(GST_BIN(pipeline), "bin");

    gst_element_set_state(pipeline, GST_STATE_READY);


    /* retrieve the source
       these queries have to be performed on the source
       as the tcambin might alter the query
       which we do not want
    */
    GstElement* source = gst_bin_get_by_name(GST_BIN(tcambin), "tcambin-source");

    GstCaps* framerate_query_caps =
        gst_caps_from_string("video/x-bayer,format=rggb,width=640,height=480");

    // if you have a mono camera try these instead
    // GstCaps* framerate_query_caps =
    //        gst_caps_from_string("video/x-raw,format=GRAY8,width=640,height=480");

    GstQuery* framerate_query = gst_query_new_caps(framerate_query_caps);

    gst_caps_unref(framerate_query_caps);

    if (gst_element_query(source, framerate_query))
    {
        // no transfer, nothing has to be freed
        GstCaps* query_result_caps = NULL;
        gst_query_parse_caps_result(framerate_query, &query_result_caps);

        char* result_caps_string = gst_caps_to_string(query_result_caps);
        printf("Camera supports these framerates: %s\n", result_caps_string);
        g_free(result_caps_string);
    }

    gst_query_unref(framerate_query);

    /*
      verify specific caps
      for this all needed fields have to be set(format, width, height and framerate)
    */

    GstCaps* accept_query_caps =
        gst_caps_from_string("video/x-bayer,format=rggb,width=640,height=480,framerate=30/1");

    GstQuery* accept_query = gst_query_new_accept_caps(accept_query_caps);

    gst_caps_unref(accept_query_caps);

    if (gst_element_query(source, accept_query))
    {
        gboolean accepts_caps;
        gst_query_parse_accept_caps_result(accept_query, &accepts_caps);

        if (accepts_caps)
        {
            printf("Caps are accepted\n");
        }
        else
        {
            printf("Caps are not accepted\n");
        }
    }

    gst_query_unref(accept_query);

    gst_object_unref(source);
    gst_object_unref(tcambin);

    gst_element_set_state(pipeline, GST_STATE_NULL);

    gst_object_unref(pipeline);

    return 0;
}
