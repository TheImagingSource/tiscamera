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

/* This example will show you how to list information about the available devices */

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

    GError* err = NULL;

    GstElement* sample_pipeline = gst_parse_launch("tcambin name=source ! fakesink", &err);

    if (sample_pipeline == NULL)
    {
        printf("Unable to create pipeline: %s\n", err->message);
        g_free(err);
        return 1;
    }

    /* create a tcambin to retrieve device information */
    GstElement* source = gst_bin_get_by_name(GST_BIN(sample_pipeline), "source");

    /* retrieve a single linked list of serials of the available devices */
    GSList* serials = tcam_prop_get_device_serials(TCAM_PROP(source));

    for (GSList* elem = serials; elem; elem = elem->next)
    {
        const char* device_serial = (gchar*)elem->data;

        char* name;
        char* identifier;
        char* connection_type;

        /* This fills the parameters to the likes of:
           name='DFK Z12GP031',
           identifier='The Imaging Source Europe GmbH-11410533'
           connection_type='aravis'
           The identifier is the name given by the backend
           The connection_type identifies the backend that is used.
                   Currently 'aravis', 'v4l2', libusb, tegra, pimipi and 'unknown' exist
        */
        gboolean ret = tcam_prop_get_device_info(TCAM_PROP(source),
                                                 device_serial,
                                                 &name,
                                                 &identifier,
                                                 &connection_type);

        if (ret) // get_device_info was successful
        {
            printf("Model: %s Serial: %s Type: %s\n",
                   name, (gchar*)elem->data, connection_type);

            g_free( name );
            g_free( identifier );
            g_free( connection_type );
        }
    }

    g_slist_free_full(serials, g_free);
    gst_object_unref(source);
    gst_object_unref(sample_pipeline);

    return 0;
}
