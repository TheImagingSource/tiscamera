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

using namespace tcam;

static const size_t name_width = 40;

void print_integer (const Property* p)
{
    PropertyInteger* i = (PropertyInteger*) p;
    auto s = p->get_struct();

    std::cout << std::setw(name_width) <<  i->get_name()
              << "(int)"<< std::right
              << " min=" << i->get_min()
              << " max=" << i->get_max()
              << " step=" << i->get_step()
              << " default=" << i->get_default()
              << " value=" << i->get_value()
              << " category=" << tcam::category2string(s.group.property_category)
              << std::endl;
}


void print_boolean (const Property* p)
{
    PropertyBoolean* s = (PropertyBoolean*) p;
    auto st = p->get_struct();

    std::cout << std::setw(name_width) << s->get_name()
              << "(bool)"
              << " default=";
    if (s->get_default())
    {
        std::cout << "true";
    }
    else
    {
        std::cout << "false";
    }
    std::cout << " value=";
    if (s->get_value())
    {
        std::cout << "true";
    }
    else
    {
        std::cout << "false";
    }
    std::cout               << " category=" << tcam::category2string(st.group.property_category)
                            << std::endl;
}


void print_button (const Property* p)
{
    auto s = p->get_struct();

    std::cout << std::setw(name_width) << p->get_name()
              << "(button)"
              << " category=" << tcam::category2string(s.group.property_category)
              << std::endl;
}


void print_enumeration (const Property* p)
{
    PropertyEnumeration* e = (PropertyEnumeration*) p;
    auto s = p->get_struct();

    std::cout << std::setw(name_width) << e->get_name()
              << "(enum) "

              << " default="<< std::setw(6) << e->get_default()
              << " category=" << tcam::category2string(s.group.property_category)
              << "\n\t\t\t\t\t\tvalue=" << e->get_value() << std::endl;

    for (const auto& val : e->get_values())
    {
        std::cout << "\t\t\t\t\t\t\t"<< val << std::endl;
    }

}


void print_double (const Property* p)
{
    PropertyDouble* d = (PropertyDouble*) p;

    auto s = d->get_struct();

    std::cout << std::setw(name_width) <<  d->get_name()
              << "(double)"<< std::right
              << " min=" << d->get_min()
              << " max=" << d->get_max()
              << " step=" << d->get_step()
              << " default=" << d->get_default()
              << " value=" << d->get_value()
              << " category=" << tcam::category2string(s.group.property_category)
              << " group=" << s.group.property_group
              << std::endl;
}


void print_single_property (const Property* p)
{
    switch (p->get_type())
    {
        case TCAM_PROPERTY_TYPE_INTEGER:
        {
            print_integer(p);
            break;
        }
        case TCAM_PROPERTY_TYPE_DOUBLE:
        {
            print_double(p);
            break;
        }
        case TCAM_PROPERTY_TYPE_STRING:
        {
            break;
        }
        case TCAM_PROPERTY_TYPE_ENUMERATION:
        {
            print_enumeration(p);
            break;
        }
        case TCAM_PROPERTY_TYPE_BOOLEAN:
        {
            print_boolean(p);
            break;
        }
        case TCAM_PROPERTY_TYPE_BUTTON:
        {
            print_button(p);
            break;
        }
        case TCAM_PROPERTY_TYPE_UNKNOWN:
        default:
        {
            std::cerr << "Unknown property type " << p->get_name() << std::endl;
        }
    }
}


void print_properties (const std::string& serial)
{
    GstElement* source = gst_element_factory_make("tcamsrc", "source");

    if (!source)
    {
        std::cerr << "Unable to create source element." << std::endl;
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
                printf("\t\t\t\t\t\t\t %s\n", g_slist_nth(entries, x)->data);
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


static std::vector<std::string> ctrl_split_string (const std::string& to_split,
                                                   const std::string &delim)
{
    std::vector<std::string> vec;

    size_t beg = 0;
    size_t end = 0;

    while (end != std::string::npos)
    {
        end = to_split.find_first_of(delim, beg);

        std::string s = to_split.substr(beg, end - beg);

        vec.push_back(s);

        beg = end + delim.size();
    }

    return vec;
}


bool set_property (std::shared_ptr<CaptureDevice> dev, const std::string& new_prop)
{

    std::vector<std::string> prop_vec = ctrl_split_string(new_prop, "=");

    if (prop_vec.size() != 2)
    {
        std::cout << "Given property string is faulty!" << std::endl;
        return false;
    }

    std::string name = prop_vec.at(0);
    std::string value = prop_vec.at(1);

    auto properties = dev->get_available_properties();

    for (Property* p : properties)
    {
        if (p->get_name().compare(name) == 0)
        {
            std::cout << "Found property!" << std::endl;
            switch(p->get_type())
            {
                case TCAM_PROPERTY_TYPE_DOUBLE:
                {
                    PropertyDouble* prop_d = (PropertyDouble*) p;

                    return prop_d->set_value(std::stod(value));
                }
                case TCAM_PROPERTY_TYPE_STRING:
                {
                    PropertyString* prop_s = (PropertyString*) p;
                    return prop_s->set_value(value);
                }
                case TCAM_PROPERTY_TYPE_ENUMERATION:
                {
                    PropertyEnumeration* prop_m = (PropertyEnumeration*) p;

                    return prop_m->set_value(value);
                }
                case TCAM_PROPERTY_TYPE_BUTTON:
                {
                    PropertyButton* button = (PropertyButton*) p;

                    return button->activate();
                }
                case TCAM_PROPERTY_TYPE_INTEGER:
                {
                    PropertyInteger* prop_i = (PropertyInteger*) p;

                    return prop_i->set_value(std::stoi(value));
                }
                case TCAM_PROPERTY_TYPE_BOOLEAN:
                {
                    PropertyBoolean* prop_s = (PropertyBoolean*) p;
                    if (value == "true" || value == "TRUE" || value == "1")
                    {
                        std::cout << "Setting " << name << " to TRUE"<< std::endl;

                        return prop_s->set_value(true);
                    }
                    else if (value == "false" || value == "FALSE" || value == "0")
                    {
                        std::cout << "Setting " << name << " to FALSE"<< std::endl;
                        return prop_s->set_value(false);
                    }
                    else
                    {
                        std::cout << "Could not interpret \"" << value << "\" as boolean." << std::endl;
                        return false;
                    }

                }
                case TCAM_PROPERTY_TYPE_UNKNOWN:
                default:
                {
                    std::cout << "Found property but am unable to determine correct property type."<< std::endl;
                    std::cout << "Aborting..." << std::endl;
                    return false;
                }
            }
        }
    }

    std::cout << "No property with name '" << name <<"'"<< std::endl;

    return false;
}



void print_state_json (std::shared_ptr<CaptureDevice> dev)
{
    std::cout << create_json_state(dev) << std::endl;
}


void load_state_json_string (std::shared_ptr<CaptureDevice> dev, const std::string json_str)
{
    load_json_state(dev, json_str);
}
