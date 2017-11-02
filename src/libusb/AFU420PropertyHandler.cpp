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

#include "AFU420Device.h"

#include "logging.h"

#include <algorithm>

using namespace tcam;



tcam::AFU420Device::AFU420PropertyHandler::AFU420PropertyHandler (AFU420Device* dev)
    : device(dev)
{}


std::vector<std::shared_ptr<Property>> tcam::AFU420Device::AFU420PropertyHandler::create_property_vector ()
{
    std::vector<std::shared_ptr<Property>> props;

    for ( const auto& p : properties )
    {
        props.push_back(p.property);
    }

    return props;
}


bool AFU420Device::AFU420PropertyHandler::set_property (const tcam::Property& new_property)
{
    auto f = [&new_property] (const property_description& d)
        {
            return ((*d.property).get_ID() == new_property.get_ID());
        };

    auto desc = std::find_if(properties.begin(), properties.end(), f);

    if (desc == properties.end())
    {
        tcam_error("Device does not have property '%s'", new_property.get_name().c_str());
        return false;
    }

    desc->property->set_struct(new_property.get_struct());

    switch (new_property.get_ID())
    {
        case TCAM_PROPERTY_EXPOSURE:
        {
            return device->set_exposure(new_property.get_struct().value.i.value);
        }
        case TCAM_PROPERTY_GAIN:
        {
            return device->set_gain(new_property.get_struct().value.i.value);
        }
        case TCAM_PROPERTY_FOCUS:
        {
            return device->set_focus(new_property.get_struct().value.i.value);
        }
    }


    return false;
}


bool AFU420Device::AFU420PropertyHandler::get_property (tcam::Property& p)
{
    auto f = [&p] (const property_description& d)
        {
            return ((*d.property).get_name().compare(p.get_name()) == 0);
        };

    auto desc = std::find_if(properties.begin(), properties.end(),f);

    if (desc == properties.end())
    {
        std::string s = "Unable to find Property \"" + p.get_name() + "\"";
        tcam_log(TCAM_LOG_ERROR, "%s", s.c_str());
        return false;
    }

    device->update_property(*desc);

    p.set_struct(desc->property->get_struct());

    return true;
}
