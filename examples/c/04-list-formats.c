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

/* This example will show you how to list the formats your device offers */

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

    // set this if you do not want the first found device
    char* serial = NULL;

    /* create a tcambin to retrieve device information */
    GstElement* source = gst_element_factory_make("tcambin", "source");

    if (serial != NULL)
    {
        GValue val = {};
        g_value_init(&val, G_TYPE_STRING);
        g_value_set_static_string(&val, serial);

        g_object_set_property(G_OBJECT(source), "serial", &val);
    }

    /* Setting the state to ready ensures that all resources
       are initialized and that we really get all format capabilities */
    gst_element_set_state(source, GST_STATE_READY);

    GstPad* pad = gst_element_get_static_pad(source, "src");

    GstCaps* caps = gst_pad_query_caps(pad, NULL);

    for (unsigned int i = 0; i < gst_caps_get_size(caps); ++i)
    {
        GstStructure* structure = gst_caps_get_structure(caps, i);

        /*
          for a simple display the following line can be used
          printf("%s\n", gst_structure_to_string(structure));
        */

        const char* name = gst_structure_get_name(structure);

        // this is only required when dealing
        // with FPD/MiPi cameras on tegra systems
        // must not be freed
        GstCapsFeatures* features = gst_caps_get_features(caps, i);

        if (features)
        {
            if (gst_caps_features_contains(features, "memory:NVMM"))
            {
                // do something with this information
                printf("NVMM ");
            }
        }

        if (gst_structure_get_field_type(structure, "format") == G_TYPE_STRING)
        {
            const char* format = gst_structure_get_string(structure, "format");

            printf("%s %s - ", name, format);

        }
        else if (gst_structure_get_field_type(structure, "format") == GST_TYPE_LIST)
        {
            printf("%s { ", name);

            const GValue* val = gst_structure_get_value(structure, "format");

            for (unsigned int x = 0; x < gst_value_list_get_size(val); ++x)
            {
                const GValue* format = gst_value_list_get_value(val, x);

                printf("%s ", g_value_get_string(format));
            }


            printf("} - ");

        }
        else
        {
            printf("format handling not implemented for unexpected type: %s\n",
                   G_VALUE_TYPE_NAME(gst_structure_get_field_type(structure,
                                                                  "format")));
            continue;
        }

        GType width_type = gst_structure_get_field_type(structure, "width");

        if (width_type == GST_TYPE_INT_RANGE)
        {
            int width_min = gst_value_get_int_range_min(gst_structure_get_value(structure,
                                                                                "width"));
            int width_max = gst_value_get_int_range_max(gst_structure_get_value(structure,
                                                                                "width"));


            printf("width: [%d-%d]", width_min, width_max);
        }
        else
        {
            int width;
            gboolean ret = gst_structure_get_int(structure, "width", &width);

            if (!ret)
            {
                printf("Unable to query width\n");
                continue;
            }

            printf("%d", width);
        }

        printf(" X ");

        GType height_type = gst_structure_get_field_type(structure, "height");

        if (height_type == GST_TYPE_INT_RANGE)
        {
            int height_min = gst_value_get_int_range_min(gst_structure_get_value(structure,
                                                                                 "height"));
            int height_max = gst_value_get_int_range_max(gst_structure_get_value(structure,
                                                                                 "height"));


            printf("height: [%d-%d]", height_min, height_max);
        }
        else
        {
            int height;
            gboolean ret = gst_structure_get_int(structure, "height", &height);

            if (!ret)
            {
                printf("Unable to query height\n");
                continue;
            }

            printf("%d", height);
        }

        printf(" - ");

        const GValue* framerate = gst_structure_get_value(structure, "framerate");

        if (G_VALUE_TYPE(framerate) == GST_TYPE_LIST)
        {
            for (unsigned int x = 0; x < gst_value_list_get_size(framerate); ++x)
            {
                const GValue* val  = gst_value_list_get_value(framerate, x);

                if (G_VALUE_TYPE(val) == GST_TYPE_FRACTION)
                {
                    int num = gst_value_get_fraction_numerator(val);
                    int den = gst_value_get_fraction_denominator(val);

                    printf("%d/%d ", num, den);
                }
                else
                {
                    printf("Handling of framerate handling not implemented for non fraction types.\n");
                    break;
                }
            }
        }
        else if (G_VALUE_TYPE(framerate) == GST_TYPE_FRACTION_RANGE)
        {
            const GValue* framerate_min = gst_value_get_fraction_range_min(framerate);
            const GValue* framerate_max = gst_value_get_fraction_range_max(framerate);

            printf("%d/%d - %d/%d",
                   gst_value_get_fraction_numerator(framerate_min),
                   gst_value_get_fraction_denominator(framerate_min),
                   gst_value_get_fraction_numerator(framerate_max),
                   gst_value_get_fraction_denominator(framerate_max));
        }
        else
        {
            printf("Unable to interpret framerate type\n");
            continue;
        }

        // we printed all information for a format decription
        // print a new line to keep everything user readable
        printf("\n");
    }

    gst_caps_unref(caps);
    gst_object_unref(pad);

    gst_element_set_state(source, GST_STATE_NULL);

    gst_object_unref(source);

    return 0;
}
