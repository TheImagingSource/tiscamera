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
#include <gst/video/video.h>
#include <unistd.h>
#include <stdlib.h>
#include <tcamprop.h>

GMainLoop *mainloop = NULL;

/**
 * This callback reads chars from stdin and terminates the mainloop when it receives a 'q'
 */
static gboolean
stdin_callback (GIOChannel * io, GIOCondition condition, gpointer data)
{
    GError *error = NULL;
    gchar in;

    if (g_io_channel_read_chars (io, &in, 1, NULL, &error) ==  G_IO_STATUS_NORMAL)
    {
        if ('q' == in)
        {
            g_main_loop_quit (mainloop);
            return FALSE;
        }
    }

    return TRUE;
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
        static guint framecount = 0;
        int pixel_data = -1;

        framecount++;

        GstBuffer* buffer = gst_sample_get_buffer(sample);
        GstMapInfo info; // contains the actual image
        if (gst_buffer_map(buffer, &info, GST_MAP_READ))
        {
            GstVideoInfo *video_info = gst_video_info_new();
            if (!gst_video_info_from_caps (video_info, gst_sample_get_caps(sample)))
            {
                // Could not parse video info (should not happen)
                g_warning("Failed to parse video info");
                return GST_FLOW_ERROR;
            }

            /* Get a pointer to the image data */
            unsigned char* data = info.data;

            /* Get the pixel value of the center pixel */
            int stride = video_info->finfo->bits / 8;
            pixel_data = info.data[video_info->width/2*stride + video_info->width * video_info->height/2 * stride];

            gst_buffer_unmap(buffer, &info);
            gst_video_info_free(video_info);
        }
        GstClockTime timestamp = GST_BUFFER_PTS(buffer);
        g_print("Captured frame %d, Pixel Value=%03d Timestamp=%" GST_TIME_FORMAT "            \r",
                framecount, pixel_data,
                GST_TIME_ARGS(timestamp));


        // delete our reference so that gstreamer can handle the sample
        gst_sample_unref (sample);
    }
    return GST_FLOW_OK;
}

static gchar **_args = NULL;

static GOptionEntry option_entries[] =
{
    {G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY, &_args, "Serial Number", "SERIAL-NUMBER"},
    NULL
};

int main (int argc, char *argv[])
{
    GError *error = NULL;
    GOptionContext *context;

    GOptionGroup *gst_options = gst_init_get_option_group(); // init gstreamer

    context = g_option_context_new("- tiscamera image buffer example");
    g_option_context_add_main_entries (context, option_entries, NULL);
    g_option_context_add_group (context, gst_options);
    if (!g_option_context_parse (context, &argc, &argv, &error))
    {
        g_print ("Failed parsing options: %s\n", error->message);
        exit(1);
    }

    mainloop = g_main_loop_new(NULL, FALSE);

    GstElement* source = gst_element_factory_make("tcambin", "source");

    if (_args != NULL)
    {
        g_object_set(G_OBJECT(source), "serial", _args[0]);
    }

    // Get the caps from the source and use the first one in the list
    // for the capsfilter.
    // This sets the first video format supported by the device as
    // the target format for the appsink
    gst_element_set_state(source, GST_STATE_READY);
    GstPad* pad = gst_element_get_static_pad(source, "src");
    GstCaps* caps = gst_pad_query_caps(pad, NULL);
    if (caps == NULL)
    {
        g_error("Failed to query caps from device!");
    }
    GstStructure *structure = gst_caps_get_structure(caps, 0);
    gint width, height;
    gst_structure_get_int(structure, "width", &width);
    gst_structure_get_int(structure, "height", &height);
    GstCaps* formatcaps = gst_caps_new_simple("video/x-raw",
                        "format", G_TYPE_STRING, gst_structure_get_string(structure, "format"),
                        "width", G_TYPE_INT, width,
                        "height", G_TYPE_INT, height,
                        NULL);
    g_print("Using video format '%s' for appsink.\n", gst_caps_to_string(formatcaps));

    //gst_element_set_state(source, GST_STATE_NULL);

    GstElement* pipeline = gst_pipeline_new("pipeline");
    GstElement* capsfilter = gst_element_factory_make("capsfilter", "caps");
    GstElement* sink = gst_element_factory_make("appsink", "sink");

    g_object_set(G_OBJECT(capsfilter), "caps", formatcaps, NULL);

    // tell appsink to notify us when it receives an image
    g_object_set(G_OBJECT(sink), "emit-signals", TRUE, NULL);

    g_signal_connect(sink, "new-sample", G_CALLBACK(callback), NULL);

    gst_bin_add(GST_BIN(pipeline), source);
    gst_bin_add(GST_BIN(pipeline), capsfilter);
    gst_bin_add(GST_BIN(pipeline), sink);

    gst_element_link_many(source, capsfilter, sink, NULL);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // Create an IO Channel for the standard input to register a key press callback
    GIOChannel *io = NULL;
    guint io_watch_id = 0;
    /* standard input callback */
    io = g_io_channel_unix_new (STDIN_FILENO);
    io_watch_id = g_io_add_watch (io, G_IO_IN, stdin_callback, NULL);
    g_io_channel_unref (io);

    g_print("Press 'q' then 'enter' to stop the stream.\n");
    g_main_loop_run(mainloop);

    // this stops the pipeline and frees all resources
    gst_element_set_state(pipeline, GST_STATE_NULL);

    /* the pipeline automatically handles all elements that have been added to it.
       thus they do not have to be cleaned up manually */
    gst_object_unref(pipeline);

    return 0;
}
