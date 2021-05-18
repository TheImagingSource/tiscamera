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

/* This example will show you how to set properties for a certain camera */

#include <gst/gst.h>

#include <stdio.h> /* printf and putchar */
#include "tcamprop.h" /* gobject introspection interface */


void print_enum_or_bool(GstElement* source, const char* name)
{
    GValue value = {};

    GValue type = {};
    /* We are only interested in the value, this  */
    gboolean ret = tcam_prop_get_tcam_property(TCAM_PROP(source),
                                               name,
                                               &value,
                                               NULL, NULL, NULL, NULL,
                                               &type, NULL, NULL, NULL);

    if (!ret)
    {
        printf("Could not query %s\n", name);
        return;
    }

    const char* t = g_value_get_string(&type);

    // exposure auto is a bool
    if (strcmp(t, "boolean") == 0)
    {
        printf("%s has value: %s\n", name,
               g_value_get_boolean(&value) ? "true" : "false");

    }
    else if (strcmp(t, "enum") == 0)
    {
        printf("%s has value: %s\n", name, g_value_get_string(&value));
    }
    g_value_unset(&value);
    g_value_unset(&type);
}


void print_properties (GstElement* source)
{

    print_enum_or_bool(source, "Exposure Auto");
    print_enum_or_bool(source, "Gain Auto");

    GValue brightness_value = G_VALUE_INIT;

    gboolean ret = tcam_prop_get_tcam_property(TCAM_PROP(source),
                                               "Brightness",
                                               &brightness_value,
                                               NULL, NULL, NULL, NULL,
                                               NULL, NULL, NULL, NULL);

    if (ret)
    {
        printf("Brightness has value: %d\n",
               g_value_get_int(&brightness_value));
        g_value_unset(&brightness_value);
    }
    else
    {
        printf("Could not query Brightness\n");
    }
}


gboolean set_bool_or_enum(GstElement* source,
                          const char* name,
                          gboolean new_value)
{
    // this function basically exists to ensure the example
    // works with all camera types.
    // If you know the property type of the properties you are
    // setting, you can simply call that
    // instead of checking the type.

    // some settings may exhibit as bool or enum,
    // depending on the camera you use.

    GValue type = {};

    gboolean ret = tcam_prop_get_tcam_property(TCAM_PROP(source),
                                               name,
                                               NULL,
                                               NULL, NULL, NULL, NULL,
                                               &type, NULL, NULL, NULL);

    if (!ret)
    {
        printf("Could not query property %s\n", name);
        return FALSE;
    }

    const char* t = g_value_get_string(&type);

    // exposure auto is a bool
    if (strcmp(t, "boolean") == 0)
    {
        GValue set_auto = G_VALUE_INIT;
        g_value_init(&set_auto, G_TYPE_BOOLEAN);

        g_value_set_boolean(&set_auto, FALSE);

        // actual set
        ret = tcam_prop_set_tcam_property(TCAM_PROP(source),
                                          name, &set_auto);
        g_value_unset( &set_auto );

    }
    else if (strcmp(t, "enum") == 0)
    {
        GValue set_auto = G_VALUE_INIT;
        g_value_init(&set_auto, G_TYPE_STRING);

        if (new_value)
        {
            g_value_set_string(&set_auto, "On");
        }
        else
        {
            g_value_set_string(&set_auto, "Off");
        }

        // actual set
        ret = tcam_prop_set_tcam_property(TCAM_PROP(source),
                                          name, &set_auto);
        g_value_unset( &set_auto );

    }
    g_value_unset( &type );

    return ret;

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
    GstElement* pipeline = gst_parse_launch("tcambin name=source  ! fakesink", &err);

    if (pipeline == NULL)
    {
        printf("Unable to create pipeline: %s\n", err->message);
        g_free(err);
        return 1;
    }

    /* create a tcambin to retrieve device information */
    GstElement* source = gst_bin_get_by_name(GST_BIN(pipeline), "source");

    const char* serial = NULL;

    if (serial != NULL)
    {
        GValue val = {};
        g_value_init(&val, G_TYPE_STRING);
        g_value_set_static_string(&val, serial);

        g_object_set_property(G_OBJECT(source), "serial", &val);
    }

    // helper function to ensure we have the right state
    // alternatively wait for the first image
    // then everything is guaranteed to be initialized
    if (!block_until_playing(pipeline))
    {
        printf("Unable to start pipeline. \n");
    }
    /* Device is now in a state for interactions */

    /*
      We print the properties for a before/after comparison,
     */
    print_properties(source);

    /*
      We set the properties to other values
     */

    set_bool_or_enum(source, "Exposure Auto", FALSE);
    set_bool_or_enum(source, "Gain Auto", FALSE);


    GValue set_brightness = G_VALUE_INIT;
    g_value_init(&set_brightness, G_TYPE_INT);

    g_value_set_int(&set_brightness, 200);

    tcam_prop_set_tcam_property(TCAM_PROP(source),
                                "Brightness", &set_brightness);

    g_value_unset(&set_brightness);

    /*
      second print for the before/after comparison
     */
    print_properties(source);

    /* cleanup, reset state */
    gst_element_set_state(source, GST_STATE_NULL);

    gst_object_unref(source);

    return 0;
}
