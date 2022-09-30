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

#include "tcam-property-1.0.h" /* gobject introspection interface */

#include <gst/gst.h>
#include <stdio.h> /* printf and putchar */


void print_enum_property(GstElement* source, const char* name)
{
    /* this is only a sample not all properties will be set here */

    GError* err = NULL;
    TcamPropertyBase* property_base = tcam_property_provider_get_tcam_property(TCAM_PROPERTY_PROVIDER(source),
                                                                               name,
                                                                               &err);

    if (err)
    {
        printf("Error while retrieving property: %s\n", err->message);
        g_error_free(err);
        err = NULL;
    }

    if (tcam_property_base_get_property_type(property_base) != TCAM_PROPERTY_TYPE_ENUMERATION)
    {
        printf("%s has wrong type. This should not happen.\n", name);
    }
    else
    {
        TcamPropertyEnumeration* property_enum = TCAM_PROPERTY_ENUMERATION(property_base);
        const char* value = tcam_property_enumeration_get_value(property_enum, &err);

        if (err)
        {
            printf("Error while retrieving property: %s\n", err->message);
            g_error_free(err);
            err = NULL;
        }
        else
        {
            printf("%s: %s\n", name, value);
        }
    }
    g_object_unref(property_base);
}


void set_enum_property(GstElement* source, const char* name, const char* value)
{
    GError* err = NULL;
    TcamPropertyBase* property_base = tcam_property_provider_get_tcam_property(TCAM_PROPERTY_PROVIDER(source),
                                                                               name,
                                                                               &err);

    if (err)
    {
        printf("Error while retrieving property: %s\n", err->message);
        g_error_free(err);
        err = NULL;
    }

    if (tcam_property_base_get_property_type(property_base) != TCAM_PROPERTY_TYPE_ENUMERATION)
    {
        printf("ExposureAuto has wrong type. This should not happen.\n");
    }
    else
    {
        TcamPropertyEnumeration* enum_property = TCAM_PROPERTY_ENUMERATION(property_base);

        tcam_property_enumeration_set_value(enum_property, value, &err);

        if (err)
        {
            printf("Error while setting property: %s\n", err->message);
            g_error_free(err);
            err = NULL;
        }
        else
        {
            printf("Set %s to %s\n", name, value);
        }
    }
    g_object_unref(property_base);
}



int main(int argc, char* argv[])
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
        err = NULL;
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

    gst_element_set_state(pipeline, GST_STATE_READY);

    /* Device is now in a state for interactions */

    /*
      We print the properties for a before/after comparison,
     */
    printf("Values before we change them:\n\n");

    print_enum_property(source, "ExposureAuto");
    print_enum_property(source, "GainAuto");

    /*
      We set the properties to other values
     */
    printf("\nChanging:\n\n");

    set_enum_property(source, "ExposureAuto", "Off");
    set_enum_property(source, "GainAuto", "Off");

    /* alternatively you can get/set directly on the TCAM_PROPERTY_PROVIDER */
    /* for this you need to know the type of the property you want to get/set */

    /* tcam_property_provider_set_tcam_enumeration(TCAM_PROPERTY_PROVIDER(source), "ExposureAuto", "Off", &err); */
    /* tcam_property_provider_set_tcam_integer(TCAM_PROPERTY_PROVIDER(source), "Brightness", 200, &err); */
    /* tcam_property_provider_set_tcam_float(TCAM_PROPERTY_PROVIDER(source), "ExposureTime", 30000.0, &err); */

    printf("\nValues after we changed them:\n\n");

    /*
      second print for the before/after comparison
     */
    print_enum_property(source, "ExposureAuto");
    print_enum_property(source, "GainAuto");

    /* cleanup, reset state */
    gst_element_set_state(pipeline, GST_STATE_NULL);

    gst_object_unref(source);
    gst_object_unref(pipeline);

    return 0;
}
