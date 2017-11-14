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
#include "utils.h"
#include "PropertyGeneration.h" // handle_auto_center

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
        case TCAM_PROPERTY_GAIN_RED:
        {
            return device->set_color_gain_factor(color_gain::ColorGainRed, new_property.get_struct().value.i.value);
        }
        case TCAM_PROPERTY_GAIN_GREEN:
        {
            device->set_color_gain_factor(color_gain::ColorGainGreen1, new_property.get_struct().value.i.value);
            return device->set_color_gain_factor(color_gain::ColorGainGreen2, new_property.get_struct().value.i.value);
        }
        case TCAM_PROPERTY_GAIN_BLUE:
        {
            return device->set_color_gain_factor(color_gain::ColorGainBlue, new_property.get_struct().value.i.value);
        }
        case TCAM_PROPERTY_OFFSET_AUTO:
        {
            auto props = device->getProperties();
            return tcam::handle_auto_center(new_property, props, {}, device->active_video_format.get_size());
        }
        case TCAM_PROPERTY_OFFSET_X:
        {
            device->active_resolution_conf_.x_addr_start = new_property.get_struct().value.i.value;
            return true;
        }
        case TCAM_PROPERTY_OFFSET_Y:
        {
            device->active_resolution_conf_.y_addr_start = new_property.get_struct().value.i.value;
            return true;
        }
        case TCAM_PROPERTY_BINNING_HORIZONTAL:
        {
            device->active_resolution_conf_.hor_binning = new_property.get_struct().value.i.value;
            return true;
        }
        case TCAM_PROPERTY_BINNING_VERTICAL:
        {
            device->active_resolution_conf_.ver_binning = new_property.get_struct().value.i.value;
            return true;
        }
        case TCAM_PROPERTY_STROBE_ENABLE:
        {
            break;
        }
        case TCAM_PROPERTY_STROBE_DELAY:
        {
            return device->set_strobe(strobe_parameter::first_strobe_delay, new_property.get_struct().value.i.value);
        }
        case TCAM_PROPERTY_STROBE_DELAY_SECOND:
        {
            return device->set_strobe(strobe_parameter::second_strobe_delay, new_property.get_struct().value.i.value);
        }
        case TCAM_PROPERTY_STROBE_DURATION:
        {
            return device->set_strobe(strobe_parameter::first_strobe_duration, new_property.get_struct().value.i.value);
        }
        case TCAM_PROPERTY_STROBE_DURATION_SECOND:
        {
            return device->set_strobe(strobe_parameter::second_strobe_duration, new_property.get_struct().value.i.value);
        }
        case TCAM_PROPERTY_STROBE_POLARITY:
        {
            return device->set_strobe(strobe_parameter::polarity, new_property.get_struct().value.b.value ? 1 : 0);
        }
        case TCAM_PROPERTY_STROBE_MODE:
        {
            return device->set_strobe(strobe_parameter::mode, new_property.get_struct().value.i.value);
        }
        case TCAM_PROPERTY_OIS_MODE:
        {
            return device->set_ois_mode(new_property.get_struct().value.i.value);
        }
        case TCAM_PROPERTY_OIS_POS_X:
        {
            auto props = device->getProperties();
            auto y_pos = find_property(props, TCAM_PROPERTY_OIS_POS_X);
            return device->set_ois_pos(new_property.get_struct().value.i.value,
                                       y_pos->get_struct().value.i.value);
        }
        case TCAM_PROPERTY_OIS_POS_Y:
        {
            auto props = device->getProperties();
            auto y_pos = find_property(props, TCAM_PROPERTY_OIS_POS_X);
            return device->set_ois_pos(y_pos->get_struct().value.i.value,
                                       new_property.get_struct().value.i.value);
        }
        default:
        {
            break;
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
