/*
 * Copyright 2017 The Imaging Source Europe GmbH
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

#include "AFU050Device.h"

#include "UsbHandler.h"
#include "UsbSession.h"

#include "logging.h"
#include "standard_properties.h"
#include "format.h"
#include "afu050_definitions.h"

#include <algorithm>
#include <cstring>

#include <unistd.h>

using namespace tcam;


tcam::AFU050Device::AFU050PropertyHandler::AFU050PropertyHandler (AFU050Device* dev)
    : device(dev)
{}


std::vector<std::shared_ptr<Property>> tcam::AFU050Device::AFU050PropertyHandler::create_property_vector ()
{
    std::vector<std::shared_ptr<Property>> props;

    for ( const auto& p : properties )
    {
        props.push_back(p.prop);
    }

    return props;
}


bool AFU050Device::AFU050PropertyHandler::set_property (const tcam::Property& new_property)
{
    auto f = [&new_property] (const property_description& d)
        {
            return ((*d.prop).get_name().compare(new_property.get_name()) == 0);
        };

    auto desc = std::find_if(properties.begin(), properties.end(),f);

    if (desc == properties.end())
    {
        SPDLOG_ERROR("Unable to find Property \"{}\"", new_property.get_name().c_str());
        return false;
    }

    desc->prop->set_struct(new_property.get_struct());

    if (desc->prop->get_type() == TCAM_PROPERTY_TYPE_INTEGER)
    {
        SPDLOG_DEBUG("Setting int {} to: {}",
                   desc->prop->get_name().c_str(),
                   (std::static_pointer_cast<PropertyInteger>(desc->prop))->get_value());

        return device->set_int_value(desc->unit, desc->id,
                                     (std::static_pointer_cast<PropertyInteger>(desc->prop))->get_value());
    }
    else if (desc->prop->get_type() == TCAM_PROPERTY_TYPE_BOOLEAN)
    {
        SPDLOG_DEBUG("Setting bool {} to: {}", desc->prop->get_name().c_str(), (std::static_pointer_cast<PropertyBoolean>(desc->prop))->get_value());

        return device->set_bool_value(desc->unit, desc->id, (std::static_pointer_cast<PropertyBoolean>(desc->prop))->get_value());
    }
    else if (desc->prop->get_type() == TCAM_PROPERTY_TYPE_BUTTON)
    {
        SPDLOG_DEBUG("Setting button {} to: {}", desc->prop->get_name().c_str(), (std::static_pointer_cast<PropertyBoolean>(desc->prop))->get_value());

        return device->set_bool_value(desc->unit, desc->id, 1);
    }
    else
    {
        SPDLOG_ERROR("Cannot set property");
    }

    return false;
}


bool AFU050Device::AFU050PropertyHandler::get_property (tcam::Property& p)
{
    auto f = [&p] (const property_description& d)
        {
            return ((*d.prop).get_name().compare(p.get_name()) == 0);
        };

    auto desc = std::find_if(properties.begin(), properties.end(),f);

    if (desc == properties.end())
    {
        std::string s = "Unable to find Property \"" + p.get_name() + "\"";
        SPDLOG_ERROR("{}", s.c_str());
        return false;
    }

    device->update_property(*desc);

    p.set_struct(desc->prop->get_struct());

    return true;
}
