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
    gst_init(&argc, &argv); // init gstreamer

    /* create a tcambin to retrieve device information */
    GstElement* source = gst_element_factory_make("tcambin", "source");

    /* retrieve a single linked list of serials of the available devices */
    GSList* serials = tcam_prop_get_device_serials(TCAM_PROP(source));

    for (GSList* elem = serials; elem; elem = elem->next)
    {
        char* name;
        char* identifier;
        char* connection_type;

        /* This fills the parameters to the likes of:
           name='DFK Z12GP031',
           identifier='The Imaging Source Europe GmbH-11410533'
           connection_type='aravis'
           The identifier is the name given by the backend
           The connection_type identifies the backend that is used.
                   Currently 'aravis', 'v4l2' and 'unknown' exist
        */
        gboolean ret = tcam_prop_get_device_info(TCAM_PROP(source),
                                                 (gchar*) elem->data,
                                                 &name,
                                                 &identifier,
                                                 &connection_type);

        if (ret) // get_device_info was successfull
        {
            printf("Model: %s Serial: %s Type: %s\n", name, (gchar*)elem->data, connection_type);
        }
    }

    gst_object_unref(source);

    return 0;
}
