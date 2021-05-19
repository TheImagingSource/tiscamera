
#include "v4l2_genicam_mapping.h"

#include <map>

//{ , { "", TCAM_PROPERTY_TYPE_UNKNOWN, {}, }, },

static const std::map<int, struct tcam::v4l2::v4l2_genicam_mapping> v4l2_conv_dict {
    // brightness
    { 0x00980900, { "BlackLevel", TCAM_PROPERTY_TYPE_DOUBLE, {}, }, },
    // Saturation
    { 0x00980902, { "", TCAM_PROPERTY_TYPE_DOUBLE, {} } },
    // Hue
    { 0x00980903, { "", TCAM_PROPERTY_TYPE_DOUBLE, {} } },
    // Gain
    { 0x00980913, { "", TCAM_PROPERTY_TYPE_DOUBLE, {} } },
    // Gamma
    { 0x00980910, { "", TCAM_PROPERTY_TYPE_DOUBLE, {} } },
    // exposure_auto
    { 0x009a0901, { "ExposureAuto", TCAM_PROPERTY_TYPE_ENUMERATION,
                    { { { 1, "Off" }, { 3, "On" } } } } },
    // exposure absolute
    { 0x009a0902, { "ExposureTime", TCAM_PROPERTY_TYPE_DOUBLE, {}, }, },
    { 0x0098090c, { "BalanceWhiteAuto", TCAM_PROPERTY_TYPE_ENUMERATION,
                    { { {0, "Off"}, {1, "Continuous" } } }, }, },
    { 0x0098090e, { "BalanceWhiteRed", TCAM_PROPERTY_TYPE_UNKNOWN, {}, }, },
    { 0x0098090f, { "BalanceWhiteBlue", TCAM_PROPERTY_TYPE_UNKNOWN, {}, }, },
    // privacy aka trigger
    { 0x009a0910, { "TriggerMode", TCAM_PROPERTY_TYPE_ENUMERATION,
                    { { { 0, "Off" }, { 1, "On" } } } }, },
    { 0x009a090a, { "Focus", TCAM_PROPERTY_TYPE_UNKNOWN, {}, }, },
    // Exposure Time (us)
    { 0x199e201, { "ExposureTime", TCAM_PROPERTY_TYPE_DOUBLE, {}, } },
    // Auto Shutter
    { 0x199e202, { "ExposureAuto", TCAM_PROPERTY_TYPE_ENUMERATION,
                   { { { 0, "Off" }, { 1, "Continuous" } } }, } },
    { 0x199e203, { "ExposureAutoReference", TCAM_PROPERTY_TYPE_UNKNOWN, {} } },
    { 0x199e204, { "Gain", TCAM_PROPERTY_TYPE_DOUBLE, {}, } },
    // Gain Auto
    { 0x199e205, { "GainAuto", TCAM_PROPERTY_TYPE_ENUMERATION,
                   { { { 0, "Off" }, { 1, "Continuous" } } }, } },
    { 0x199e207, { "BalanceWhitePreset", TCAM_PROPERTY_TYPE_UNKNOWN, {}, }, },
    { 0x199e208, { "TriggerMode", TCAM_PROPERTY_TYPE_ENUMERATION,
                   { { { 0, "Off" }, { 1, "On" } } } }, },
    { 0x0199e209, { "TriggerSoftware", TCAM_PROPERTY_TYPE_UNKNOWN, {}, },},
    { 0x0199e210, { "TriggerDelay", TCAM_PROPERTY_TYPE_UNKNOWN, {} } },
    { 0x0199e211, { "StrobeEnable", TCAM_PROPERTY_TYPE_UNKNOWN, {} } },
    { 0x0199e212, { "StrobePolarity", TCAM_PROPERTY_TYPE_UNKNOWN, {} } },
    { 0x0199e213, { "StrobeExposure", TCAM_PROPERTY_TYPE_UNKNOWN, {} } },
    { 0x0199e214, { "StrobeDuration", TCAM_PROPERTY_TYPE_UNKNOWN, {} } },
    { 0x0199e215, { "StrobeDelay", TCAM_PROPERTY_TYPE_UNKNOWN, {} } },
    { 0x0199e216, { "GPOut", TCAM_PROPERTY_TYPE_INTEGER, {} } },
    { 0x0199e217, { "GPIn", TCAM_PROPERTY_TYPE_INTEGER, {} } },
    { 0x0199e218, { "OffsetX", TCAM_PROPERTY_TYPE_UNKNOWN, {}, }, },
    { 0x0199e219, { "OffsetY", TCAM_PROPERTY_TYPE_UNKNOWN, {}, }, },
    { 0x0199e220, { "OffsetAuto", TCAM_PROPERTY_TYPE_UNKNOWN, {}, }, },
    { 0x0199e921, { "BalanceWhiteRed",  TCAM_PROPERTY_TYPE_UNKNOWN, {}}, },
    { 0x0199e922, { "BalanceWhiteGreen",  TCAM_PROPERTY_TYPE_UNKNOWN, {}}, },
    { 0x0199e923, { "BalanceWhiteBlue",  TCAM_PROPERTY_TYPE_UNKNOWN, {}}, },
    { 0x0199e927, { "OffsetX", TCAM_PROPERTY_TYPE_UNKNOWN, {} } },
    { 0x0199e928, { "OffsetY", TCAM_PROPERTY_TYPE_UNKNOWN, {} } },
    { 0x0199e234, { "TriggerPolarity", TCAM_PROPERTY_TYPE_UNKNOWN, {} }, },
    { 0x0199e235, { "TriggerOperation", TCAM_PROPERTY_TYPE_UNKNOWN, {}, }, },
    { 0x0199e236, { "TriggerExposureMode", TCAM_PROPERTY_TYPE_UNKNOWN, {}, }, },
    { 0x0199e237, { "TriggerBurstCount", TCAM_PROPERTY_TYPE_UNKNOWN, {}, }, },
    { 0x0199e238, { "TriggerDebounceTime", TCAM_PROPERTY_TYPE_UNKNOWN, {}, }, },
    { 0x0199e239, { "TriggerMaskTime", TCAM_PROPERTY_TYPE_UNKNOWN, {}, }, },
    { 0x0199e240, { "TriggerNoiseSuppressionTime", TCAM_PROPERTY_TYPE_UNKNOWN, {} }, },
    { 0x0199e241, { "AutoFunctionsROIControl", TCAM_PROPERTY_TYPE_UNKNOWN, {}, }, },
    { 0x0199e242, { "AutoFunctionsROILeft", TCAM_PROPERTY_TYPE_UNKNOWN, {} }, },
    { 0x0199e243, { "AutoFunctionsROITop", TCAM_PROPERTY_TYPE_UNKNOWN, {} }, },
    { 0x0199e244, { "AutoFunctionsROIWidth", TCAM_PROPERTY_TYPE_UNKNOWN, {} }, },
    { 0x0199e245, { "AutoFunctionsROIHeight", TCAM_PROPERTY_TYPE_UNKNOWN, {} }, },
    { 0x0199e246, { "BalanceWhiteMode", TCAM_PROPERTY_TYPE_UNKNOWN, {} },},
    { 0x0199e247, { "BalanceWhiteAutoPreset", TCAM_PROPERTY_TYPE_UNKNOWN, {} }, },
    { 0x0199e248, { "BalanceWhiteGreen", TCAM_PROPERTY_TYPE_UNKNOWN, {} } },
    { 0x0199e249, { "BalanceWhiteTemperaturePreset", TCAM_PROPERTY_TYPE_UNKNOWN, {} }, },
    { 0x0199e250, { "BalanceWhiteTemperature", TCAM_PROPERTY_TYPE_UNKNOWN, {} }, },
    { 0x0199e251, { "ReverseY", TCAM_PROPERTY_TYPE_UNKNOWN, {}, }, },
    { 0x0199e252, { "ReverseX", TCAM_PROPERTY_TYPE_UNKNOWN, {}, }, },
    { 0x0199e253, { "HighlightReduction", TCAM_PROPERTY_TYPE_UNKNOWN, {}, }, },
    { 0x0199e254, { "ExposureAutoUpperLimitAuto", TCAM_PROPERTY_TYPE_ENUMERATION,
                    { { { 0, "Off" }, { 1, "On" } } }, }, },
    { 0x0199e255, { "ExposureAutoLowerLimit", TCAM_PROPERTY_TYPE_DOUBLE, {}, }, },
    { 0x0199e256, { "ExposureAutoUpperLimit", TCAM_PROPERTY_TYPE_DOUBLE, {} },},
    { 0x0199e257, { "OverrideScanningMode", TCAM_PROPERTY_TYPE_UNKNOWN, {}, }, },
    { 0x0199e258, { "AutoFunctionsROIPreset", TCAM_PROPERTY_TYPE_UNKNOWN, {} },},
    { 0x0199e259, { "GainAutoLowerLimit", TCAM_PROPERTY_TYPE_DOUBLE, {}, }, },
    { 0x0199e260, { "GainAutoUpperLimit", TCAM_PROPERTY_TYPE_DOUBLE, {}, }, },
    // TriggerGlobalResetRelease
    { 0x0199e261, { "SensorShutterMode", TCAM_PROPERTY_TYPE_UNKNOWN, {}, }, },
    { 0x0199e262, { "IMXLowLatencyMode", TCAM_PROPERTY_TYPE_UNKNOWN, {}, }, },
};


const struct tcam::v4l2::v4l2_genicam_mapping* tcam::v4l2::find_mapping(uint32_t v4l2_id)
{
    auto it = v4l2_conv_dict.find(v4l2_id);

    if (it == v4l2_conv_dict.end())
    {
        return nullptr;
    }

    return &(it->second);
}
