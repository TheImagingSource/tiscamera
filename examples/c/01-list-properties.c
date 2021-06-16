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

/* This example will show you how to list the available properties */

#include <gst/gst.h>

#include <stdio.h> /* printf and putchar */
#include <string.h>
#include <unistd.h>
#include "tcamprop.h" /* gobject introspection interface */


void list_properties (GstElement* source)
{

    GSList* names = tcam_prop_get_tcam_property_names(TCAM_PROP(source));

    for ( GSList* cur = names; cur != NULL; cur = cur->next )
    {
        const char* name = (char*)cur->data;

        GValue value = {};
        GValue min = {};
        GValue max = {};
        GValue default_value = {};
        GValue step_size = {};
        GValue type = {};
        GValue flags = {};
        GValue category = {};
        GValue group = {};

        gboolean ret = tcam_prop_get_tcam_property(TCAM_PROP(source),
                                                   name,
                                                   &value,
                                                   &min,
                                                   &max,
                                                   &default_value,
                                                   &step_size,
                                                   &type,
                                                   &flags,
                                                   &category,
                                                   &group);

        if (!ret)
        {
            printf("Could not query property '%s'\n", name);
            continue;
        }

        const char* t = g_value_get_string(&type);
        if (strcmp(t, "integer") == 0)
        {
            printf("%s(integer) min: %d max: %d step: %d value: %d default: %d  grouping %s %s\n",
                   name,
                   g_value_get_int(&min), g_value_get_int(&max),
                   g_value_get_int(&step_size),
                   g_value_get_int(&value), g_value_get_int(&default_value),
                   g_value_get_string(&category), g_value_get_string(&group));
        }
        else if (strcmp(t, "double") == 0)
        {
            printf("%s(double) min: %f max: %f step: %f value: %f default: %f  grouping %s %s\n",
                   name,
                   g_value_get_double(&min), g_value_get_double(&max),
                   g_value_get_double(&step_size),
                   g_value_get_double(&value), g_value_get_double(&default_value),
                   g_value_get_string(&category), g_value_get_string(&group));
        }
        else if (strcmp(t, "string") == 0)
        {
            printf("%s(string) value: %s default: %s  grouping %s %s\n",
                   name,
                   g_value_get_string(&value), g_value_get_string(&default_value),
                   g_value_get_string(&category), g_value_get_string(&group));
        }
        else if (strcmp(t, "enum") == 0)
        {
            GSList* entries = tcam_prop_get_tcam_menu_entries(TCAM_PROP(source), name);

            if (entries == NULL)
            {
                printf("%s returned no enumeration values.\n", name);
                continue;
            }

            printf("%s(enum) value: %s default: %s  grouping %s %s\n",
                   name,
                   g_value_get_string(&value), g_value_get_string(&default_value),
                   g_value_get_string(&category), g_value_get_string(&group));
            printf("Entries: \n");
            for (unsigned int x = 0; x < g_slist_length(entries); ++x)
            {
                printf("\t %s\n", (char*) g_slist_nth(entries, x)->data);
            }

            g_slist_free_full( entries, g_free );
        }
        else if (strcmp(t, "boolean") == 0)
        {
            printf("%s(boolean) value: %s default: %s  grouping %s %s\n",
                   name,
                   g_value_get_boolean(&value) ? "true" : "false", g_value_get_boolean(&default_value) ? "true" : "false",
                   g_value_get_string(&category), g_value_get_string(&group));
        }
        else if (strcmp(t, "button") == 0)
        {
            printf("%s(button) grouping %s %s\n", name, g_value_get_string(&category), g_value_get_string(&group));
        }
        else
        {
            printf("Property '%s' has type '%s' .\n", name, t);
        }

        g_value_unset( &value );
        g_value_unset( &min );
        g_value_unset( &max );
        g_value_unset( &default_value );
        g_value_unset( &step_size );
        g_value_unset( &type );
        g_value_unset( &flags );
        g_value_unset( &category );
        g_value_unset( &group );
    }

    g_slist_free_full(names,g_free);
}


gboolean block_until_playing (GstElement* pipeline)
{
    while (TRUE)
    {
        GstState state;
        GstState pending;

        // wait 0.1 seconds for something to happen
        GstStateChangeReturn ret = gst_element_get_state(pipeline ,&state, &pending, 100000000);

        if (ret == GST_STATE_CHANGE_SUCCESS)
        {
            return TRUE;
        }
        else if (ret == GST_STATE_CHANGE_FAILURE)
        {
            printf("Failed to change state %s %s %s\n",
                   gst_element_state_change_return_get_name(ret),
                   gst_element_state_get_name(state),
                   gst_element_state_get_name(pending));

            return FALSE;
        }
    }
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

    GError* err = NULL;

    // this is a placeholder definition
    // normally your pipeline would be defined here
    GstElement* pipeline = gst_parse_launch("tcambin name=source ! fakesink", &err);

    if (pipeline == NULL)
    {
        printf("Unable to create pipeline: %s\n", err->message);
        g_free(err);
        return 1;
    }

    /* get the tcambin to retrieve device information */
    GstElement* source = gst_bin_get_by_name(GST_BIN(pipeline), "source");

    const char* serial = NULL; // set this if you do not want the first found device

    if (serial != NULL)
    {
        GValue val = {};
        g_value_init(&val, G_TYPE_STRING);
        g_value_set_static_string(&val, serial);

        g_object_set_property(G_OBJECT(source), "serial", &val);
    }

    printf("Properties before state PLAYING:\n");
    list_properties(source);

    // in the READY state the camera will always be initialized
    // in the PLAYING state additional properties may appear from gstreamer elements
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // helper function to ensure we have the right state
    // alternatively wait for the first image
    if (!block_until_playing(pipeline))
    {
        printf("Unable to start pipeline. \n");
    }

    printf("Properties during state PLAYING:\n");
    list_properties(source);

    gst_element_set_state(pipeline, GST_STATE_NULL);

    gst_object_unref(source);
    gst_object_unref(pipeline);

    return 0;
}
