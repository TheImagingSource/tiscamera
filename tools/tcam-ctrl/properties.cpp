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

#include "general.h"
#include <regex>

#include <gst/gst.h>
#include <iomanip>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

#include <gst-helper/gst_ptr.h>

#include <tcam-property-1.0.h>

static const size_t name_width = 40;

namespace
{

gst_helper::gst_ptr<GstElement> open_element(const std::string& element_name = "tcamsrc")
{
    gst_helper::gst_ptr<GstElement> source = gst_helper::make_ptr(gst_element_factory_make(element_name.c_str(), "source"));

    if (!source)
    {
        std::cerr << "Unable to create source element." << std::endl;
        return nullptr;
    }
    return source;
}


bool set_serial(gst_helper::gst_ptr<GstElement>& element, const std::string& serial)
{
    if (!is_valid_device_serial(serial))
    {
        std::cerr << "Device with given serial does not exist." << std::endl;
        return false;
    }

    GValue val = {};
    g_value_init(&val, G_TYPE_STRING);
    g_value_set_static_string(&val, serial.c_str());

    g_object_set_property(G_OBJECT(element.get()), "serial", &val);
    g_value_unset(&val);

    return true;
}


/*
 * Helper struct for state handling
 */
struct ElementStateGuard
{
    explicit ElementStateGuard(GstElement& element)
        : p_element(element)
    {}

    ~ElementStateGuard()
    {
        gst_element_set_state(&p_element, GST_STATE_NULL);
    }

    bool set_state(GstState state)
    {
        auto change_res = gst_element_set_state(&p_element, state);

        if (change_res == GST_STATE_CHANGE_SUCCESS)
        {
            return true;
        }
        else if (change_res == GST_STATE_CHANGE_ASYNC)
        {
            GstClockTime check_timeout = 2000000; // 2 seconds
            auto check_res = gst_element_get_state(&p_element, nullptr, nullptr, check_timeout);

            // only check for success
            // if after 2 seconds that is not the case something is fishy
            // user has to take care of that
            if (check_res == GST_STATE_CHANGE_SUCCESS)
            {
                return true;
            }
        }
        return false;
    }

    GstElement& p_element;
};


std::string get_flag_desc_string(TcamPropertyBase* prop)
{
    std::string ret = "Available: ";
    GError* err = nullptr;
    bool av = tcam_property_base_is_available(prop, &err);
    if (av)
    {
        ret += "yes";
    }
    else
    {
        ret += "no";
    }

    ret += "\tLocked: ";

    bool lo = tcam_property_base_is_locked(prop, &err);

    if (lo)
    {
        ret += "yes";
    }
    else
    {
        ret += "no";
    }
    return ret;
}

} // namespace


void print_properties(const std::string& serial)
{

    auto source = open_element();

    if (!source)
    {
        return;
    }

    if (!set_serial(source, serial))
    {
        return;
    }

    {
        ElementStateGuard state_guard(*source.get());

        if (!state_guard.set_state(GST_STATE_READY))
        {
            std::cerr << "Unable to set state of tcamsrc to READY." << std::endl;
            return;
        }

        GError* err = nullptr;
        GSList* n = tcam_property_provider_get_tcam_property_names(TCAM_PROPERTY_PROVIDER(source.get()), &err);

        // convert GSList to vector
        // do this to make sorting easier
        // names should be alphabetically sorted
        // this is simply to make finding a property
        // more predictable
        std::vector<std::string> names;

        names.reserve(g_slist_length(n));

        for (unsigned int i = 0; i < g_slist_length(n); ++i)
        {
            names.push_back((char*)g_slist_nth(n, i)->data);
        }

        g_slist_free_full(n, g_free);

        std::sort(names.begin(), names.end(),
                  [] (const std::string& a, const std::string& b)
                  {
                      return a<b;
                  });

        for (const auto& name : names)
        {
            err = nullptr;

            TcamPropertyBase* base_property = tcam_property_provider_get_tcam_property(TCAM_PROPERTY_PROVIDER(source.get()),
                                                                                       name.c_str(), &err);

            if (err)
            {
                std::cout << "Error while retrieving property \"" << name << "\": " << err->message << std::endl;
                g_error_free(err);
                err = nullptr;
                continue;
            }

            if (!base_property)
            {
                std::cout << "Unable to retrieve property \"" << name << "\"" << std::endl;
                continue;
            }

            TcamPropertyType type = tcam_property_base_get_property_type(base_property);

            switch(type)
            {
                case TCAM_PROPERTY_TYPE_INTEGER:
                {
                    TcamPropertyInteger* integer = TCAM_PROPERTY_INTEGER(base_property);

                    gint64 min;
                    gint64 max;
                    gint64 def = tcam_property_integer_get_default(integer, &err);
                    gint64 step;
                    tcam_property_integer_get_range(integer, &min, &max, &step, &err);

                    if (err)
                    {
                        std::cout << err->message << std::endl;
                        g_error_free(err);
                        err = nullptr;
                        break;
                    }


                    gint64 value = tcam_property_integer_get_value(integer, &err);

                    if (err)
                    {
                        std::cerr << err->message << std::endl;
                        g_error_free(err);
                        err = nullptr;
                        break;
                    }

                    std::string unit;
                    const char* tmp_unit = tcam_property_integer_get_unit(integer);

                    if (tmp_unit)
                    {
                        unit = tmp_unit;
                    }

                    std::cout << std::setw(20) << std::left << name << "\ttype: Integer\t"
                              << "Display Name: \"" << tcam_property_base_get_display_name(base_property) << "\" "
                              << "Category: " << tcam_property_base_get_category(base_property) << std::endl
                              << "\t\t\tDescription: " << tcam_property_base_get_description(base_property) << std::endl
                              << "\t\t\tUnit: " << unit << std::endl
                              << "\t\t\tVisibility: " << tcam_property_base_get_visibility(base_property) << std::endl
                              << "\t\t\tPresentation: " << g_enum_to_string(tcam_property_intrepresentation_get_type(),
                                                                            tcam_property_integer_get_representation(integer)) << std::endl
                              << "\t\t\t" << get_flag_desc_string(base_property) << std::endl
                              << "" << std::endl
                              << "\t\t\tMin: " << min << "\tMax: " << max << "\tStep: " << step << std::endl
                              << "\t\t\tDefault: " << def << std::endl
                              << "\t\t\tValue: " << value
                              << std::endl << std::endl;

                    break;

                }
                case TCAM_PROPERTY_TYPE_FLOAT:
                {
                    TcamPropertyFloat* f = TCAM_PROPERTY_FLOAT(base_property);

                    gdouble min;
                    gdouble max;
                    gdouble def = tcam_property_float_get_default(f, &err);
                    gdouble step;
                    tcam_property_float_get_range(f, &min, &max, &step, &err);

                    if (err)
                    {
                        std::cerr << err->message << std::endl;
                        g_error_free(err);
                        err = nullptr;
                        break;
                    }

                    gdouble value = tcam_property_float_get_value(f, &err);

                    if (err)
                    {
                        std::cerr << err->message << std::endl;
                        g_error_free(err);
                        err = nullptr;
                        break;
                    }

                    std::string unit;
                    const char* tmp_unit = tcam_property_float_get_unit(f);

                    if (tmp_unit)
                    {
                        unit = tmp_unit;
                    }

                    std::cout << std::setw(20) << std::left << std::setprecision(3) << std::fixed
                              << name << "\ttype: Float\t"
                              << "Display Name: \"" << tcam_property_base_get_display_name(base_property) << "\" "
                              << "Category: " << tcam_property_base_get_category(base_property) << std::endl
                              << "\t\t\tDescription: " << tcam_property_base_get_description(base_property) << std::endl
                              << "\t\t\tUnit: " << unit << std::endl
                              << "\t\t\tVisibility: " << tcam_property_base_get_visibility(base_property) << std::endl
                              << "\t\t\tPresentation: " << g_enum_to_string(tcam_property_floatrepresentation_get_type(),
                                                                            tcam_property_float_get_representation(f)) << std::endl
                              << "\t\t\t" << get_flag_desc_string(base_property) << std::endl
                              << "" << std::endl
                              << "\t\t\tMin: " << min << "\tMax: " << max << "\tStep: " << step << std::endl
                              << "\t\t\tDefault: " << def << std::endl
                              << "\t\t\tValue: " << value
                              << std::endl << std::endl;

                    break;
                }
                case TCAM_PROPERTY_TYPE_ENUMERATION:
                {
                    TcamPropertyEnumeration* e = TCAM_PROPERTY_ENUMERATION(base_property);

                    const char* value = tcam_property_enumeration_get_value(e, &err);

                    if (err)
                    {
                        std::cerr << name << ": " << err->message << std::endl;
                        g_error_free(err);
                        err = nullptr;
                        break;
                    }

                    const char* def = tcam_property_enumeration_get_default(e, &err);

                    if (err)
                    {
                        std::cerr << name << ": " << err->message << std::endl;
                        g_error_free(err);
                        err = nullptr;
                        break;
                    }

                    std::string entries;

                    GSList* enum_entries = tcam_property_enumeration_get_enum_entries(e, &err);

                    if (err)
                    {
                        std::cerr << name << ": " << err->message << std::endl;
                        g_error_free(err);
                        err = nullptr;
                        break;
                    }

                    if (enum_entries)
                    {
                        for (GSList* entry = enum_entries; entry != nullptr; entry = entry->next)
                        {
                            entries += " ";
                            entries += (const char*)entry->data;
                        }

                        g_slist_free_full(enum_entries, g_free);
                    }


                    std::cout << std::setw(20) << std::left << name << "\ttype: Enumeration\t"
                              << "Display Name: \"" << tcam_property_base_get_display_name(base_property) << "\" "
                              << "Category: " << tcam_property_base_get_category(base_property) << std::endl
                              << "\t\t\tDescription: " << tcam_property_base_get_description(base_property) << std::endl
                              << "\t\t\tVisibility: " << tcam_property_base_get_visibility(base_property) << std::endl
                              << "\t\t\t" << get_flag_desc_string(base_property) << std::endl
                              << "" << std::endl
                              << "\t\t\tEntries:" << entries << std::endl
                              << "\t\t\tDefault: " << def << std::endl
                              << "\t\t\tValue: " << value
                              << std::endl << std::endl;

                    break;
                }
                case TCAM_PROPERTY_TYPE_BOOLEAN:
                {
                    TcamPropertyBoolean* b = TCAM_PROPERTY_BOOLEAN(base_property);
                    gboolean value = tcam_property_boolean_get_value(b, &err);
                    gboolean def = tcam_property_boolean_get_default(b, &err);

                    if (err)
                    {
                        std::cerr << err->message << std::endl;
                        g_error_free(err);
                        err = nullptr;
                        break;
                    }

                    auto bool_string = [] (bool bv)
                    {
                        if (bv)
                        {
                            return "true";
                        }
                        return "false";
                    };

                    std::cout << std::setw(20) << std::left << name << "\ttype: Boolean\t"
                              << "Display Name: \"" << tcam_property_base_get_display_name(base_property) << "\" "
                              << "Category: " << tcam_property_base_get_category(base_property) << std::endl
                              << "\t\t\tDescription: " << tcam_property_base_get_description(base_property) << std::endl
                              << "\t\t\t" << get_flag_desc_string(base_property) << std::endl
                              << "" << std::endl
                              << "\t\t\tDefault: " << bool_string(def) << std::endl
                              << "\t\t\tValue: " << bool_string(value)
                              << std::endl << std::endl;

                    break;

                }
                case TCAM_PROPERTY_TYPE_COMMAND:
                {
                    std::cout << std::setw(20) << std::left << name << "\ttype: Command\t"
                              << "Display Name: \"" << tcam_property_base_get_display_name(base_property) << "\" "
                              << "Category: " << tcam_property_base_get_category(base_property) << std::endl
                              << "\t\t\tDescription: " << tcam_property_base_get_description(base_property) << std::endl
                              << "\t\t\t" << get_flag_desc_string(base_property) << std::endl
                              << std::endl << std::endl;

                    break;

                }
                default:
                {
                    break;
                }
                std::cout << std::endl << "\n" << std::endl;

                if (base_property)
                {
                    g_object_unref(base_property);
                }
            }

        }

    } // ElementStateGuard
}


void print_tcam_properties(const std::string& serial, OutputType print_type)
{
    auto source = open_element();
    if (!source)
    {
        return;
    }

    if (!set_serial(source, serial))
    {
        return;
    }

    {
        ElementStateGuard state_guard(*source.get());
        if (!state_guard.set_state(GST_STATE_READY))
        {
            std::cerr << "Unable to change source to state READY." << std::endl;
            return;
        }

        GValue current_properties = G_VALUE_INIT;
        g_value_init(&current_properties, GST_TYPE_STRUCTURE);

        g_object_get_property(G_OBJECT(source.get()), "tcam-properties", &current_properties);

        // get a string to print the current property state
        // gst_value_get_structure has transfer: none
        char* struc_str = gst_structure_to_string(gst_value_get_structure(&current_properties));

        if (!struc_str)
        {
            std::cerr << "Unable to extract string from GstStructure" << std::endl;
            g_value_unset(&current_properties);
            return;
        }

        if (print_type == OutputType::AsIs)
        {
            std::cout << struc_str << std::endl;
        }
        else
        {
            std::regex to_remove ("\\([a-zA-Z0-9]*\\)"); // name=(type)value -> remove `(type)`
            std::regex to_remove2 ("\\,\\ "); // `name=value, name=value` -> `name->value,name->value`
            std::regex to_remove3 ("\""); // `"` -> `\"`

            std::string s (struc_str);
            std::string res;
            std::regex_replace(std::back_inserter(res), s.begin(), s.end(), to_remove, "");

            std::string res2;
            std::regex_replace(std::back_inserter(res2), res.begin(), res.end(), to_remove2, ",");

            std::string res3;
            std::regex_replace(std::back_inserter(res3), res2.begin(), res2.end(), to_remove3, "\\\"");

            std::cout << res3 << std::endl;
        }

        g_free(struc_str);
        g_value_unset(&current_properties);
    }
}


void load_tcam_properties(const std::string& serial,
                          const std::string& property_string)
{
    auto source = open_element();
    if (!source)
    {
        return;
    }

    if (!set_serial(source, serial))
    {
        return;
    }

    GValue current_properties = G_VALUE_INIT;
    g_value_init(&current_properties, GST_TYPE_STRUCTURE);

    auto new_property_struct = gst_helper::make_ptr<GstStructure>(gst_structure_new_from_string(property_string.c_str()));

    if (!new_property_struct)
    {
        std::cerr << "Not loadable." << std::endl;

        g_value_unset(&current_properties);
        return;
    }

    gst_value_set_structure(&current_properties, new_property_struct.get());

    {
        ElementStateGuard state_guard(*source.get());
        state_guard.set_state(GST_STATE_READY);

        g_object_get_property(G_OBJECT(source.get()), "tcam-properties", &current_properties);
    }
    g_value_unset(&current_properties);

}


void print_state_json(const std::string& serial)
{
    auto source = open_element();

    if (!source)
    {
        return;
    }

    if (!set_serial(source, serial))
    {
        return;
    }

    {
        ElementStateGuard state__guard (*source.get());
        state__guard.set_state(GST_STATE_READY);

        char* state_str = nullptr;
        g_object_get(G_OBJECT(source.get()), "tcam-properties-json", &state_str, NULL);
        if (state_str != nullptr)
        {
            std::cout << state_str << std::endl;
            g_free(state_str);
        }

    }
}


void load_state_json_string(const std::string& serial, const std::string& json_str)
{
    auto source = open_element();

    if (!source)
    {
        return;
    }
    if (!set_serial(source, serial))
    {
        return;
    }

    {
        ElementStateGuard state_guard(*source.get());
        state_guard.set_state(GST_STATE_READY);

        g_object_set(G_OBJECT(source.get()), "tcam-properties-json", json_str.c_str(), NULL);

    }
}
