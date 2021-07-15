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

#include "v4l2_genicam_conversion.h"

namespace
{

class ConverterGamma : public tcam::v4l2::ConverterIntToDouble
{
public:
    virtual double to_double(int64_t value) final
    {
        return value / m_conversion_factor;
    };

    virtual int64_t to_int(double value) final
    {
        return value * m_conversion_factor;
    };

private:
    double m_conversion_factor = 100;
};


// this function and it sibling exist for
// for the dfk 72. It's color channels have a range from 0-63
double wb_channel_from_std_to_dev(double value)
{
    const int std_max = 255;
    const int dev_max = 63;

    return value * dev_max / std_max;
}

double wb_channel_from_dev_to_std(double value)
{
    const int std_max = 255;
    const int dev_max = 63;

    return value * std_max / dev_max;
}



} // namespace


namespace tcam::v4l2
{

std::shared_ptr<ConverterIntToDouble> find_int_to_double(uint32_t v4l2_id)
{
    switch (v4l2_id)
    {
        case 0x00980910: // Gamma
        {
            return std::make_shared<ConverterGamma>();
        }
        default:
        {
            return nullptr;
        }
    }
}

converter_scale find_scale(uint32_t v4l2_id)
// std::shared_ptr<ConverterScale> find_scale(uint32_t v4l2_id)
{
    switch (v4l2_id)
    {
        // BalanceWhite{Red,Green,Blue}
        case 0x0199e921:
        case 0x0199e922:
        case 0x0199e923:
        {
            return {*wb_channel_from_std_to_dev, *wb_channel_from_dev_to_std};
        }
        default:
        {
            return {nullptr, nullptr};
        }
    }
}


outcome::result<std::map<int, std::string>> find_menu_entries(uint32_t v4l2_id)
{
    switch (v4l2_id)
    {
        case 0x009a0901: // exposure auto
        {
            return std::map<int, std::string> { { 1, "Off" }, { 3, "Continuous" } };
        }
        case 0x0098090c: // white balance
        case 0x199e202: // auto shutter
        case 0x199e205: // gain auto
        {
            return std::map<int, std::string> { { 0, "Off" }, { 1, "Continuous" } };
        }
        case 0x009a0910: // TriggerMode
        case 0x199e208: // TriggerMode
        case 0x0199e254: // ExposureAutoUpperLimitAuto
        {
            return std::map<int, std::string> { { 0, "Off" }, { 1, "On" } };
        }
        default:
        {
            return tcam::status::PropertyDoesNotExist;
        }
    }
}


} // namespace tcam::v4l2
