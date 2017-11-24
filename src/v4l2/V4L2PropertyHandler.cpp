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

#include "V4l2Device.h"
#include "logging.h"
#include "utils.h"
#include "PropertyGeneration.h"

#include <algorithm>

using namespace tcam;


V4l2Device::V4L2PropertyHandler::V4L2PropertyHandler (V4l2Device* dev)
    : device(dev)
{}


std::vector<std::shared_ptr<Property>> V4l2Device::V4L2PropertyHandler::create_property_vector ()
{
    std::vector<std::shared_ptr<Property>> props;

    for ( const auto& p : properties )
    {
        props.push_back(p.prop);
    }

    return props;
}


bool V4l2Device::V4L2PropertyHandler::set_property (const Property& new_property)
{
    auto f = [&new_property] (const property_description& d)
        {
            return ((*d.prop).get_name().compare(new_property.get_name()) == 0);
        };

    auto desc = std::find_if(properties.begin(), properties.end(),f);

    if (desc == properties.end())
    {
        tcam_log(TCAM_LOG_ERROR, "Unable to find Property \"%s\"", new_property.get_name().c_str());
        return false;
    }

    if (desc->id == EMULATED_PROPERTY)
    {
        if (new_property.get_ID() == TCAM_PROPERTY_OFFSET_AUTO)
        {
            auto props = create_property_vector();
            return handle_auto_center(new_property,
                                      props,
                                      device->get_sensor_size(),
                                      device->active_video_format.get_size());
        }
        else
        {
            tcam_log(TCAM_LOG_ERROR, "Emulated property not implemented \"%s\"", new_property.get_name().c_str());
            return false;
        }
    }
    else
    {
        desc->prop->set_struct(new_property.get_struct());

        if (device->changeV4L2Control(*desc))
        {
            return true;
        }
    }
    return false;
}


bool V4l2Device::V4L2PropertyHandler::get_property (Property& p)
{
    auto f = [&p] (const property_description& d)
        {
            return ((*d.prop).get_name().compare(p.get_name()) == 0);
        };

    auto desc = std::find_if(properties.begin(), properties.end(),f);

    if (desc == properties.end())
    {
        std::string s = "Unable to find Property \"" + p.get_name() + "\"";
        tcam_log(TCAM_LOG_ERROR, "%s", s.c_str());
        return false;
    }

    device->updateV4L2Property(*desc);
    p.set_struct(desc->prop->get_struct());

    return false;
}
