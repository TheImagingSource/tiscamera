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

#include "tcamgstjson.h"

#include "../../../external/json/json.hpp"
#include "../../gobject/tcamprop.h"
#include "../../json.h"
#include "../../logging.h"

// for convenience
using json = nlohmann::json;


static bool tcam_property_to_json(_TcamProp* prop, json& parent, const char* name)
{
    GValue value = G_VALUE_INIT;
    GValue type = G_VALUE_INIT;

    gboolean ret = tcam_prop_get_tcam_property(TCAM_PROP(prop),
                                               name,
                                               &value,
                                               nullptr,
                                               nullptr,
                                               nullptr,
                                               nullptr,
                                               &type,
                                               nullptr,
                                               nullptr,
                                               nullptr);

    if (!ret || g_value_get_string(&type) == nullptr)
    {
        g_value_unset(&value);
        g_value_unset(&type);

        SPDLOG_WARN("Unable to read property '{}'", name);
        return false;
    }

    if (g_strcmp0("button", g_value_get_string(&type)) == 0) // button property can be skipped
    {
        g_value_unset(&value);
        g_value_unset(&type);
        return true;
    }
    g_value_unset(&type);

    try
    {
        switch (G_VALUE_TYPE(&value))
        {
            case G_TYPE_INT:
            {
                parent.push_back(json::object_t::value_type(name, g_value_get_int(&value)));
                break;
            }
            case G_TYPE_DOUBLE:
            {
                parent.push_back(json::object_t::value_type(name, g_value_get_double(&value)));
                break;
            }
            case G_TYPE_STRING:
            {
                parent.push_back(json::object_t::value_type(name, g_value_get_string(&value)));
                break;
            }
            case G_TYPE_BOOLEAN:
            {
                parent.push_back(
                    json::object_t::value_type(name, (bool)g_value_get_boolean(&value)));
                break;
            }
            default:
            {
                break;
            }
        }
    }
    catch (const std::logic_error& err)
    {
        g_value_unset(&value);
        SPDLOG_ERROR(err.what());
        return false;
    }
    g_value_unset(&value);
    return true;
}


std::string tcam::gst::create_device_settings(const std::string& serial, _TcamProp* tcam)
{
    if (!tcam)
    {
        return "";
    }

    json j;
    try
    {
        if (!serial.empty())
        {
            j["serial"] = serial;
        }
        j["version"] = tcam::JSON_FILE_VERSION_CURRENT;
        j["properties"] = {};
    }
    catch (std::logic_error& err)
    {
        SPDLOG_ERROR(err.what());
        return "";
    }

    GSList* names = tcam_prop_get_tcam_property_names(tcam);

    for (unsigned int i = 0; i < g_slist_length(names); ++i)
    {
        const char* prop_name = (const char*)g_slist_nth_data(names, i);

        bool ret = tcam_property_to_json(tcam, j["properties"], prop_name);
        if (!ret)
        {
            SPDLOG_WARN("Could not convert {} to json.", prop_name);
        }
    }
    g_slist_free_full(names, ::g_free);

    int indent = 4;
    return j.dump(indent);
}


bool tcam::gst::load_device_settings(_TcamProp* tcam,
                                     const std::string& serial,
                                     const std::string& cache)
{
    if (!tcam)
    {
        return false;
    }

    SPDLOG_INFO(cache.c_str());
    json j;
    try
    {
        j = json::parse(cache);
    }
    catch (json::parse_error& e)
    {
        SPDLOG_ERROR("Unable to parse property settings. JSON parser returned: {}", e.what());
        return false;
    }

    std::string serial_str;
    try
    {
        serial_str = j.at("serial");
    }
    catch (json::out_of_range& e)
    {
        SPDLOG_DEBUG("State string has no serial. Omitting check.");
    }

    std::string version;
    try
    {
        version = j.at("version");
    }
    catch (json::out_of_range& e)
    {
        SPDLOG_DEBUG("State string has no version. Omitting check.");
    }

    if (!serial_str.empty())
    {
        if (serial_str.compare(serial) != 0)
        {
            SPDLOG_ERROR("Serial mismatch. State string will not be evaluated.");
            return false;
        }
    }

    if (!version.empty())
    {
        if (version != tcam::JSON_FILE_VERSION_CURRENT)
        {
            SPDLOG_ERROR("Version mismatch for state file.");
            return false;
        }
    }

    /*
      This check is to allow for simplified
      versions that can be used with gst-launch
      all meta data is omitted and the properties
      name does not exist.
      basically root == properties
     */
    json props;

    if (j.find("properties") != j.end())
    {
        props = j["properties"];
    }
    else
    {
        props = j;
    }


    /*
      the order is alphabetical i.e. not in the same order the input data is
      since we use the string names for property identification
      we use the reverse order to apply auto properties before properties
      e.g. 'Exposure Auto' before 'Exposure'.
      This allows the disabling of read-only flags for exposure
      before setting the exposure value
    */
    for (auto iter = props.rbegin(); iter != props.rend(); iter++)
    {
        GValue value = G_VALUE_INIT;

        if (iter.value().is_boolean())
        {
            g_value_init(&value, G_TYPE_BOOLEAN);
            g_value_set_boolean(&value, iter.value());
        }
        else if (iter.value().is_number_float())
        {
            g_value_init(&value, G_TYPE_DOUBLE);
            g_value_set_double(&value, iter.value());
        }
        else if (iter.value().is_number_integer())
        {
            g_value_init(&value, G_TYPE_INT);
            g_value_set_int(&value, iter.value());
        }
        else
        {
            g_value_init(&value, G_TYPE_STRING);
            g_value_set_string(&value, iter.value().get<std::string>().c_str());
        }

        gboolean ret = tcam_prop_set_tcam_property(tcam, iter.key().c_str(), &value);

        if (!ret)
        {
            // iter.value().dump() will add "" around a string
            // this is to signify that it is in fact, a string
            //
            SPDLOG_ERROR("Setting '{}' to {} caused an error",
                         iter.key().c_str(),
                         iter.value().dump().c_str());
        }
        g_value_unset(&value);
    }

    return true;
}
