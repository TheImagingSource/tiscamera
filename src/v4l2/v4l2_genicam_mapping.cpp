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

#include "v4l2_property_impl.h"

#include <cmath>
#include <tcamprop1.0_base/tcamprop_property_info_list.h>

using namespace tcam;

namespace tcam::v4l2
{
struct v4l2_genicam_mapping
{
    constexpr v4l2_genicam_mapping(uint32_t id, mapping_type type) noexcept
        : v4l2_id(id), mapping_type_(type)
    {
    }

    template<class Tprop_static_info>
    constexpr v4l2_genicam_mapping(uint32_t id,
                                   const Tprop_static_info* info_type,
                                   uint32_t preferred_id = 0)
        : v4l2_id(id), preferred_id_(preferred_id), info_(info_type),
          info_property_type_(Tprop_static_info::property_type)
    {
    }
    template<class Tprop_static_info>
    constexpr v4l2_genicam_mapping(uint32_t id,
                                   const Tprop_static_info* info_type,
                                   mapping_type map_type)
        : v4l2_id(id), preferred_id_(0), info_(info_type),
          info_property_type_(Tprop_static_info::property_type), mapping_type_(map_type)
    {
    }

    template<class Tprop_static_info>
    constexpr v4l2_genicam_mapping(uint32_t id,
                                   const Tprop_static_info* info_type,
                                   converter_scale converter,
                                   uint32_t preferred_id = 0)
        : v4l2_id(id), preferred_id_(preferred_id), info_(info_type),
          info_property_type_(Tprop_static_info::property_type), converter_ { converter }
    {
    }

    constexpr v4l2_genicam_mapping(uint32_t id,
                                   const tcamprop1::prop_static_info_enumeration* info_type,
                                   fetch_menu_entries_func func,
                                   uint32_t preferred_id = 0)
        : v4l2_id(id), preferred_id_(preferred_id), info_(info_type),
          info_property_type_(tcamprop1::prop_type::Enumeration), fetch_menu_entries_(func)
    {
    }

    uint32_t v4l2_id = 0; // only used for lookup
    uint32_t preferred_id_ =
        0; // when a control with this id is present, use that instead of this one

    tcamprop1::prop_static_info const* info_ = nullptr;
    tcamprop1::prop_type info_property_type_ = tcamprop1::prop_type::Boolean;

    tcam::v4l2::converter_scale
        converter_ = {}; // only valid for tcamprop1::prop_type::Integer and tcamprop1::prop_type::Float
    fetch_menu_entries_func fetch_menu_entries_ =
        nullptr; // only valid for tcamprop1::prop_type::Enumeration

    mapping_type mapping_type_ = mapping_type::normal;
};
} // namespace tcam::v4l2

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
    return std::llround(value * 100.0);
}

static const converter_scale gamma_converter = { gamma_from_std_to_dev, gamma_from_dev_to_std };


// this function and it sibling exist for
// for the dfk 72. It's color channels have a range from 0-63
int64_t wb_channel_from_std_to_dev(double value)
{
    const double std_max = 4.0;
    const int dev_max = 63;

    return std::llround(value * dev_max / std_max);
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
    return std::llround(value / 100);
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

    return std::llround(value * dev_max / std_max);
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

menu_entry_list fetch_menu_entries_off_once()
{
    return convert_to_menu_entry_list(prop_lst::enum_entries_off_once);
}

menu_entry_list fetch_menu_entries_StrobePolarity()
{
    return convert_to_menu_entry_list(prop_lst::StrobePolarity_enum_entries);
}

menu_entry_list fetch_menu_entries_TriggerOperation()
{
    return convert_to_menu_entry_list(prop_lst::TriggerOperation_enum_entries);
}

menu_entry_list fetch_menu_entries_TriggerSelector()
{
    return convert_to_menu_entry_list(prop_lst::TriggerSelector_enum_entries);
}

// clang-format off
static const tcam::v4l2::v4l2_genicam_mapping v4l2_conv_dict[] = {
    { 0x00980900 /*V4L2_CID_BRIGHTNESS*/,       &prop_lst::BlackLevel },
    { 0x00980901 /*V4L2_CID_CONTRAST*/,         &prop_lst::Contrast },
    { 0x00980902 /*V4L2_CID_SATURATION*/,       &prop_lst::Saturation, saturation_converter },
    { 0x00980903 /*V4L2_CID_HUE*/,              &prop_lst::Hue },
    { V4L2_CID_AUTO_WHITE_BALANCE,              &prop_lst::BalanceWhiteAuto, fetch_menu_entries_off_continuous },
    { 0x0098090e /*V4L2_CID_RED_BALANCE*/,      &prop_lst::BalanceWhiteRed, wb_256_channel_converter },
    { 0x0098090f /*V4L2_CID_BLUE_BALANCE*/,     &prop_lst::BalanceWhiteBlue, wb_256_channel_converter },
    { 0x00980910 /*V4L2_CID_GAMMA*/,            &prop_lst::Gamma, gamma_converter },
    { 0x00980913 /*V4L2_CID_GAIN*/,             &prop_lst::Gain, 0x0199e204 },

    { 0x009a0901 /*V4L2_CID_EXPOSURE_AUTO*/,    &prop_lst::ExposureAuto, fetch_menu_entries_v4l2_ExposureAuto, 0x0199e203 },
    { 0x009a0902 /*V4L2_CID_EXPOSURE_ABSOLUTE*/,&prop_lst::ExposureTime, exposure_absolute_converter, 0x199e201 },
    { 0x009a090a /*V4L2_CID_FOCUS_ABSOLUTE*/,   &prop_lst::Focus },
    { V4L2_CID_ZOOM_ABSOLUTE,                   &prop_lst::Zoom },
    { 0x009a0910 /*V4L2_CID_PRIVACY*/,          &prop_lst::TriggerMode, fetch_menu_entries_off_on },

    { 0x0199e201, &prop_lst::ExposureTime },                                        // usb23 usb33 "Exposure Time (us)"
    { 0x0199e202, &prop_lst::ExposureAuto, fetch_menu_entries_off_continuous },     // usb23 usb33 "Auto Shutter"
    { 0x0199e203, &prop_lst::ExposureAutoReference },                               // usb23 usb33 
    { 0x0199e204, &prop_lst::Gain, gain_converter },                                // usb23 "Gain (dB/100)"
    { 0x0199e205, &prop_lst::GainAuto, fetch_menu_entries_off_continuous },         // usb23 usb33
    { V4L2_CID_TIS_WHITEBALANCE_ONE_PUSH, mapping_type::internal },                 // usb23 usb33 "White Balance One Push"
    { 0x0199e207, &prop_lst::BalanceWhiteAutoPreset },                              // usb23 usb33 "White Balance Preset"
    { 0x0199e208, &prop_lst::TriggerMode, fetch_menu_entries_off_on, 0x009a0910 },  // usb2 usb23 usb33 "Trigger Mode"
    { 0x0199e209, &prop_lst::TriggerSoftware },                                     // usb2 usb23 usb33
    { 0x0199e210, &prop_lst::TriggerDelay, 0x199e272 },                             // usb23 
    { 0x0199e211, &prop_lst::StrobeEnable, fetch_menu_entries_off_on },             // usb2 usb23 usb33
    { 0x0199e212, &prop_lst::StrobePolarity, fetch_menu_entries_StrobePolarity },   // usb2 usb23 usb33
    { 0x0199e213, &prop_lst::StrobeOperation },                                     // usb23 usb33 "Strobe Exposure"/"Strobe Operation"
    { 0x0199e214, &prop_lst::StrobeDuration },                                      // usb23 usb33
    { 0x0199e215, &prop_lst::StrobeDelay },                                         // usb23 usb33
    { 0x0199e216, &prop_lst::GPOut },                                               // usb23 usb33 "GPOUT" boolean
    { 0x0199e217, &prop_lst::GPIn },                                                // usb23 usb33 "GPIN" boolean
    { 0x0199e218, &prop_lst::OffsetX },                                             // usb23 usb33 "ROI Offset X"
    { 0x0199e219, &prop_lst::OffsetY },                                             // usb23 usb33 "ROI Offset Y"
    { 0x0199e220, &prop_lst::OffsetAutoCenter, fetch_menu_entries_off_on },         // usb23 usb33 "ROI Auto Center",

    { 0x0199e221, &prop_lst::FocusAuto, fetch_menu_entries_off_once },              // usb23 usb33 "Auto Focus One Push",
    { 0x0199e222, &prop_lst::AutoFocusROIEnable },                                  // usb23 usb33 "Auto Focus ROI Enable",
    { 0x0199e223, &prop_lst::AutoFocusROILeft },                                    // usb23 usb33 "Auto Focus ROI Left",
    { 0x0199e224, &prop_lst::AutoFocusROITop },                                     // usb23 usb33 "Auto Focus ROI Top",
#if 0   // most likely never used
    { 0x0199e225, mapping_type::internal },         // usb23 "ATR Enable"
    { 0x0199e226, mapping_type::internal },         // usb23 "ATR Gain"
    { 0x0199e227, mapping_type::internal },         // usb23 "ATR Enable Wide D"
    { 0x0199e228, mapping_type::internal },         // usb23 "ATR Contrast"
    { 0x0199e229, mapping_type::internal },         // usb23 "ATR Contrast Auto"
    { 0x0199e230, mapping_type::internal },         // usb23 "ATR Chroma"

    { 0x0199e231, mapping_type::internal },         // usb23 "Image Stabilization"
    { 0x0199e232, mapping_type::internal },         // usb23 "Noise Reduction"
    { 0x0199e233, mapping_type::internal },         // usb23 "Face Detection"
#endif

    { 0x0199e234, &prop_lst::TriggerActivation },   // usb23 usb33 "Trigger Polarity"
    { 0x0199e235, &prop_lst::TriggerOperation, fetch_menu_entries_TriggerOperation },   // usb23 usb33
    { 0x0199e236, &prop_lst::TriggerSelector, fetch_menu_entries_TriggerSelector },     // usb23 usb33 
    { 0x0199e237, &prop_lst::TriggerBurstCount },   // usb23 usb33 "Trigger Burst Count",
    { 0x0199e238, &prop_lst::TriggerDebouncer },    // usb23 usb33 "Trigger Debounce Time (us)",
    { 0x0199e239, &prop_lst::TriggerMask },         // usb23 usb33 "Trigger Mask Time (us)",
    // 0x0199e23a - 0x0199e23f not set
    { 0x0199e240, &prop_lst::TriggerDenoise },              // usb23 usb33 "Trigger Noise Suppression Time"
    { 0x0199e241, &prop_lst::AutoFunctionsROIEnable },      // usb23 usb33 "Auto Functions ROI Control",
    { 0x0199e242, &prop_lst::AutoFunctionsROILeft },        // usb23 usb33
    { 0x0199e243, &prop_lst::AutoFunctionsROITop },         // usb23 usb33
    { 0x0199e244, &prop_lst::AutoFunctionsROIWidth },       // usb23 usb33
    { 0x0199e245, &prop_lst::AutoFunctionsROIHeight },      // usb23 usb33
    { 0x0199e246, &prop_lst::BalanceWhiteMode, mapping_type::blacklist },               // usb23 usb33 "White Balance Mode"
    { 0x0199e247, &prop_lst::BalanceWhiteAutoPreset, mapping_type::blacklist },         // usb23 usb33 "White Balance Auto Preset"
    { 0x0199e248, &prop_lst::BalanceWhiteGreen, wb_256_channel_converter },             // usb23 usb33 integer V4L2_WHITE_BALANCE_GREEN
    { 0x0199e249, &prop_lst::BalanceWhiteTemperaturePreset, mapping_type::blacklist },  // usb23 "White Balance Temp. Preset"
    // 0x0199e24a - 0x0199e24f not set
    { 0x0199e250, &prop_lst::BalanceWhiteTemperature, mapping_type::blacklist },        // usb23 usb33 "White Balance Temperature",
    { 0x0199e251, &prop_lst::ReverseY },                                                // usb23 usb33 "Flip Horizontal",
    { 0x0199e252, &prop_lst::ReverseX },                                                // usb23 usb33 "Flip Vertical"
    { 0x0199e253, &prop_lst::ExposureAutoHighlightReduction },                          // usb23 usb33 "Highlight Reduction"
    { 0x0199e254, &prop_lst::ExposureAutoUpperLimitAuto, fetch_menu_entries_off_on },   // usb23 usb33
    { 0x0199e255, &prop_lst::ExposureAutoLowerLimit },                                  // usb23 usb33 "Exposure Auto Lower Limit (us)"
    { 0x0199e256, &prop_lst::ExposureAutoUpperLimit },                                  // usb23 usb33 "Exposure Auto Upper Limit (us)"
    { 0x0199e257, mapping_type::internal },                                             // usb23 usb33 "Override Scanning Mode" integer
    { 0x0199e258, &prop_lst::AutoFunctionsROIPreset },                                  // usb23 usb33 "Auto Functions ROI Preset",
    { 0x0199e259, &prop_lst::GainAutoLowerLimit },                                      // usb23 usb33 "Gain Auto Lower Limit"
    // 0x0199e25a - 0x0199e25f not set
    { 0x0199e260, &prop_lst::GainAutoUpperLimit },                                      // usb23 usb33 "Gain Auto Upper Limit"
    { 0x0199e261, &prop_lst::TriggerOperation, fetch_menu_entries_TriggerOperation },   // usb23 usb33 "Trigger Global Reset Release" boolean
    { 0x0199e262, &prop_lst::IMXLowLatencyTriggerMode },                                // usb33

    { 0x199e263, mapping_type::internal },                                          // usb23 usb33 Scanning Mode Selector
    { 0x199e264, mapping_type::internal },                                          // usb23 usb33 Scanning Mode Identifier
    { 0x199e265, mapping_type::internal },                                          // usb23 usb33 Scanning Mode Scale Horizontal
    { 0x199e266, mapping_type::internal },                                          // usb23 usb33 Scanning Mode Scale Vertical
    { 0x199e267, mapping_type::internal },                                          // usb23 usb33 Scanning Mode Binning H
    { 0x199e268, mapping_type::internal },                                          // usb23 usb33 Scanning Mode Binning V
    { 0x199e269, mapping_type::internal },                                          // usb23 usb33 Scanning Mode Skipping H
    { 0x199e270, mapping_type::internal },                                          // usb23 usb33 Scanning Mode Skipping V
    { 0x199e271, mapping_type::internal },                                          // usb23 usb33 Scanning Mode Flags

    { 0x199e272, &prop_lst::TriggerDelay, trigger_delay_100ns_converter },          // usb33 "Trigger Delay (100ns)"

    { 0x0199e920, mapping_type::internal },                                         // usb2 ExtIO
    // { 0x0199e921, &prop_lst::BalanceWhiteRed, wb_for_DFK72_channel_converter },     // usb2 "GainR"
    // { 0x0199e922, &prop_lst::BalanceWhiteGreen, wb_for_DFK72_channel_converter },   // usb2 "GainG"
    // { 0x0199e923, &prop_lst::BalanceWhiteBlue, wb_for_DFK72_channel_converter },    // usb2 "GainB"
    { 0x0199e921, mapping_type::blacklist },     // usb2 "GainR"
    { 0x0199e922, mapping_type::blacklist },   // usb2 "GainG"
    { 0x0199e923, mapping_type::blacklist },    // usb2 "GainB"
    // 0x0199e924 ?
    { 0x0199e925, mapping_type::internal },                                         // usb2 "Binning"
    // 0x0199e926 ?
    { 0x0199e927, &prop_lst::OffsetX },                                                 // usb2 "X Offset"
    { 0x0199e928, &prop_lst::OffsetY },                                                 // usb2 "Y Offset"
    { 0x0199e929, mapping_type::internal },                                             // usb2 "Skipping"
    { 0x0199e92a, &prop_lst::TriggerOperation, fetch_menu_entries_TriggerOperation },   // usb2 "Trigger Global Reset Shutter"

};

// clang-format on

static const tcam::v4l2::v4l2_genicam_mapping* find_mapping(uint32_t v4l2_id)
{
    for (const auto& entry : v4l2_conv_dict)
    {
        if (entry.v4l2_id == v4l2_id)
        {
            return &entry;
        }
    }
    return nullptr;
}

} // namespace


tcam::v4l2::v4l2_genicam_mapping_info tcam::v4l2::find_mapping_info(uint32_t v4l2_id)
{
    auto ptr = find_mapping(v4l2_id);
    if (!ptr)
    {
        return {};
    }
    if (ptr->info_)
    {
        return v4l2_genicam_mapping_info { ptr->preferred_id_, ptr->mapping_type_, ptr };
    }
    else
    {
        return v4l2_genicam_mapping_info { ptr->preferred_id_, ptr->mapping_type_, nullptr };
    }
}

auto tcam::v4l2::create_mapped_prop(
    const std::vector<v4l2_queryctrl>& device_qctrl_list,
    const v4l2_queryctrl& current_prop_qctrl,
    const v4l2_genicam_mapping& mapping,
    const std::shared_ptr<tcam::v4l2::V4L2PropertyBackend>& p_property_backend)
    -> std::shared_ptr<tcam::property::IPropertyBase>
{
    using namespace tcam::property;

    if (current_prop_qctrl.id == V4L2_CID_AUTO_WHITE_BALANCE)
    {
        if (is_id_present(device_qctrl_list, V4L2_CID_TIS_WHITEBALANCE_ONE_PUSH))
        {
            // replace BalanceWhiteAuto with our special enum control when 0x0199e206/V4L2_CID_TIS_WHITEBALANCE_ONE_PUSH is also present
            return std::make_shared<prop_impl_33U_balance_white_auto>(current_prop_qctrl,
                                                                      p_property_backend);
        }
    }

    switch (mapping.info_property_type_)
    {
        case tcamprop1::prop_type::Boolean:
        {
            return std::make_shared<V4L2PropertyBoolImpl>(
                current_prop_qctrl,
                p_property_backend,
                static_cast<const tcamprop1::prop_static_info_boolean*>(mapping.info_));
        }
        case tcamprop1::prop_type::Integer:
        {
            return std::make_shared<V4L2PropertyIntegerImpl>(
                current_prop_qctrl,
                p_property_backend,
                static_cast<const tcamprop1::prop_static_info_integer*>(mapping.info_),
                mapping.converter_);
        }
        case tcamprop1::prop_type::Float:
        {
            return std::make_shared<V4L2PropertyDoubleImpl>(
                current_prop_qctrl,
                p_property_backend,
                static_cast<const tcamprop1::prop_static_info_float*>(mapping.info_),
                mapping.converter_);
        }
        case tcamprop1::prop_type::Command:
        {
            return std::make_shared<V4L2PropertyCommandImpl>(
                current_prop_qctrl,
                p_property_backend,
                static_cast<const tcamprop1::prop_static_info_command*>(mapping.info_));
        }
        case tcamprop1::prop_type::Enumeration:
        {
            return std::make_shared<V4L2PropertyEnumImpl>(
                current_prop_qctrl,
                p_property_backend,
                static_cast<const tcamprop1::prop_static_info_enumeration*>(mapping.info_),
                mapping.fetch_menu_entries_);
        }
    }
    return nullptr;
}