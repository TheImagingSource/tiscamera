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

#include "v4l2_genicam_mapping.h"

#include <tcamprop1.0_base/tcamprop_property_info_list.h>
#include <linux/v4l2-controls.h>

#include <cmath>

using namespace tcam;

namespace
{
namespace prop_lst = tcamprop1::prop_list;
using namespace tcam::v4l2;

double gamma_from_dev_to_std(int64_t value)
{
    return value / 100.0;
}

int64_t gamma_from_std_to_dev(double value)
{
    return std::llround( value * 100.0 );
}

static const converter_scale gamma_converter = { gamma_from_std_to_dev, gamma_from_dev_to_std };


// this function and it sibling exist for
// for the dfk 72. It's color channels have a range from 0-63
int64_t wb_channel_from_std_to_dev(double value)
{
    const double std_max = 4.0;
    const int dev_max = 63;

    return std::llround( value * dev_max / std_max);
}

double wb_channel_from_dev_to_std(int64_t value)
{
    const double std_max = 4.0;
    const int dev_max = 63;

    return value * std_max / dev_max;
}

static const converter_scale wb_for_DFK72_channel_converter = { wb_channel_from_std_to_dev,
                                                      wb_channel_from_dev_to_std };

int64_t exposure_absolute_from_std_to_dev(double value)
{
    return std::llround( value / 100 );
}

double exposure_absolute_from_dev_to_std(int64_t value)
{
    return value * 100.;
}

static const converter_scale exposure_absolute_converter = { exposure_absolute_from_std_to_dev,
                                                             exposure_absolute_from_dev_to_std };


int64_t wb_256_channel_from_std_to_dev(double value)
{
    const double std_max = 4.0;
    const int dev_max = 256;

    return std::llround( value * dev_max / std_max);
}

double wb_256_channel_from_dev_to_std(int64_t value)
{
    const double std_max = 4.0;
    const int dev_max = 256;

    return value * std_max / dev_max;
}

static const converter_scale wb_256_channel_converter = { wb_256_channel_from_std_to_dev,
                                                          wb_256_channel_from_dev_to_std };

static const converter_scale saturation_converter = { [](double v) -> int64_t
                                                      { return std::lround(v / 100. * 64.); },
                                                      [](int64_t v) -> double
                                                      {
                                                          return v / 64. * 100.;
                                                      } };
static const converter_scale gain_converter = { [](double v) -> int64_t
                                                { return std::lround(v * 100.); },
                                                [](int64_t v) -> double
                                                {
                                                    return v / 100.;
                                                } };
static const converter_scale trigger_delay_100ns_converter = { [](double v) -> int64_t
                                                { return std::lround(v * 10.); },
                                                [](int64_t v) -> double
                                                {
                                                    return v / 10.;
                                                } };

auto fetch_menu_entries_v4l2_ExposureAuto()
{
    return menu_entry_list { { 1, "Off" }, { 3, "Continuous" } };
}

auto fetch_menu_entries_off_continuous()
{
    return menu_entry_list { { 0, "Off" }, { 1, "Continuous" } };
}
auto fetch_menu_entries_off_on()
{
    return menu_entry_list { { 0, "Off" }, { 1, "On" } };
}

template<class T> auto convert_to_menu_entry_list(const T& arr)
{
    menu_entry_list rval;
    rval.reserve(arr.size());
    for (size_t i = 0; i < arr.size(); ++i)
    {
        rval.push_back(menu_entry { static_cast<int>(i), std::string { arr[i] } });
    }
    return rval;
}

menu_entry_list fetch_menu_entries_StrobePolarity()
{
    return convert_to_menu_entry_list(prop_lst::StrobePolarity_enum_entries);
}

menu_entry_list fetch_menu_entries_TriggerOperation()
{
    return convert_to_menu_entry_list(prop_lst::TriggerOperation_enum_entries);
}

// clang-format off
static const tcam::v4l2::v4l2_genicam_mapping v4l2_conv_dict[] = {
    { 0x00980900 /*V4L2_CID_BRIGHTNESS*/,       &prop_lst::BlackLevel },
    { 0x00980901 /*V4L2_CID_CONTRAST*/,         &prop_lst::Contrast },
    { 0x00980902 /*V4L2_CID_SATURATION*/,       &prop_lst::Saturation, saturation_converter },
    { 0x00980903 /*V4L2_CID_HUE*/,              &prop_lst::Hue },
    { 0x0098090c /*V4L2_CID_AUTO_WHITE_BALANCE*/,&prop_lst::BalanceWhiteAuto, fetch_menu_entries_off_continuous },
    { 0x0098090e /*V4L2_CID_RED_BALANCE*/,      &prop_lst::BalanceWhiteRed, wb_256_channel_converter },
    { 0x0098090f /*V4L2_CID_BLUE_BALANCE*/,     &prop_lst::BalanceWhiteBlue, wb_256_channel_converter },
    { 0x00980910 /*V4L2_CID_GAMMA*/,            &prop_lst::Gamma, gamma_converter },
    { 0x00980913 /*V4L2_CID_GAIN*/,             &prop_lst::Gain, 0x0199e204 },

    { 0x009a0901 /*V4L2_CID_EXPOSURE_AUTO*/,    &prop_lst::ExposureAuto, fetch_menu_entries_v4l2_ExposureAuto, 0x0199e203 },
    { 0x009a0902 /*V4L2_CID_EXPOSURE_ABSOLUTE*/,&prop_lst::ExposureTime, exposure_absolute_converter, 0x199e201 },
    { 0x009a090a /*V4L2_CID_FOCUS_ABSOLUTE*/,   &prop_lst::Focus },
    { V4L2_CID_ZOOM_ABSOLUTE,                   &prop_lst::Zoom },
    { 0x009a0910 /*V4L2_CID_PRIVACY*/,          &prop_lst::TriggerMode, fetch_menu_entries_off_on },

    { 0x0199e201, &prop_lst::ExposureTime },
    { 0x0199e202, &prop_lst::ExposureAuto, fetch_menu_entries_off_continuous },
    { 0x0199e203, &prop_lst::ExposureAutoReference },
    { 0x0199e204, &prop_lst::Gain, gain_converter },
    { 0x0199e205, &prop_lst::GainAuto, fetch_menu_entries_off_continuous },
    { 0x0199e206, mapping_type::internal }, // White Balance One Push
    { 0x0199e207, &prop_lst::BalanceWhiteAutoPreset },
    { 0x0199e208, &prop_lst::TriggerMode, fetch_menu_entries_off_on, 0x009a0910 },
    { 0x0199e209, &prop_lst::TriggerSoftware },
    { 0x0199e210, &prop_lst::TriggerDelay, 0x199e272 },
    { 0x0199e211, &prop_lst::StrobeEnable, fetch_menu_entries_off_on },
    { 0x0199e212, &prop_lst::StrobePolarity, fetch_menu_entries_StrobePolarity },
    { 0x0199e213, &prop_lst::StrobeOperation },
    { 0x0199e214, &prop_lst::StrobeDuration },
    { 0x0199e215, &prop_lst::StrobeDelay },
    { 0x0199e216, &prop_lst::GPOut },
    { 0x0199e217, &prop_lst::GPIn },
    { 0x0199e218, &prop_lst::OffsetX },
    { 0x0199e219, &prop_lst::OffsetY },
    { 0x0199e220, &prop_lst::OffsetAutoCenter, fetch_menu_entries_off_on },
    { 0x0199e921, &prop_lst::BalanceWhiteRed, wb_for_DFK72_channel_converter },
    { 0x0199e922, &prop_lst::BalanceWhiteGreen, wb_for_DFK72_channel_converter },
    { 0x0199e923, &prop_lst::BalanceWhiteBlue, wb_for_DFK72_channel_converter },
    // 0x0199e924 ?
    { 0x0199e925, mapping_type::internal }, // Binning
    // 0x0199e926 ?
    { 0x0199e927, &prop_lst::OffsetX },
    { 0x0199e928, &prop_lst::OffsetY },
    { 0x0199e929, mapping_type::internal }, // V4L2_CID_SKIP boolean
    // 0x0199e92a V4L2_CID_GLOBAL_RESET_SHUTTER bool
    { 0x0199e234, &prop_lst::TriggerActivation },
    { 0x0199e235, &prop_lst::TriggerOperation },
    { 0x0199e236, &prop_lst::TriggerSelector },
    { 0x0199e237, &prop_lst::TriggerBurstCount },
    { 0x0199e238, &prop_lst::TriggerDebouncer },
    { 0x0199e239, &prop_lst::TriggerMask },
    // 0x0199e23a - 0x0199e23f not set
    { 0x0199e240, &prop_lst::TriggerDenoise },
    { 0x0199e241, &prop_lst::AutoFunctionsROIEnable },
    { 0x0199e242, &prop_lst::AutoFunctionsROILeft },
    { 0x0199e243, &prop_lst::AutoFunctionsROITop },
    { 0x0199e244, &prop_lst::AutoFunctionsROIWidth },
    { 0x0199e245, &prop_lst::AutoFunctionsROIHeight },
    { 0x0199e246, &prop_lst::BalanceWhiteMode }, // usb3.json enumeration V4L2_WHITE_BALANCE_MODE
    { 0x0199e247, &prop_lst::BalanceWhiteAutoPreset },
    { 0x0199e248, &prop_lst::BalanceWhiteGreen, wb_256_channel_converter }, // usb3.json integer V4L2_WHITE_BALANCE_GREEN
    { 0x0199e249, &prop_lst::BalanceWhiteTemperaturePreset },
    // 0x0199e24a - 0x0199e24f not set
    { 0x0199e250, &prop_lst::BalanceWhiteTemperature },
    { 0x0199e251, &prop_lst::ReverseY },
    { 0x0199e252, &prop_lst::ReverseX },
    { 0x0199e253, &prop_lst::ExposureAutoHighlightReduction },
    { 0x0199e254, &prop_lst::ExposureAutoUpperLimitAuto, fetch_menu_entries_off_on },
    { 0x0199e255, &prop_lst::ExposureAutoLowerLimit },
    { 0x0199e256, &prop_lst::ExposureAutoUpperLimit },
    { 0x0199e257, mapping_type::internal }, // usb3.xml "Override Scanning Mode" integer
    { 0x0199e258, &prop_lst::AutoFunctionsROIPreset },
    { 0x0199e259, &prop_lst::GainAutoLowerLimit },
    // 0x0199e25a - 0x0199e25f not set
    { 0x0199e260, &prop_lst::GainAutoUpperLimit },
    { 0x0199e261, &prop_lst::TriggerOperation, fetch_menu_entries_TriggerOperation },   // "Trigger Global Reset Release" boolean
    { 0x0199e262, &prop_lst::IMXLowLatencyTriggerMode },

    { 0x199e263, mapping_type::internal }, // Scanning Mode Selector
    { 0x199e264, mapping_type::internal }, // Scanning Mode Identifier
    { 0x199e265, mapping_type::internal }, // Scanning Mode Scale Horizontal
    { 0x199e266, mapping_type::internal }, // Scanning Mode Scale Vertical
    { 0x199e267, mapping_type::internal }, // Scanning Mode Binning H
    { 0x199e268, mapping_type::internal }, // Scanning Mode Binning V
    { 0x199e269, mapping_type::internal }, // Scanning Mode Skipping H
    { 0x199e270, mapping_type::internal }, // Scanning Mode Skipping V
    { 0x199e271, mapping_type::internal }, // Scanning Mode Flags

    { 0x199e272, &prop_lst::TriggerDelay, trigger_delay_100ns_converter },  // Trigger Delay (100ns)
};

// clang-format on


} // namespace


const struct tcam::v4l2::v4l2_genicam_mapping* tcam::v4l2::find_mapping(uint32_t v4l2_id)
{
    for (unsigned int i = 0; i < sizeof(v4l2_conv_dict) / sizeof(v4l2_genicam_mapping); i++)
    {
        if (v4l2_conv_dict[i].v4l2_id == v4l2_id)
        {
            return &v4l2_conv_dict[i];
        }
    }
    return nullptr;
}
