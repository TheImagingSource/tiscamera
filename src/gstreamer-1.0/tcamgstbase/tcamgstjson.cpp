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
#include "../../json.h"
#include "../../logging.h"

// for convenience
using json = nlohmann::json;


std::string tcam::gst::create_device_settings(const std::string& serial, TcamPropertyProvider* tcam)
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

    GError* err = nullptr;
    GSList* names = tcam_property_provider_get_tcam_property_names(tcam, &err);

    for (unsigned int i = 0; i < g_slist_length(names); ++i)
    {
        const char* prop_name = (const char*)g_slist_nth_data(names, i);

        auto prop_base = tcam_property_provider_get_tcam_property(tcam, prop_name, &err);


        switch (tcam_property_base_get_property_type(prop_base))
        {
            case TCAM_PROPERTY_TYPE_INTEGER:
            {
                auto prop_i = TCAM_PROPERTY_INTEGER(prop_base);

                auto value = tcam_property_integer_get_value(prop_i, &err);

                if (!err)
                {
                    j["properties"].push_back(json::object_t::value_type(prop_name, value));
                }

                break;
            }
            case TCAM_PROPERTY_TYPE_FLOAT:
            {
                auto f = TCAM_PROPERTY_FLOAT(prop_base);

                auto value = tcam_property_float_get_value(f, &err);

                if (!err)
                {
                    j["properties"].push_back(json::object_t::value_type(prop_name, value));
                }

                break;
            }
            case TCAM_PROPERTY_TYPE_ENUMERATION:
            {
                auto e = TCAM_PROPERTY_ENUMERATION(prop_base);

                auto value = tcam_property_enumeration_get_value(e, &err);

                if (!err)
                {
                    j["properties"].push_back(json::object_t::value_type(prop_name, value));
                }

                break;
            }
            case TCAM_PROPERTY_TYPE_BOOLEAN:
            {
                auto b = TCAM_PROPERTY_BOOLEAN(prop_base);

                auto value = tcam_property_boolean_get_value(b,&err);

                if (!err)
                {
                    j["properties"].push_back(json::object_t::value_type(prop_name, value));
                }

                break;
            }
            case TCAM_PROPERTY_TYPE_COMMAND:
            {
                auto c = TCAM_PROPERTY_COMMAND(prop_base);

                tcam_property_command_set_command(c, &err);
                break;
            }
            default:
            {
                break;
            }
        }

        if (err)
        {
            SPDLOG_ERROR("Reading '{}' caused an error: {}",
                         prop_name,
                         err->message);
            g_error_free(err);
            err = nullptr;
        }

    }
    g_slist_free_full(names, ::g_free);

    int indent = 4;
    return j.dump(indent);
}


bool tcam::gst::load_device_settings(TcamPropertyProvider* tcam,
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

        GError* err = nullptr;
        auto prop_base = tcam_property_provider_get_tcam_property(tcam, iter.key().c_str(), &err);

        switch (tcam_property_base_get_property_type(prop_base))
        {
            case TCAM_PROPERTY_TYPE_INTEGER:
            {
                if (!iter.value().is_number())
                {
                    SPDLOG_ERROR("Value for {} is not a number", iter.key().c_str());
                    break;
                }

                auto i = TCAM_PROPERTY_INTEGER(prop_base);

                tcam_property_integer_set_value(i, iter.value(), &err);

                break;
            }
            case TCAM_PROPERTY_TYPE_FLOAT:
            {
                if (!iter.value().is_number())
                {
                    SPDLOG_ERROR("Value for {} is not a number", iter.key().c_str());
                    break;
                }

                auto f = TCAM_PROPERTY_FLOAT(prop_base);

                tcam_property_float_set_value(f, iter.value(), &err);

                break;
            }
            case TCAM_PROPERTY_TYPE_ENUMERATION:
            {

                auto e = TCAM_PROPERTY_ENUMERATION(prop_base);

                tcam_property_enumeration_set_value(e, iter.value().get<std::string>().c_str(), &err);

                break;
            }
            case TCAM_PROPERTY_TYPE_BOOLEAN:
            {

                if (!iter.value().is_boolean())
                {
                    SPDLOG_ERROR("Value for {} is not a bool", iter.key().c_str());
                    break;
                }

                auto b = TCAM_PROPERTY_BOOLEAN(prop_base);

                tcam_property_boolean_set_value(b, iter.value(), &err);

                break;
            }
            case TCAM_PROPERTY_TYPE_COMMAND:
            {
                auto c = TCAM_PROPERTY_COMMAND(prop_base);

                tcam_property_command_set_command(c, &err);
                break;
            }
            default:
            {
                break;
            }
        }

        if (err)
        {
            // iter.value().dump() will add "" around a string
            // this is to signify that it is in fact, a string
            //
            SPDLOG_ERROR("Setting '{}' to {} caused an error: {}",
                         iter.key().c_str(),
                         iter.value().dump().c_str(),
                         err->message);
            g_error_free(err);
            err = nullptr;
        }
    }

    return true;
}
