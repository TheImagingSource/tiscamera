/*
 * Copyright 2021 The Imaging Source Europe GmbH
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

#include "AFU420DeviceBackend.h"

#include "AFU420Device.h"
#include "utils.h"

using namespace tcam::afu420;

namespace tcam::property
{

AFU420DeviceBackend::AFU420DeviceBackend(AFU420Device* dev) : p_device(dev) {}


static int camera_to_color_gain(double value)
{
    return map_value_ranges(0, (4.0 - (1.0 / 256.0)), 0, 255, value);
}


outcome::result<int64_t> AFU420DeviceBackend::get_int(tcam::afu420::AFU420Property id)
{
    switch (id)
    {
        case tcam::afu420::AFU420Property::ExposureTime:
        {
            return p_device->get_exposure();
        }
        case tcam::afu420::AFU420Property::Gain:
        {
            return p_device->get_gain();
        }
        case tcam::afu420::AFU420Property::Focus:
        {
            return p_device->get_focus();
        }
        case tcam::afu420::AFU420Property::HDR:
        {
            return p_device->get_hdr();
        }
        case tcam::afu420::AFU420Property::WB_Red:
        {
            double value;
            p_device->get_color_gain_factor(AFU420Device::color_gain::ColorGainRed, value);

            return camera_to_color_gain(value);
        }
        case tcam::afu420::AFU420Property::WB_Green:
        {
            double value;
            p_device->get_color_gain_factor(AFU420Device::color_gain::ColorGainGreen1, value);

            return camera_to_color_gain(value);
        }
        case tcam::afu420::AFU420Property::WB_Blue:
        {
            double value;
            p_device->get_color_gain_factor(AFU420Device::color_gain::ColorGainBlue, value);

            return camera_to_color_gain(value);
        }
        case tcam::afu420::AFU420Property::StrobeDelay:
        {
            return p_device->get_strobe(AFU420Device::strobe_parameter::first_strobe_delay);
        }
        case tcam::afu420::AFU420Property::StrobeDuration:
        {
            return p_device->get_strobe(AFU420Device::strobe_parameter::first_strobe_duration);
        }
        case tcam::afu420::AFU420Property::StrobeDelaySecond:
        {
            return p_device->get_strobe(AFU420Device::strobe_parameter::second_strobe_delay);
        }
        case tcam::afu420::AFU420Property::StrobeDurationSecond:
        {
            return p_device->get_strobe(AFU420Device::strobe_parameter::second_strobe_duration);
        }
        case tcam::afu420::AFU420Property::StrobePolarity:
        {
            return p_device->get_strobe(AFU420Device::strobe_parameter::polarity);
        }
        case tcam::afu420::AFU420Property::OISMode:
        {
            return p_device->get_ois_mode();
        }
        case tcam::afu420::AFU420Property::OISPosX:
        {
            return p_device->m_ois_pos_x;
        }
        case tcam::afu420::AFU420Property::OISPosY:
        {
            return p_device->m_ois_pos_y;
        }
        case tcam::afu420::AFU420Property::BinningHorizontal:
        {
            return p_device->m_binning.width;
        }
        case tcam::afu420::AFU420Property::BinningVertical:
        {
            return p_device->m_binning.height;
        }
        case tcam::afu420::AFU420Property::OffsetAuto:
        {
            return p_device->m_offset_auto;
        }
        case tcam::afu420::AFU420Property::OffsetX:
        {
            return p_device->m_offset.width;
        }
        case tcam::afu420::AFU420Property::OffsetY:
        {
            return p_device->m_offset.height;
        }
        default:
        {
            return tcam::status::PropertyDoesNotExist;
        }
    }
}


outcome::result<void> AFU420DeviceBackend::set_int(tcam::afu420::AFU420Property id,
                                                   int64_t new_value)
{
    switch (id)
    {
        case tcam::afu420::AFU420Property::ExposureTime:
        {
            if (p_device->set_exposure(new_value))
            {
                return outcome::success();
            }
            return tcam::status::UndefinedError;
        }
        case tcam::afu420::AFU420Property::Gain:
        {
            if (p_device->set_gain(new_value))
            {
                return outcome::success();
            }
            return tcam::status::UndefinedError;
        }
        case tcam::afu420::AFU420Property::Focus:
        {
            if (p_device->set_focus(new_value))
            {
                return outcome::success();
            }
            return tcam::status::UndefinedError;
        }
        case tcam::afu420::AFU420Property::HDR:
        {
            if (p_device->set_hdr(new_value))
            {
                return outcome::success();
            }
            return tcam::status::UndefinedError;
        }
        case tcam::afu420::AFU420Property::WB_Red:
        {
            if (p_device->set_color_gain_factor(AFU420Device::color_gain::ColorGainRed, new_value))
            {
                return outcome::success();
            }
            return tcam::status::UndefinedError;
        }
        case tcam::afu420::AFU420Property::WB_Green:
        {
            if (p_device->set_color_gain_factor(AFU420Device::color_gain::ColorGainGreen1,
                                                new_value))
            {
                return outcome::success();
            }
            return tcam::status::UndefinedError;
        }
        case tcam::afu420::AFU420Property::WB_Blue:
        {
            if (p_device->set_color_gain_factor(AFU420Device::color_gain::ColorGainBlue, new_value))
            {
                return outcome::success();
            }
            return tcam::status::UndefinedError;
        }
        case tcam::afu420::AFU420Property::StrobeMode:
        {
            if (p_device->set_strobe(AFU420Device::strobe_parameter::mode, new_value))
            {
                return outcome::success();
            }
            return tcam::status::UndefinedError;
        }
        case tcam::afu420::AFU420Property::StrobeDelay:
        {
            if (p_device->set_strobe(AFU420Device::strobe_parameter::first_strobe_delay, new_value))
            {
                return outcome::success();
            }
            return tcam::status::UndefinedError;
        }
        case tcam::afu420::AFU420Property::StrobeDuration:
        {
            if (p_device->set_strobe(AFU420Device::strobe_parameter::first_strobe_duration,
                                     new_value))
            {
                return outcome::success();
            }
            return tcam::status::UndefinedError;
        }
        case tcam::afu420::AFU420Property::StrobeDelaySecond:
        {
            if (p_device->set_strobe(AFU420Device::strobe_parameter::second_strobe_delay,
                                     new_value))
            {
                return outcome::success();
            }
            return tcam::status::UndefinedError;
        }
        case tcam::afu420::AFU420Property::StrobeDurationSecond:
        {
            if (p_device->set_strobe(AFU420Device::strobe_parameter::second_strobe_duration,
                                     new_value))
            {
                return outcome::success();
            }
            return tcam::status::UndefinedError;
        }
        case tcam::afu420::AFU420Property::StrobePolarity:
        {
            if (p_device->set_strobe(AFU420Device::strobe_parameter::polarity, new_value))
            {
                return outcome::success();
            }
            return tcam::status::UndefinedError;
        }
        case tcam::afu420::AFU420Property::OISMode:
        {
            if (p_device->set_ois_mode(new_value))
            {
                return outcome::success();
            }
            return tcam::status::UndefinedError;
        }
        case tcam::afu420::AFU420Property::OISPosX:
        {
            p_device->m_ois_pos_x = new_value;

            if (p_device->set_ois_pos(p_device->m_ois_pos_x, p_device->m_ois_pos_y))
            {
                return outcome::success();
            }
            return tcam::status::UndefinedError;
        }
        case tcam::afu420::AFU420Property::OISPosY:
        {
            p_device->m_ois_pos_y = new_value;

            if (p_device->set_ois_pos(p_device->m_ois_pos_x, p_device->m_ois_pos_y))
            {
                return outcome::success();
            }
            return tcam::status::UndefinedError;
        }
        case tcam::afu420::AFU420Property::BinningHorizontal:
        {
            p_device->m_binning.width = new_value;
            return outcome::success();
        }
        case tcam::afu420::AFU420Property::BinningVertical:
        {
            p_device->m_binning.height = new_value;
            return outcome::success();
        }
        case tcam::afu420::AFU420Property::OffsetAuto:
        {
            p_device->m_offset_auto = new_value;
            return outcome::success();
        }
        case tcam::afu420::AFU420Property::OffsetX:
        {
            p_device->m_offset.width = new_value;
            return outcome::success();
        }
        case tcam::afu420::AFU420Property::OffsetY:
        {
            p_device->m_offset.height = new_value;
            return outcome::success();
        }
        default:
        {
            return tcam::status::PropertyDoesNotExist;
        }
    }
}


outcome::result<bool> AFU420DeviceBackend::get_bool(tcam::afu420::AFU420Property id)
{
    switch (id)
    {
        case tcam::afu420::AFU420Property::Shutter:
        {
            return p_device->get_shutter();
        }
        default:
        {
            return tcam::status::PropertyDoesNotExist;
        }
    }
}


outcome::result<void> AFU420DeviceBackend::set_bool(tcam::afu420::AFU420Property id, bool new_value)
{
    switch (id)
    {
        case tcam::afu420::AFU420Property::Shutter:
        {
            if (p_device->set_shutter(new_value))
            {
                return outcome::success();
            }
            return tcam::status::UndefinedError;
        }
        default:
        {
            return tcam::status::PropertyDoesNotExist;
        }
    }
}


} // namespace tcam::property
