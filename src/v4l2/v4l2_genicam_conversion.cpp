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

double gamma_from_dev_to_std(double value)
{
    return value / 100;
}


double gamma_from_std_to_dev(double value)
{
    return value * 100;
}

// this function and it sibling exist for
// for the dfk 72. It's color channels have a range from 0-63
double wb_channel_from_std_to_dev(double value)
{
    const double std_max = 4.0;
    const int dev_max = 63;

    return value * dev_max / std_max;
}

double wb_channel_from_dev_to_std(double value)
{
    const double std_max = 4.0;
    const int dev_max = 63;

    return value * std_max / dev_max;
}


double exposure_absolute_from_std_to_dev(double value)
{
    return value / 100;
}

double exposure_absolute_from_dev_to_std(double value)
{
    return value * 100;
}


double wb_256_channel_from_std_to_dev(double value)
{
    const double std_max = 4.0;
    const int dev_max = 256;

    return value * dev_max / std_max;
}


double wb_256_channel_from_dev_to_std(double value)
{
    const double std_max = 4.0;
    const int dev_max = 256;

    return value * std_max / dev_max;
}


} // namespace


namespace tcam::v4l2
{

converter_scale find_scale(uint32_t v4l2_id)
// std::shared_ptr<ConverterScale> find_scale(uint32_t v4l2_id)
{
    switch (v4l2_id)
    {
        case 0x009a0902: // exposure_absolute
        {
            return {*exposure_absolute_from_std_to_dev, *exposure_absolute_from_dev_to_std};
        }
        case 0x00980910: // Gamma
        {
            return {*gamma_from_std_to_dev, *gamma_from_dev_to_std};
        }
        // BalanceWhite{Red,Green,Blue}
        case 0x0199e921:
        case 0x0199e922:
        case 0x0199e923:
        {
            return {*wb_channel_from_std_to_dev, *wb_channel_from_dev_to_std};
        }
        case 0x098090e:
        case 0x098090f:
        case 0x0199e248:
        {
            return {*wb_256_channel_from_std_to_dev, *wb_256_channel_from_dev_to_std};
        }
        default:
        {
            return {[](double val){return val;}, [](double val){return val;}};
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
        case 0x0199e208: // TriggerMode
        case 0x0199e211: // StrobeEnable
        case 0x0199e254: // ExposureAutoUpperLimitAuto
        case 0x0199e220:    // OffsetAutoCenter
        {
            return std::map<int, std::string> { { 0, "Off" }, { 1, "On" } };
        }
        case 0x0199e212: // StrobePolarity
        {
            return std::map<int, std::string> { { 0, "ActiveHigh" }, { 1, "ActiveLow" } };
        }
        case 0x0199e261: // TriggerOperation
        {
            return std::map<int, std::string> { { 0, "Default" }, { 1, "GlobalResetRelease" } };
        }
        default:
        {
            return tcam::status::PropertyDoesNotExist;
        }
    }
}


} // namespace tcam::v4l2
