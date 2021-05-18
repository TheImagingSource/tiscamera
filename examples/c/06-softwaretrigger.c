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

#include <unistd.h> /* sleep  */
#include <stdio.h>  /* printf */
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

    GstElement* pipeline = gst_parse_launch("tcambin name=source ! videoconvert ! ximagesink", &err);

    /* test for error */
    if (pipeline == NULL)
    {
        printf("Could not create pipeline. Cause: %s\n", err->message);
        return 1;
    }

    GstElement* source = gst_bin_get_by_name(GST_BIN(pipeline), "source");

    if (serial != NULL)
    {
        GValue val = {};
        g_value_init(&val, G_TYPE_STRING);
        g_value_set_static_string(&val, serial);

        g_object_set_property(G_OBJECT(source), "serial", &val);

        g_value_unset(&val);
    }

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    /*
      This sleep exists only to ensure
      that a live image exists before trigger mode is activated.
      for all other purposes this can be removed.
     */
    sleep(2);

    GValue trigger_val = {};
    g_value_init(&trigger_val, G_TYPE_BOOLEAN);
    g_value_set_boolean(&trigger_val, TRUE);

    // Check for the type of trigger
    // Depending on the camera model trigger may be a bool or an enum
    const gchar* trigger_mode_type =  tcam_prop_get_tcam_property_type(TCAM_PROP(source),
                                                                       "Trigger Mode");

    if (!trigger_mode_type)
    {
        printf("Unable to determine type of 'Trigger Mode'\n");
        exit(1);
    }

    gboolean trigger_is_bool = TRUE;
    GValue trigger = {};

    if (strcmp(trigger_mode_type, "enum") == 0)
    {
        trigger_is_bool = FALSE;
        g_value_init(&trigger, G_TYPE_STRING);
        g_value_set_string(&trigger, "On");
        tcam_prop_set_tcam_property(TCAM_PROP(source), "Trigger Mode", &trigger);
    }
    else
    {
        g_value_init(&trigger, G_TYPE_BOOLEAN);
        g_value_set_boolean(&trigger, TRUE);

        tcam_prop_set_tcam_property(TCAM_PROP(source), "Trigger Mode", &trigger);
    }

    while (0 == 0)
    {
        printf("Press 'q' then 'enter' to stop the stream.\n");
        printf("Press 'Enter' to trigger a new image.\n");

        char c = getchar();

        if (c == 'q')
        {
            break;
        }

        gboolean r = tcam_prop_set_tcam_property(TCAM_PROP(source), "Software Trigger", &trigger_val);
        if (!r)
        {
            printf("!!! Could not trigger. !!!\n");
        }
        else
        {
            printf("=== Triggered image. ===\n");
        }
    }

    /* deactivate trigger mode */
    /* this is simply to prevent confusion when the camera ist started without wanting to trigger */

    if (trigger_is_bool)
    {
        g_value_set_boolean(&trigger, FALSE);
        tcam_prop_set_tcam_property(TCAM_PROP(source), "Trigger Mode", &trigger);
    }
    else
    {
        g_value_set_string(&trigger, "Off");
        tcam_prop_set_tcam_property(TCAM_PROP(source), "Trigger Mode", &trigger);
    }

    g_value_unset(&trigger);
    g_value_unset(&trigger_val);

    // this stops the pipeline and frees all resources
    gst_element_set_state(pipeline, GST_STATE_NULL);

    gst_object_unref(source);
    /* the pipeline automatically handles all elements that have been added to it.
       thus they do not have to be cleaned up manually */
    gst_object_unref(pipeline);

    return 0;
}
