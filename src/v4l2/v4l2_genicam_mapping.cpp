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

#include "../logging.h"
#include "sensor_id_33u.h"
#include "v4l2_property_impl.h"

#include <cmath>
#include <tcamprop1.0_base/tcamprop_property_info_list.h>

using namespace tcam;

//#define ENABLE_23U_SUPPORT 1

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

    constexpr v4l2_genicam_mapping(uint32_t id,
                                   const tcamprop1::prop_static_info_float* info_type,
                                   converter_scale_init_float converter,
                                   uint32_t preferred_id = 0)
        : v4l2_id(id), preferred_id_(preferred_id), info_(info_type),
          info_property_type_(tcamprop1::prop_type::Float), converter_float_ { converter }
    {
    }

    constexpr v4l2_genicam_mapping(uint32_t id,
                                   const tcamprop1::prop_static_info_integer* info_type,
                                   converter_scale_init_integer converter,
                                   uint32_t preferred_id = 0)
        : v4l2_id(id), preferred_id_(preferred_id), info_(info_type),
          info_property_type_(tcamprop1::prop_type::Integer), converter_integer_ { converter }
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

    // only valid for tcamprop1::prop_type::Integer and tcamprop1::prop_type::Float
    tcam::v4l2::converter_scale_init_float converter_float_;
    tcam::v4l2::converter_scale_init_integer converter_integer_;

    // only valid for tcamprop1::prop_type::Enumeration
    fetch_menu_entries_func fetch_menu_entries_ = nullptr;

    mapping_type mapping_type_ = mapping_type::normal;
};
} // namespace tcam::v4l2

namespace
{
namespace prop_lst = tcamprop1::prop_list;
using namespace tcam::v4l2;

static const converter_scale_init_float gamma_converter = {
    [](double value) -> int64_t { return std::llround(value * 100.0); },
    [](int64_t value) -> double { return value / 100.0; },
};

int64_t exposure_absolute_from_std_to_dev(double value)
{
    return std::llround(value / 100);
}

double exposure_absolute_from_dev_to_std(int64_t value)
{
    return value * 100.;
}

static const converter_scale_init_float exposure_absolute_converter = {
    exposure_absolute_from_std_to_dev,
    exposure_absolute_from_dev_to_std,
};

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

static const converter_scale_init_float wb_256_channel_converter = {
    wb_256_channel_from_std_to_dev,
    wb_256_channel_from_dev_to_std,
};

static const converter_scale_init_float saturation_converter = {
    [](double v) -> int64_t { return std::lround(v / 100. * 64.); },
    [](int64_t v) -> double { return v / 64. * 100.; },
};
static const converter_scale_init_float gain_db100_converter = {
    [](double v) -> int64_t { return std::lround(v * 100.); },
    [](int64_t v) -> double { return v / 100.; },
};
static const converter_scale_init_float trigger_delay_100ns_converter = {
    [](double v) -> int64_t { return std::lround(v * 10.); },
    [](int64_t v) -> double { return v / 10.; },
};

auto fetch_menu_entries_v4l2_ExposureAuto()
{
    return menu_entry_list { { 1, "Off" }, { 3, "Continuous" } };
}

static auto fetch_menu_entries_off_continuous()
{
    return menu_entry_list { { 0, "Off" }, { 1, "Continuous" } };
}
static auto fetch_menu_entries_off_on()
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

static menu_entry_list fetch_menu_entries_off_once()
{
    return convert_to_menu_entry_list(prop_lst::enum_entries_off_once);
}

static menu_entry_list fetch_menu_entries_StrobePolarity()
{
    return convert_to_menu_entry_list(prop_lst::StrobePolarity_enum_entries);
}

static menu_entry_list fetch_menu_entries_StrobeOperation()
{
    return menu_entry_list { { 0, "FixedDuration" }, { 1, "Exposure" } };
}

menu_entry_list fetch_menu_entries_TriggerOperation()
{
    return convert_to_menu_entry_list(prop_lst::TriggerOperation_enum_entries);
}

// clang-format off
static const tcam::v4l2::v4l2_genicam_mapping generic_v4l2_conv_table[] = {
    { 0x00980900 /*V4L2_CID_BRIGHTNESS*/,       &prop_lst::BlackLevel },
    { 0x00980901 /*V4L2_CID_CONTRAST*/,         &prop_lst::Contrast },
    { 0x00980902 /*V4L2_CID_SATURATION*/,       &prop_lst::Saturation, saturation_converter },
    { 0x00980903 /*V4L2_CID_HUE*/,              &prop_lst::Hue },
    { 0x00980910 /*V4L2_CID_GAMMA*/,            &prop_lst::Gamma, gamma_converter },
    { 0x0098091b /*V4L2_CID_SHARPNESS*/,        &prop_lst::Sharpness},

    { 0x009a0902 /*V4L2_CID_EXPOSURE_ABSOLUTE*/,&prop_lst::ExposureTime, exposure_absolute_converter, 0x199e201 },
    { 0x199e201, &prop_lst::ExposureTime },                                        // usb23 usb33 "Exposure Time (us)"
    { 0x009a0901 /*V4L2_CID_EXPOSURE_AUTO*/,    &prop_lst::ExposureAuto, fetch_menu_entries_v4l2_ExposureAuto, 0x199e202 },
    { 0x199e202, &prop_lst::ExposureAuto, fetch_menu_entries_off_continuous },     // usb23 usb33 "Auto Shutter"
    { 0x199e203, &prop_lst::ExposureAutoReference },                               // usb23 usb33
    { 0x199e255, &prop_lst::ExposureAutoLowerLimit },                              // usb23 usb33 "Exposure Auto Lower Limit (us)"
    { 0x199e256, &prop_lst::ExposureAutoUpperLimit },                              // usb23 usb33 "Exposure Auto Upper Limit (us)"
    { 0x199e254, &prop_lst::ExposureAutoUpperLimitAuto },                          // usb23 usb33
    { 0x199e253, &prop_lst::ExposureAutoHighlightReduction },                      // usb23 usb33 "Highlight Reduction"
#if defined ENABLE_23U_SUPPORT
    { 0x00980913 /*V4L2_CID_GAIN*/,             &prop_lst::Gain, 0x199e204 },
    { 0x199e204, &prop_lst::Gain, gain_db100_converter },                           // usb23 "Gain (dB/100)"
#else
    { 0x00980913 /*V4L2_CID_GAIN*/,             &prop_lst::Gain },
#endif
    { 0x199e205, &prop_lst::GainAuto, fetch_menu_entries_off_continuous },         // usb23 usb33
    { 0x199e259, &prop_lst::GainAutoLowerLimit },                                  // usb23 usb33 "Gain Auto Lower Limit"
    { 0x199e260, &prop_lst::GainAutoUpperLimit },                                  // usb23 usb33 "Gain Auto Upper Limit"

    { V4L2_CID_AUTO_WHITE_BALANCE,  &prop_lst::BalanceWhiteAuto, fetch_menu_entries_off_continuous },
    { V4L2_CID_TIS_WHITEBALANCE_ONE_PUSH, mapping_type::internal },                // usb23 usb33 "White Balance One Push"
    { 0x199e246, &prop_lst::BalanceWhiteMode, mapping_type::blacklist },               // usb23 usb33 "White Balance Mode"
    { 0x199e247, &prop_lst::BalanceWhiteAutoPreset, mapping_type::blacklist },         // usb23 usb33 "White Balance Auto Preset"
    { 0x199e207, &prop_lst::BalanceWhiteAutoPreset },                                  // usb23 usb33 "White Balance Preset"
    { 0x199e249, &prop_lst::BalanceWhiteTemperaturePreset, mapping_type::blacklist },  // usb23 "White Balance Temp. Preset"
    { 0x199e250, &prop_lst::BalanceWhiteTemperature, mapping_type::blacklist },        // usb23 usb33 "White Balance Temperature",
    { 0x0098090e /*V4L2_CID_RED_BALANCE*/,      &prop_lst::BalanceWhiteRed, wb_256_channel_converter },
    { 0x199e248, &prop_lst::BalanceWhiteGreen, wb_256_channel_converter },             // usb23 usb33 integer V4L2_WHITE_BALANCE_GREEN
    { 0x0098090f /*V4L2_CID_BLUE_BALANCE*/,     &prop_lst::BalanceWhiteBlue, wb_256_channel_converter },

    { 0x199e241, &prop_lst::AutoFunctionsROIEnable },      // usb23 usb33 "Auto Functions ROI Control",
    { 0x199e258, &prop_lst::AutoFunctionsROIPreset },      // usb23 usb33 "Auto Functions ROI Preset",
    { 0x199e242, &prop_lst::AutoFunctionsROILeft },        // usb23 usb33
    { 0x199e243, &prop_lst::AutoFunctionsROITop },         // usb23 usb33
    { 0x199e244, &prop_lst::AutoFunctionsROIWidth },       // usb23 usb33
    { 0x199e245, &prop_lst::AutoFunctionsROIHeight },      // usb23 usb33

    { 0x009a0910 /*V4L2_CID_PRIVACY*/,          &prop_lst::TriggerMode, fetch_menu_entries_off_on },
    { 0x199e208, &prop_lst::TriggerMode, fetch_menu_entries_off_on, 0x009a0910 },  // usb2 usb23 usb33 "Trigger Mode"
    { 0x199e209, &prop_lst::TriggerSoftware },                                     // usb2 usb23 usb33
    { 0x199e210, &prop_lst::TriggerDelay, 0x199e272 },                             // usb23
    { 0x199e211, &prop_lst::StrobeEnable, fetch_menu_entries_off_on },             // usb2 usb23 usb33
    { 0x199e212, &prop_lst::StrobePolarity, fetch_menu_entries_StrobePolarity },   // usb2 usb23 usb33
    { 0x199e213, &prop_lst::StrobeOperation, fetch_menu_entries_StrobeOperation },                                     // usb23 usb33 "Strobe Exposure"/"Strobe Operation"
    { 0x199e214, &prop_lst::StrobeDuration },                                      // usb23 usb33
    { 0x199e215, &prop_lst::StrobeDelay },                                         // usb23 usb33
    { 0x199e216, &prop_lst::GPOut },                                               // usb23 usb33 "GPOUT" boolean
    { 0x199e217, &prop_lst::GPIn },                                                // usb23 usb33 "GPIN" boolean

    { 0x199e220, &prop_lst::OffsetAutoCenter, fetch_menu_entries_off_on },         // usb23 usb33 "ROI Auto Center",
    { 0x199e218, &prop_lst::OffsetX },                                             // usb23 usb33 "ROI Offset X"
    { 0x199e219, &prop_lst::OffsetY },                                             // usb23 usb33 "ROI Offset Y"

    { 0x199e927, &prop_lst::OffsetX },                                                 // usb2 "X Offset"
    { 0x199e928, &prop_lst::OffsetY },                                                 // usb2 "Y Offset"

    { 0x009a090a /*V4L2_CID_FOCUS_ABSOLUTE*/,   &prop_lst::Focus },
    { 0x199e221, &prop_lst::FocusAuto, fetch_menu_entries_off_once },              // usb23 usb33 "Auto Focus One Push",
    { 0x199e222, &prop_lst::AutoFocusROIEnable },                                  // usb23 usb33 "Auto Focus ROI Enable",
    { 0x199e223, &prop_lst::AutoFocusROILeft },                                    // usb23 usb33 "Auto Focus ROI Left",
    { 0x199e224, &prop_lst::AutoFocusROITop },                                     // usb23 usb33 "Auto Focus ROI Top",

    { V4L2_CID_ZOOM_ABSOLUTE,                   &prop_lst::Zoom },

    { 0x199e234, &prop_lst::TriggerActivation },                                        // usb23 usb33 "Trigger Polarity"
    { 0x199e235, &prop_lst::TriggerOperation },                                         // usb23 usb33
    { 0x199e92a, &prop_lst::TriggerOperation, fetch_menu_entries_TriggerOperation, 0x199e235 },    // usb2 "Trigger Global Reset Shutter" boolean
    { 0x199e261, &prop_lst::TriggerOperation, fetch_menu_entries_TriggerOperation, 0x199e235 },    // usb23 usb33 "Trigger Global Reset Release" boolean
    { 0x199e236, &prop_lst::TriggerSelector },                                          // usb23 usb33 "Trigger Exposure Mode"
    { 0x199e237, &prop_lst::AcquisitionBurstFrameCount },                               // usb23 usb33 "Trigger Burst Count", we use "AcquisitionBurstFrameCount" as the name for this
    { 0x199e238, &prop_lst::TriggerDebouncer },                                         // usb23 usb33 "Trigger Debounce Time (us)",
    { 0x199e239, &prop_lst::TriggerMask },                                              // usb23 usb33 "Trigger Mask Time (us)",
    { 0x199e240, &prop_lst::TriggerDenoise },                                           // usb23 usb33 "Trigger Noise Suppression Time"
    { 0x199e272, &prop_lst::TriggerDelay, trigger_delay_100ns_converter },              // usb33 "Trigger Delay (100ns)"
    { 0x199e262, &prop_lst::IMXLowLatencyTriggerMode },                                // usb33

    { 0x199e251, &prop_lst::ReverseX },                                                // usb23 usb33 "Flip Horizontal",
    { 0x199e252, &prop_lst::ReverseY },                                                // usb23 usb33 "Flip Vertical"

    { 0x199e257, mapping_type::internal },                                             // usb23 usb33 "Override Scanning Mode" integer
    { 0x199e263, mapping_type::internal },                                              // usb23 usb33 Scanning Mode Selector
    { 0x199e264, mapping_type::internal },                                              // usb23 usb33 Scanning Mode Identifier
    { 0x199e265, mapping_type::internal },                                              // usb23 usb33 Scanning Mode Scale Horizontal
    { 0x199e266, mapping_type::internal },                                              // usb23 usb33 Scanning Mode Scale Vertical
    { 0x199e267, mapping_type::internal },                                              // usb23 usb33 Scanning Mode Binning H
    { 0x199e268, mapping_type::internal },                                              // usb23 usb33 Scanning Mode Binning V
    { 0x199e269, mapping_type::internal },                                              // usb23 usb33 Scanning Mode Skipping H
    { 0x199e270, mapping_type::internal },                                              // usb23 usb33 Scanning Mode Skipping V
    { 0x199e271, mapping_type::internal },                                              // usb23 usb33 Scanning Mode Flag
    { 0x199e273, &prop_lst::ExpandOutputRange},                                         // usb33 dfg/hdmi Expand Output Range
    { 0x199e274, &prop_lst::ShowInfoOverlay},                                           // usb33 dfg/hdmi Show Info Overlay

    { 0x199e920, mapping_type::internal },                                             // usb2 "ExtO"
    // 0x199e921- 0x199e923 Gain channels
    { 0x199e925, mapping_type::internal },                                             // usb2 "Binning"
    { 0x199e929, mapping_type::internal },                                             // usb2 "Skipping"

#if 0   // most likely never used
    { 0x199e225, mapping_type::internal },         // usb23 "ATR Enable"
    { 0x199e226, mapping_type::internal },         // usb23 "ATR Gain"
    { 0x199e227, mapping_type::internal },         // usb23 "ATR Enable Wide D"
    { 0x199e228, mapping_type::internal },         // usb23 "ATR Contrast"
    { 0x199e229, mapping_type::internal },         // usb23 "ATR Contrast Auto"
    { 0x199e230, mapping_type::internal },         // usb23 "ATR Chroma"

    { 0x199e231, mapping_type::internal },         // usb23 "Image Stabilization"
    { 0x199e232, mapping_type::internal },         // usb23 "Noise Reduction"
    { 0x199e233, mapping_type::internal },         // usb23 "Face Detection"
#endif
};

static const converter_scale_init_float dxk72_gain_mul_dB_converter =
{ // actual range of sensor register seems to be [8;63], but firmware says its [4;63] so we just assume the firmware is right
    [](double v) -> int64_t
    { // 10 ^ (v / 20), this is voltage gain
        return std::lround(std::pow(10., v / 20.) * 4.);
    },
    [](int64_t v) -> double
    {
        return 20. * std::log10(v / 4.);
    },
    {},         // min
    {},         // max
    { 0.1 },    // stp, I am unsure if this is great. practically we have 59 valid values, and a range of [0.0;23.946]
    { 0. }      // default gain in the camera is bad!!
};

static const converter_scale_init_float dxk42_gain_mul_dB_converter =
{ // value range is factor = register_value / 32, register_value e [0;255]?, 32 == multiplication factor of 1
    [](double v) -> int64_t
    { // 10 ^ (v / 20), this is voltage gain
        return std::lround(std::pow(10., v / 20.) * 32.);
    },
    [](int64_t v) -> double
    {
        double factor = v / 32.;
        return 20. * std::log10(factor);
    },
    { 0. },     // min
    {},         // max
    { 0.1 },    // stp
    { 0. }      // def
};

static const converter_scale_init_float dxk22_gain_mul_dB_converter =
{ // value range is factor = register_value / 16, register_value e [16;64]
    [](double v) -> int64_t
    { // 10 ^ (v / 20), this is voltage gain
        return std::lround(std::pow(10., v / 20.) * 16.);
    },
    [](int64_t v) -> double
    {
        double factor = v / 16.;
        return 20. * std::log10(factor);
    },
    {},         // min
    {},         // max
    { 0.1 },    // stp
    { 0. }      // def
};

#if 0
// this function and it sibling exist for
// for the dfk 72. It's color channels have a range from 0-63
int64_t wb_for_DFK72_channel_from_std_to_dev( double value )
{
    const double std_max = 4.0;
    const int dev_max = 63;

    return std::llround( value * dev_max / std_max );
}

double wb_for_DFK72_from_dev_to_std( int64_t value )
{
    const double std_max = 4.0;
    const int dev_max = 63;

    return value * std_max / dev_max;
}

static const converter_scale_init_float wb_for_DFK72_channel_converter = { wb_for_DFK72_channel_from_std_to_dev,
                                                                wb_for_DFK72_from_dev_to_std };
#endif

static const v4l2_genicam_mapping    dxk72_conv_dict[] =
{
    { 0x00980913 /*V4L2_CID_GAIN*/, &prop_lst::Gain, dxk72_gain_mul_dB_converter },
    // { 0x0199e921, &prop_lst::BalanceWhiteRed, wb_for_DFK72_channel_converter },      // usb2 "GainR"
    // { 0x0199e922, &prop_lst::BalanceWhiteGreen, wb_for_DFK72_channel_converter },    // usb2 "GainG"
    // { 0x0199e923, &prop_lst::BalanceWhiteBlue, wb_for_DFK72_channel_converter },     // usb2 "GainB"
    { 0x0199e921, mapping_type::blacklist },    // usb2 "GainR"
    { 0x0199e922, mapping_type::blacklist },    // usb2 "GainG"
    { 0x0199e923, mapping_type::blacklist },    // usb2 "GainB"
};

static const v4l2_genicam_mapping    dxk42_conv_dict[] =
{
    { 0x00980913 /*V4L2_CID_GAIN*/, &prop_lst::Gain, dxk42_gain_mul_dB_converter },
};

static const v4l2_genicam_mapping    dxk22_conv_dict[] =
{
    { 0x00980913 /*V4L2_CID_GAIN*/, &prop_lst::Gain, dxk22_gain_mul_dB_converter },
};

namespace quirk_33u
{

static const converter_scale_init_float TriggerDenoise = {
    nullptr,
    nullptr,
    {},
    { 100'000. } // we override TriggerDenoise.Maximum, It is reported as 1e6, but should be 1e5
};

static const converter_scale_init_float ExposureTime_default = {
    nullptr,
    nullptr,
    {},
    {},
    {},
    { 33'333.0 },   // We fix the default value to 33'333 us
};

static const converter_scale_init_float gain_to_db_factor_0_1 = {
    [](double v) -> int64_t { return std::lround(v / 0.1); },
    [](int64_t v) -> double { return v * 0.1; },
};

static const converter_scale_init_float gain_to_db_factor_0_3 = {
    [](double v) -> int64_t { return std::lround(v / 0.3); },
    [](int64_t v) -> double { return v * 0.3; },
};

static const converter_scale_init_float gain_to_db_factor_AR0521 = {
    [](double v) -> int64_t { return std::lround(v / 0.1875); },
    [](int64_t v) -> double { return v * 0.1875; },
};

static const converter_scale_init_float gain_to_db_factor_IMX183 = {
    [](double v) -> int64_t { return std::lround( 2048. - (std::pow( 10., (v / (-20.))) * 2048.) ); },
    [](int64_t v) -> double { return std::abs( (-20.) * std::log10((2048. - v) / 2048.) ); },
    {},
    {},
    { 0.0135 },
};

// Min=144,Max=301,=> [0.0;3.2], step ~= 0.0204
// This is the same for P2000 and P5000
static const converter_scale_init_float gain_to_db_factor_P2000 = {
    [](double v) -> int64_t { return std::lround( std::pow( 10., v / 10.) * 144. ); },
    [](int64_t v) -> double { return 10. * std::log10(v / 144.); },
    { 0.0 },
    { 3.2 },    // we need to hardcode the maximum because old firmware versions had a wrong range (raw max == 1200)
    { 0.020 },
    { 0. },
};

// Min=144,Max=1200
static const converter_scale_init_float gain_to_db_factor_P1300 = {
    [](double v) -> int64_t { return std::lround( std::pow( 10., v / 10.) * 144. ); },
    [](int64_t v) -> double { return 10. * std::log10(v / 144.); },
    { 0.0 },
    { 9.208 },
    { 0.008 },
    { 0. }
};

// Min=100,Max=383,=> [0.0;5.83], step ~= 0.0206
static const converter_scale_init_float gain_to_db_factor_J003 = {
    [](double v) -> int64_t { return std::lround( std::pow( 10., v / 10.) * 100. ); },
    [](int64_t v) -> double { return 10. * std::log10(v / 100.); },
    { 0 },
    {},
    { 0.020 },
    { 0. }
};

// Min=13,Max=63 => [0.0;6.85]
static const converter_scale_init_float gain_to_db_factor_AR0234 = {
    [](double v) -> int64_t { return std::lround( std::pow( 10., v / 10.) * 13. ); },
    [](int64_t v) -> double { return 10. * std::log10(v / 13.); },
    {},
    {},
    { 0.13 },
    { 0. }
};

static const converter_scale_init_integer fix_auto_functions_wh_default = {
    nullptr,
    nullptr,
    {},
    {},
    {},
    {},
    []( prop_range_integer_default r ) { return prop_range_integer_default{ r.range, r.range.max }; },
};

static const converter_scale_init_float ExposureAutoLowerLimit_default = {
    nullptr,
    nullptr,
    {},
    {},
    {},
    {},
    []( prop_range_float_default r ) { return prop_range_float_default{ r.range, r.range.min }; },
};

} // quirk_33u

static const v4l2_genicam_mapping    dxk33u_conv_dict[] =
{
    { 0x199e201, &prop_lst::ExposureTime, quirk_33u::ExposureTime_default },

    { 0x199e240, &prop_lst::TriggerDenoise, quirk_33u::TriggerDenoise },

    { 0x199e255, &prop_lst::ExposureAutoLowerLimit, quirk_33u::ExposureAutoLowerLimit_default },
    { 0x199e256, &prop_lst::ExposureAutoUpperLimit, quirk_33u::ExposureTime_default },

    { 0x199e244, &prop_lst::AutoFunctionsROIWidth, quirk_33u::fix_auto_functions_wh_default },
    { 0x199e245, &prop_lst::AutoFunctionsROIHeight, quirk_33u::fix_auto_functions_wh_default },
};

static const v4l2_genicam_mapping    dxk33u_GainDB_factor_0_1[] =
{
    { V4L2_CID_GAIN, &prop_lst::Gain, quirk_33u::gain_to_db_factor_0_1 },
    { 0x199e259, &prop_lst::GainAutoLowerLimit, quirk_33u::gain_to_db_factor_0_1 },
    { 0x199e260, &prop_lst::GainAutoUpperLimit, quirk_33u::gain_to_db_factor_0_1 },
};

static const v4l2_genicam_mapping    dxk33u_GainDB_factor_0_3[] =
{
    { V4L2_CID_GAIN, &prop_lst::Gain, quirk_33u::gain_to_db_factor_0_3 },
    { 0x199e259, &prop_lst::GainAutoLowerLimit, quirk_33u::gain_to_db_factor_0_3 },
    { 0x199e260, &prop_lst::GainAutoUpperLimit, quirk_33u::gain_to_db_factor_0_3 },
};

static const v4l2_genicam_mapping    dxk33u_GainDB_factor_AR0521[] =
{
    { V4L2_CID_GAIN, &prop_lst::Gain, quirk_33u::gain_to_db_factor_AR0521 },
    { 0x199e259, &prop_lst::GainAutoLowerLimit, quirk_33u::gain_to_db_factor_AR0521 },
    { 0x199e260, &prop_lst::GainAutoUpperLimit, quirk_33u::gain_to_db_factor_AR0521 },
};

static const v4l2_genicam_mapping    dxk33u_GainDB_factor_IMX183[] =
{
    { V4L2_CID_GAIN, &prop_lst::Gain, quirk_33u::gain_to_db_factor_IMX183 },
    { 0x199e259, &prop_lst::GainAutoLowerLimit, quirk_33u::gain_to_db_factor_IMX183 },
    { 0x199e260, &prop_lst::GainAutoUpperLimit, quirk_33u::gain_to_db_factor_IMX183 },
};

static const v4l2_genicam_mapping    dxk33u_GainDB_factor_P2000[] =
{
    { V4L2_CID_GAIN, &prop_lst::Gain, quirk_33u::gain_to_db_factor_P2000 },
    { 0x199e259, &prop_lst::GainAutoLowerLimit, quirk_33u::gain_to_db_factor_P2000 },
    { 0x199e260, &prop_lst::GainAutoUpperLimit, quirk_33u::gain_to_db_factor_P2000 },
};

static const v4l2_genicam_mapping    dxk33u_GainDB_factor_P1300[] =
{
    { V4L2_CID_GAIN, &prop_lst::Gain, quirk_33u::gain_to_db_factor_P1300 },
    { 0x199e259, &prop_lst::GainAutoLowerLimit, quirk_33u::gain_to_db_factor_P1300 },
    { 0x199e260, &prop_lst::GainAutoUpperLimit, quirk_33u::gain_to_db_factor_P1300 },
};

static const v4l2_genicam_mapping    dxk33u_GainDB_factor_J003[] =
{
    { V4L2_CID_GAIN, &prop_lst::Gain, quirk_33u::gain_to_db_factor_J003 },
    { 0x199e259, &prop_lst::GainAutoLowerLimit, quirk_33u::gain_to_db_factor_J003 },
    { 0x199e260, &prop_lst::GainAutoUpperLimit, quirk_33u::gain_to_db_factor_J003 },
};

static const v4l2_genicam_mapping    dxk33u_GainDB_factor_AR0234[] =
{
    { V4L2_CID_GAIN, &prop_lst::Gain, quirk_33u::gain_to_db_factor_AR0234 },
    { 0x199e259, &prop_lst::GainAutoLowerLimit, quirk_33u::gain_to_db_factor_AR0234 },
    { 0x199e260, &prop_lst::GainAutoUpperLimit, quirk_33u::gain_to_db_factor_AR0234 },
};

static const v4l2_genicam_mapping   dxk52_GainDB_factor_AR0234[] =
{
    { V4L2_CID_GAIN, &prop_lst::Gain, quirk_33u::gain_to_db_factor_AR0234 },
};

static const v4l2_genicam_mapping dxg_hdmi_conv_dict[] =
{
    {V4L2_CID_EXPOSURE_AUTO, mapping_type::blacklist},
    {V4L2_CID_EXPOSURE_ABSOLUTE, mapping_type::blacklist},
};
// clang-format on


template<size_t N>
static const tcam::v4l2::v4l2_genicam_mapping* find_mapping_(const v4l2_genicam_mapping (&arr)[N],
                                                             uint32_t v4l2_id)
{
    for (const auto& entry : arr)
    {
        if (entry.v4l2_id == v4l2_id)
        {
            return &entry;
        }
    }
    return nullptr;
}

const tcam::v4l2::v4l2_genicam_mapping* find_mapping_info_33u_gain(uint32_t product_id,
                                                                   uint32_t v4l2_id)
{
    switch (product_id & PRODUCT_ID_SENSOR_MASK)
    {
        case PRODUCT_ID_SENSOR_AR0234:
            return find_mapping_(dxk33u_GainDB_factor_AR0234, v4l2_id);

        case PRODUCT_ID_SENSOR_P1300:
            return find_mapping_(dxk33u_GainDB_factor_P1300, v4l2_id);

        case PRODUCT_ID_SENSOR_P2000:
        case PRODUCT_ID_SENSOR_P5000:
            return find_mapping_(dxk33u_GainDB_factor_P2000, v4l2_id);
        case PRODUCT_ID_SENSOR_MT9J003:
            return find_mapping_(dxk33u_GainDB_factor_J003, v4l2_id);

        case PRODUCT_ID_SENSOR_IMX174:
        case PRODUCT_ID_SENSOR_IMX178:
        case PRODUCT_ID_SENSOR_IMX226:
        case PRODUCT_ID_SENSOR_IMX249:
        case PRODUCT_ID_SENSOR_IMX250:
        case PRODUCT_ID_SENSOR_IMX252:
        case PRODUCT_ID_SENSOR_IMX253:
        case PRODUCT_ID_SENSOR_IMX255:
        case PRODUCT_ID_SENSOR_IMX264:
        case PRODUCT_ID_SENSOR_IMX265:
        case PRODUCT_ID_SENSOR_IMX267:
        case PRODUCT_ID_SENSOR_IMX273:
        case PRODUCT_ID_SENSOR_IMX287:
        case PRODUCT_ID_SENSOR_IMX304:
        case PRODUCT_ID_SENSOR_IMX542:
        case PRODUCT_ID_SENSOR_IMX541:
        case PRODUCT_ID_SENSOR_IMX540:
        case PRODUCT_ID_SENSOR_IMX545:
        case PRODUCT_ID_SENSOR_IMX546:
        case PRODUCT_ID_SENSOR_IMX547:
            return find_mapping_(dxk33u_GainDB_factor_0_1, v4l2_id);

        case PRODUCT_ID_SENSOR_IMX290:
        case PRODUCT_ID_SENSOR_IMX462:
            return find_mapping_(dxk33u_GainDB_factor_0_3, v4l2_id);

        case PRODUCT_ID_SENSOR_AR0521:
            return find_mapping_(dxk33u_GainDB_factor_AR0521, v4l2_id);


        case PRODUCT_ID_SENSOR_IMX183:
            return find_mapping_(dxk33u_GainDB_factor_IMX183, v4l2_id);

        // stuff
        case PRODUCT_ID_SENSOR_:
        case PRODUCT_ID_SENSOR_HDMI:
            return nullptr;
    }
    return nullptr;
}

const tcam::v4l2::v4l2_genicam_mapping* find_mapping_info_specific(v4l2_device_type dev_type,
                                                                   uint32_t product_id,
                                                                   uint32_t v4l2_id)
{
    switch (dev_type)
    {
        case tcam::v4l2::v4l2_device_type::dxk72: // MT9P031
            return find_mapping_(dxk72_conv_dict, v4l2_id);
        case tcam::v4l2::v4l2_device_type::dxk42: // MT9M021
            return find_mapping_(dxk42_conv_dict, v4l2_id);
        case tcam::v4l2::v4l2_device_type::dxk22: // MT9V023
            return find_mapping_(dxk22_conv_dict, v4l2_id);
        case tcam::v4l2::v4l2_device_type::dxk52:
        {
            if (auto ptr = find_mapping_(dxk52_GainDB_factor_AR0234, v4l2_id);
                ptr != nullptr)
            {
                return ptr;
            }
            break;
        }
        case tcam::v4l2::v4l2_device_type::dxk33u:
        case tcam::v4l2::v4l2_device_type::dxk37u:
        case tcam::v4l2::v4l2_device_type::dxk38u:
        {
            if (auto ptr = find_mapping_info_33u_gain(product_id, v4l2_id); ptr != nullptr)
            {
                return ptr;
            }
            if (auto ptr = find_mapping_(dxk33u_conv_dict, v4l2_id); ptr != nullptr)
            {
                return ptr;
            }
            break;
        }
        case tcam::v4l2::v4l2_device_type::dxk_hdmi:
        {
            if (auto ptr = find_mapping_(dxg_hdmi_conv_dict, v4l2_id); ptr != nullptr)
            {
                return ptr;
            }
            if (auto ptr = find_mapping_(dxk33u_conv_dict, v4l2_id); ptr != nullptr)
            {
                return ptr;
            }
            break;
        }
        case tcam::v4l2::v4l2_device_type::unknown:
        default:
            break;
    }
    return nullptr;
}

} // namespace


tcam::v4l2::v4l2_genicam_mapping_info tcam::v4l2::find_mapping_info(v4l2_device_type dev_type,
                                                                    uint32_t product_id,
                                                                    uint32_t v4l2_id)
{
    const tcam::v4l2::v4l2_genicam_mapping* ptr =
        find_mapping_info_specific(dev_type, product_id, v4l2_id);
    if (ptr == nullptr)
    {
        ptr = find_mapping_(generic_v4l2_conv_table, v4l2_id);
    }
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
    [[maybe_unused]] v4l2_device_type dev_type,
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
                mapping.converter_integer_);
        }
        case tcamprop1::prop_type::Float:
        {
            return std::make_shared<V4L2PropertyDoubleImpl>(
                current_prop_qctrl,
                p_property_backend,
                static_cast<const tcamprop1::prop_static_info_float*>(mapping.info_),
                mapping.converter_float_);
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
        case tcamprop1::prop_type::String:
        {
            SPDLOG_ERROR("Currently no string property support implemented in v4l2.");
            return nullptr;
        }
    }
    return nullptr;
}

std::vector<uint32_t> tcam::v4l2::get_ordered_v4l2_id_list()
{
    std::vector<uint32_t> rval;
    rval.reserve(std::size(generic_v4l2_conv_table));
    for (const auto& entry : generic_v4l2_conv_table) { rval.push_back(entry.v4l2_id); }
    return rval;
}
