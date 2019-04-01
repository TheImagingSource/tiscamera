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

void print_properties (GstElement* source)
{

    GValue exp_auto_value = G_VALUE_INIT;

    /* We are only interested in the value, this  */
    gboolean ret = tcam_prop_get_tcam_property(TCAM_PROP(source),
                                               "Exposure Auto",
                                               &exp_auto_value,
                                               NULL, NULL, NULL, NULL,
                                               NULL, NULL, NULL, NULL);

    if (ret)
    {
        printf("Exposure Auto has value: %s\n",
               g_value_get_boolean(&exp_auto_value) ? "true" : "false");
    }
    else
    {
        printf("Could not query Exposure Auto\n");
    }
    GValue gain_auto_value = G_VALUE_INIT;

    ret = tcam_prop_get_tcam_property(TCAM_PROP(source),
                                      "Gain Auto",
                                      &gain_auto_value,
                                      NULL, NULL, NULL, NULL,
                                      NULL, NULL, NULL, NULL);
    if (ret)
    {
        printf("Gain Auto has value: %s\n",
               g_value_get_boolean(&gain_auto_value) ? "true" : "false");
    }
    else
    {
        printf("Could not query Gain Auto\n");
    }

    GValue exp_value = G_VALUE_INIT;

    ret = tcam_prop_get_tcam_property(TCAM_PROP(source),
                                      "Exposure",
                                      &exp_value,
                                      NULL, NULL, NULL, NULL,
                                      NULL, NULL, NULL, NULL);

    if (ret)
    {
        printf("Exposure has value: %d\n",
               g_value_get_int(&exp_value));
    }
    else
    {
        printf("Could not query Exposure\n");
    }
}


int main (int argc, char *argv[])
{
    gst_init(&argc, &argv); // init gstreamer

    /* create a tcambin to retrieve device information */
    GstElement* source = gst_element_factory_make("tcambin", "source");

    const char* serial = NULL;

    if (serial != NULL)
    {
        GValue val = {};
        g_value_init(&val, G_TYPE_STRING);
        g_value_set_static_string(&val, serial);

        g_object_set_property(G_OBJECT(source), "serial", &val);
    }

    /* in the READY state the camera will always be initialized */
    gst_element_set_state(source, GST_STATE_READY);

    /* Device is now in a state for interactions */

    /*
      We print the properties for a before/after comparison,
     */
    print_properties(source);

    /*
      We set the properties to other values
     */

    GValue set_auto = G_VALUE_INIT;
    g_value_init(&set_auto, G_TYPE_BOOLEAN);

    g_value_set_boolean(&set_auto, FALSE);

    tcam_prop_set_tcam_property(TCAM_PROP(source),
                                "Exposure Auto", &set_auto);
    /* reuse set_auto. Auto Exposure and Auto Gain have the same type */
    tcam_prop_set_tcam_property(TCAM_PROP(source),
                                "Gain Auto", &set_auto);

    GValue set_exposure = G_VALUE_INIT;
    g_value_init(&set_exposure, G_TYPE_INT);

    g_value_set_int(&set_exposure, 3000);

    tcam_prop_set_tcam_property(TCAM_PROP(source),
                                "Exposure", &set_exposure);

    /*
      second print for the before/after comparison
     */
    print_properties(source);

    /* cleanup, reset state */
    gst_element_set_state(source, GST_STATE_NULL);

    gst_object_unref(source);

    return 0;
}
