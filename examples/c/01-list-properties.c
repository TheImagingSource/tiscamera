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

#include "tcam-property-1.0.h" /* gobject introspection interface */

#include <gst/gst.h>
#include <stdio.h> /* printf and putchar */
#include <string.h>
#include <unistd.h>

void print_flags(TcamPropertyBase* prop)
{
    printf("Available: ");
    GError* err = NULL;
    gboolean av = tcam_property_base_is_available(prop, &err);
    if (av)
    {
        printf("yes");
    }
    else
    {
        printf("no");
    }

    printf("\tLocked: ");

    gboolean lo = tcam_property_base_is_locked(prop, &err);

    if (lo)
    {
        printf("yes");
    }
    else
    {
        printf("no");
    }
}


void list_properties(GstElement* source)
{

    GError* err = NULL;
    GSList* names =  tcam_property_provider_get_tcam_property_names(TCAM_PROPERTY_PROVIDER(source), &err);

    for (unsigned int i = 0; i < g_slist_length(names); ++i)
    {
        char* name = (char*)g_slist_nth(names, i)->data;

        TcamPropertyBase* base_property = tcam_property_provider_get_tcam_property(TCAM_PROPERTY_PROVIDER(source), name, &err);

        TcamPropertyType type = tcam_property_base_get_property_type(base_property);

        switch(type)
        {
            case TCAM_PROPERTY_TYPE_INTEGER:
            {
                TcamPropertyInteger* integer = TCAM_PROPERTY_INTEGER(base_property);

                gint64 def = tcam_property_integer_get_default(integer, &err);

                gint64 min;
                gint64 max;
                gint64 step;
                tcam_property_integer_get_range(integer, &min, &max, &step, &err);

                if (err)
                {
                    printf("%s\n", err->message);
                    g_error_free(err);
                    err = NULL;
                    break;
                }

                gint64 value = tcam_property_integer_get_value(integer, &err);

                if (err)
                {
                    printf("%s\n", err->message);
                    g_error_free(err);
                    err = NULL;
                    break;
                }

                const char* unit = "";
                const char* tmp_unit = tcam_property_integer_get_unit(integer);

                if (tmp_unit)
                {
                    unit = tmp_unit;
                }

                printf("%s\ttype: Integer\t"
                       "Display Name: \"%s\" "
                       "Category: %s\n"
                       "\t\t\tDescription: %s\n"
                       "\t\t\tUnit: %s\n"
                       "\t\t\tVisibility: %s\n"
                       "\t\t\tPresentation: %s\n\t\t\t",
                       name,
                       tcam_property_base_get_display_name(base_property),
                       tcam_property_base_get_category(base_property),
                       tcam_property_base_get_description(base_property),
                       unit,
                       g_enum_to_string(tcam_property_visibility_get_type() , tcam_property_base_get_visibility(base_property)),
                       g_enum_to_string(tcam_property_intrepresentation_get_type(), tcam_property_integer_get_representation(integer)));
                print_flags(base_property);
                printf("\n\n"
                       "\t\t\tDefault: %ld\n"
                       "\t\t\tValue: %ld"
                       "\n\n", def, value);

                break;

            }
            case TCAM_PROPERTY_TYPE_FLOAT:
            {
                TcamPropertyFloat* f = TCAM_PROPERTY_FLOAT(base_property);

                gdouble def = tcam_property_float_get_default(f, &err);
;
                gdouble min;
                gdouble max;
                gdouble step;
                tcam_property_float_get_range(f, &min, &max, &step, &err);

                if (err)
                {
                    printf("%s\n", err->message);
                    g_error_free(err);
                    err = NULL;
                    break;
                }

                gdouble value = tcam_property_float_get_value(f, &err);

                if (err)
                {
                    printf("%s\n", err->message);
                    g_error_free(err);
                    err = NULL;
                    break;
                }


                const char* unit = "";
                const char* tmp_unit = tcam_property_float_get_unit(f);

                if (tmp_unit)
                {
                    unit = tmp_unit;
                }

                printf("%s\ttype: Float\t"
                       "Display Name: \"%s\" "
                       "Category: %s\n"
                       "\t\t\tDescription: %s\n"
                       "\t\t\tUnit: %s\n"
                       "\t\t\tVisibility: %s\n"
                       "\t\t\tPresentation: %s\n\t\t\t",
                       name,
                       tcam_property_base_get_display_name(base_property),
                       tcam_property_base_get_category(base_property),
                       tcam_property_base_get_description(base_property),
                       unit,
                       g_enum_to_string(tcam_property_visibility_get_type() , tcam_property_base_get_visibility(base_property)),
                       g_enum_to_string(tcam_property_intrepresentation_get_type(), tcam_property_float_get_representation(f)));
                print_flags(base_property);
                printf("\n\n"
                       "\t\t\tDefault: %f\n"
                       "\t\t\tValue: %f"
                       "\n\n", def, value);

                break;
            }
            case TCAM_PROPERTY_TYPE_ENUMERATION:
            {
                TcamPropertyEnumeration* e = TCAM_PROPERTY_ENUMERATION(base_property);

                const char* value = tcam_property_enumeration_get_value(e, &err);

                if (err)
                {
                    printf("%s\n", err->message);
                    g_error_free(err);
                    err = NULL;
                    break;
                }

                const char* def = tcam_property_enumeration_get_default(e, &err);

                if (err)
                {
                    printf("%s\n", err->message);
                    g_error_free(err);
                    err = NULL;
                    break;
                }

                printf("%s\ttype: Enumeration\t"
                       "Display Name: \"%s\" "
                       "Category: %s\n"
                       "\t\t\tDescription: %s\n"
                       "\t\t\tVisibility: %s\n"
                       "\t\t\t",
                       name, tcam_property_base_get_display_name(base_property),
                       tcam_property_base_get_category(base_property),
                       tcam_property_base_get_description(base_property),
                       g_enum_to_string(tcam_property_visibility_get_type() , tcam_property_base_get_visibility(base_property)));
                print_flags(base_property);
                printf("\n\n"
                       "\t\t\tEntries:");

                GSList* enum_entries = tcam_property_enumeration_get_enum_entries(e, &err);

                if (err)
                {
                    printf("%s\n", err->message);
                    g_error_free(err);
                    break;
                }

                if (enum_entries)
                {
                    for (GSList* entry = enum_entries; entry != NULL; entry = entry->next)
                    {
                        printf(" %s", (const char*)entry->data);
                    }

                    g_slist_free_full(enum_entries, g_free);
                }
                printf("\n\t\t\tDefault: %s\n"
                       "\t\t\tValue: %s\n\n\n", def, value);

                break;
            }
            case TCAM_PROPERTY_TYPE_BOOLEAN:
            {
                TcamPropertyBoolean* b = TCAM_PROPERTY_BOOLEAN(base_property);
                gboolean value = tcam_property_boolean_get_value(b, &err);
                gboolean def = tcam_property_boolean_get_default(b, &err);

                if (err)
                {
                    printf("%s\n", err->message);
                    g_error_free(err);
                    err = NULL;
                    break;
                }

                const char* val_str = "false";
                const char* def_str = "false";

                if (value)
                {
                    val_str = "true";
                }

                if (def)
                {
                    def_str = "true";
                }

                printf("%s\ttype: Boolean\t"
                       "Display Name: \"%s\" "
                       "Category: %s\n"
                       "\t\t\tDescription: %s\n"
                       "\t\t\t",
                       name,
                       tcam_property_base_get_display_name(base_property),
                       tcam_property_base_get_category(base_property),
                       tcam_property_base_get_description(base_property)
                    );
                print_flags(base_property);
                printf("\n\n\t\t\tDefault: %s\n"
                       "\t\t\tValue: %s\n\n\n",
                       def_str, val_str);

                break;
            }
            case TCAM_PROPERTY_TYPE_COMMAND:
            {
                printf("%s\ttype: Command\t"
                       "Display Name: \"%s\" "
                       "Category: %s\n"
                       "\t\t\tDescription: %s\n"
                       "\t\t\t",
                       name,
                       tcam_property_base_get_display_name(base_property),
                       tcam_property_base_get_category(base_property),
                       tcam_property_base_get_description(base_property));
                print_flags(base_property);
                printf("\n\n\n");
                        break;
            }
            default:
            {
                break;
            }
            printf("\n\n\n");
        }
        g_object_unref(base_property);
    }
    g_slist_free_full(names, g_free);
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

    // in the READY state the camera will always be initialized
    gst_element_set_state(pipeline, GST_STATE_READY);

    list_properties(source);

    // cleanup
    gst_element_set_state(pipeline, GST_STATE_NULL);

    gst_object_unref(source);
    gst_object_unref(pipeline);

    return 0;
}
