/*
 * Copyright 2014 The Imaging Source Europe GmbH
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

#include "properties.h"

#include "utils.h"

#include <iostream>
#include <iomanip>
#include <memory>

#include <gst/gst.h>
#include "tcamprop.h"

#include "tcam.h"

#include "general.h"

using namespace tcam;

static const size_t name_width = 40;



void print_properties (const std::string& serial)
{
    GstElement* source = gst_element_factory_make("tcamsrc", "source");

    if (!source)
    {
        std::cerr << "Unable to create source element." << std::endl;
        return;
    }

    if (!is_valid_device_serial(source, serial))
    {
        std::cerr << "Device with given serial does not exist." << std::endl;
        return;
    }

    std::string str;

    GValue val = {};
    g_value_init(&val, G_TYPE_STRING);
    g_value_set_static_string(&val, serial.c_str());

    g_object_set_property(G_OBJECT(source), "serial", &val);

    GSList* names = tcam_prop_get_tcam_property_names(TCAM_PROP(source));

    for (unsigned int i = 0; i < g_slist_length(names); ++i)
    {
        char* name = (char*)g_slist_nth(names, i)->data;

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
        std::cout << std::left;

        const char* t = g_value_get_string(&type);
        if (strcmp(t, "integer") == 0)
        {
            std::cout << std::setw(name_width) << name
                      << "(int)"<< std::right
                      << " min=" << g_value_get_int(&min)
                      << " max=" << g_value_get_int(&max)
                      << " step=" << g_value_get_int(&step_size)
                      << " default=" << g_value_get_int(&default_value)
                      << " value=" << g_value_get_int(&default_value)
                      << " category=" << g_value_get_string(&category)
                      << " group=" << g_value_get_string(&group)
                      << std::endl;
        }
        else if (strcmp(t, "double") == 0)
        {
            std::cout << std::setw(name_width) << name
                      << "(double)"<< std::right
                      << " min=" << g_value_get_double(&min)
                      << " max=" << g_value_get_double(&max)
                      << " step=" << g_value_get_double(&step_size)
                      << " default=" << g_value_get_double(&default_value)
                      << " value=" << g_value_get_double(&value)
                      << " category=" << g_value_get_string(&category)
                      << " group=" << g_value_get_string(&group)
                      << std::endl;
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


            std::cout << std::setw(name_width) << name
                      << "(enum) "
                      << " default="<< std::setw(6) << g_value_get_string(&default_value)
                      << " category=" << g_value_get_string(&category)
                      << " group=" << g_value_get_string(&group)
                      << "\n\t\t\t\t\t\tvalue=" << g_value_get_string(&value) << std::endl;
            for (unsigned int x = 0; x < g_slist_length(entries); ++x)
            {
                printf("\t\t\t\t\t\t\t %s\n", (const char*)g_slist_nth(entries, x)->data);
            }
        }
        else if (strcmp(t, "boolean") == 0)
        {
            std::cout << std::setw(name_width) << name
                      << "(bool)"
                      << " default=";
            if (g_value_get_boolean(&default_value))
            {
                std::cout << "true";
            }
            else
            {
                std::cout << "false";
            }
            std::cout << " value=";
            if (g_value_get_boolean(&value))
            {
                std::cout << "true";
            }
            else
            {
                std::cout << "false";
            }
            std::cout << " category=" << g_value_get_string(&category)
                      << " group=" << g_value_get_string(&group)
                      << std::endl;
        }
        else if (strcmp(t, "button") == 0)
        {
            std::cout << std::setw(name_width) << name
                      << "(button)"
                      << " category=" << g_value_get_string(&category)
                      << " group=" << g_value_get_string(&group)
                      << std::endl;
        }
        else
        {
            printf("Property '%s' has type '%s' .\n", name, t);
        }
    }

    g_slist_free(names);
    gst_object_unref(source);

}


void print_state_json (const std::string& serial)
{

    GstElement* source = gst_element_factory_make("tcamsrc", "source");

    if (!source)
    {
        std::cerr << "Unable to create source element." << std::endl;
        return;
    }

    if (!is_valid_device_serial(source, serial))
    {
        std::cerr << "Device with given serial does not exist." << std::endl;
        return;
    }

    GValue val = {};
    g_value_init(&val, G_TYPE_STRING);
    g_value_set_static_string(&val, serial.c_str());

    g_object_set_property(G_OBJECT(source), "serial", &val);

    gst_element_set_state(source, GST_STATE_READY);

    const char* state_str = nullptr;
    g_object_get(G_OBJECT(source), "state", &state_str, NULL);

    std::cout << state_str << std::endl;
    gst_element_set_state(source, GST_STATE_NULL);

    gst_object_unref(source);
}


void load_state_json_string (const std::string& serial, const std::string& json_str)
{
    GstElement* source = gst_element_factory_make("tcamsrc", "source");

    if (!source)
    {
        std::cerr << "Unable to create source element." << std::endl;
        return;
    }

    if (!is_valid_device_serial(source, serial))
    {
        std::cerr << "Device with given serial does not exist." << std::endl;
        return;
    }

    GValue val = {};
    g_value_init(&val, G_TYPE_STRING);
    g_value_set_static_string(&val, serial.c_str());

    g_object_set_property(G_OBJECT(source), "serial", &val);

    gst_element_set_state(source, GST_STATE_READY);

    g_object_set(G_OBJECT(source), "state", json_str.c_str(), NULL);
    gst_element_set_state(source, GST_STATE_NULL);

    gst_object_unref(source);
}
