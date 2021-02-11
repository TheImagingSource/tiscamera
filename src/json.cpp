/*
 * Copyright 2018 The Imaging Source Europe GmbH
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


#include "logging.h"

#include <iostream>
#include <json.h>
#include <json/json.hpp>


using json = nlohmann::json;

// std::string


bool property_to_json(tcam::Property* prop, json& parent)
{
    parent.push_back(json::object_t::value_type(prop->get_name(), prop->to_string()));

    return true;
}


std::string tcam::create_json_state(std::shared_ptr<CaptureDevice> dev)
{
    auto props = dev->get_available_properties();


    json j;

    // static const char* JSON_FILE_VERSION_CURRENT = "v0.1";

    try
    {
        j["serial"] = dev->get_device().get_serial();

        j["version"] = JSON_FILE_VERSION_CURRENT;
        j["properties"] = {};
    }
    catch (std::logic_error& err)
    {
        // std::cerr << err.what() << std::endl;
        return "";
    }

    for (const auto p : props)
    {
        if (p->get_type() == TCAM_PROPERTY_TYPE_BUTTON)
        {
            continue;
        }

        property_to_json(p, j["properties"]);
    }

    return j.dump(4);
}


bool serial_matches(json j, const std::string& serial)
{
    std::string serial_str;
    try
    {
        serial_str = j.at("serial");
    }
    catch (json::out_of_range& e)
    {
        SPDLOG_DEBUG("State string has no serial. Omitting check.");
        return true;
    }

    if (!serial_str.empty())
    {
        if (serial_str.compare(serial) != 0)
        {
            SPDLOG_ERROR("Serial mismatch. State string will not be evaluated.");
            return false;
        }
    }
    return true;
}


bool version_matches(json j, const std::string& wanted_version = tcam::JSON_FILE_VERSION_CURRENT)
{
    std::string version;
    try
    {
        version = j.at("version");
    }
    catch (json::out_of_range& e)
    {
        SPDLOG_DEBUG("State string has no version. Omitting check.");
        return true;
    }

    SPDLOG_ERROR(version.c_str());


    if (!version.empty())
    {
        if (version != wanted_version)
        {
            SPDLOG_ERROR("Version mismatch for state file.");
            return false;
        }
    }
    return true;
}


std::pair<bool, std::vector<std::string>> tcam::load_json_state(std::shared_ptr<CaptureDevice> dev,
                                                                const std::string& state)
{
    json j;

    std::vector<std::string> msgs;

    try
    {
        j = json::parse(state);
    }
    catch (json::parse_error& e)
    {
        std::string s = "Unable to parse property settings. JSON parser returned: ";
        s.append(e.what());
        SPDLOG_ERROR(s.c_str());
        msgs.push_back(s);
        return { false, msgs };
    }

    if (!serial_matches(j, dev->get_device().get_serial()))
    {
        return { false, { "Serial missmatch" } };
    }

    if (!version_matches(j))
    {
        return { false, { "Version missmatch" } };
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

    auto dev_props = dev->get_available_properties();

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
        auto p = dev->get_property_by_name(iter.key());

        if (p)
        {
            bool value_ret = p->from_string(iter.value().dump());

            if (!value_ret)
            {
                msgs.push_back("Unabe to apply '" + iter.value().dump() + "' to '" + iter.key()
                               + "'. Ignoring.");
            }
        }
        else
        {
            msgs.push_back("No property with name '" + iter.key() + "'. Ignoring.");
        }
    }

    return { true, msgs };
}
