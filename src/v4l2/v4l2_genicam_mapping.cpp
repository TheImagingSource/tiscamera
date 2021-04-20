
#include "v4l2_genicam_mapping.h"

#include <map>


static const std::map<int, struct tcam::v4l2::v4l2_genicam_mapping> v4l2_conv_dict {
    {
        0x00980900, // brightness
        {
            "BlackLevel",
            TCAM_PROPERTY_TYPE_DOUBLE,
            {},
        },
    },
    {
        0x009a0902, // exposure absolute
        {
            "ExposureTime",
            TCAM_PROPERTY_TYPE_DOUBLE,
            {},
        },
    },
    {
        0x199e204,
        {
            "Gain",
            TCAM_PROPERTY_TYPE_DOUBLE,
            {},
        }
    } ,
    { 0x199e205, // Gain Auto
      {
          "GainAuto",
          TCAM_PROPERTY_TYPE_ENUMERATION,
          { { { 0, "Off" }, { 1, "Continuous" } } },
      } },
    { 0x00980913, // Gain
      { "", TCAM_PROPERTY_TYPE_DOUBLE, {} } },
    { 0x199e202, // Auto Shutter
      {
          "ExposureAuto",
          TCAM_PROPERTY_TYPE_ENUMERATION,
          { { { 0, "Off" }, { 1, "Continuous" } } },
      } },
    { 0x199e201, // Exposure Time (us)
      {
          "ExposureTime",
          TCAM_PROPERTY_TYPE_DOUBLE,
          {},
      } },
    {
        0x199e209,
        {
            "TriggerSoftware",
            TCAM_PROPERTY_TYPE_UNKNOWN,
            {},
        },
    },
    {
        0x199e208,
        { "TriggerMode", TCAM_PROPERTY_TYPE_ENUMERATION, { { { 0, "Off" }, { 1, "On" } } } },
    },
    { 0x0199e203, { "ExposureAutoReference", TCAM_PROPERTY_TYPE_UNKNOWN, {} } },
    { 0x00980910, // Gamma
      { "", TCAM_PROPERTY_TYPE_DOUBLE, {} } },
    { 0x00980903, // Hue
      { "", TCAM_PROPERTY_TYPE_DOUBLE, {} } },
    { 0x00980902, // Saturation
      { "", TCAM_PROPERTY_TYPE_DOUBLE, {} } },
    { 0x0199e210, { "TriggerDelay", TCAM_PROPERTY_TYPE_UNKNOWN, {} } },
    { 0x0199e211, { "StrobeEnable", TCAM_PROPERTY_TYPE_UNKNOWN, {} } },
    { 0x0199e212, { "StrobePolarity", TCAM_PROPERTY_TYPE_UNKNOWN, {} } },
    { 0x0199e213, { "StrobeExposure", TCAM_PROPERTY_TYPE_UNKNOWN, {} } },
    { 0x0199e214, { "StrobeDuration", TCAM_PROPERTY_TYPE_UNKNOWN, {} } },
    { 0x0199e215, { "StrobeDelay", TCAM_PROPERTY_TYPE_UNKNOWN, {} } },
    { 0x0199e217, { "GPIn", TCAM_PROPERTY_TYPE_INTEGER, {} } },
    { 0x0199e216, { "GPOut", TCAM_PROPERTY_TYPE_INTEGER, {} } },
    {
        0x0199e234,
        { "TriggerPolarity", TCAM_PROPERTY_TYPE_UNKNOWN, {} },
    },
    {
        0x0199e240,
        { "TriggerNoiseSuppressionTime", TCAM_PROPERTY_TYPE_UNKNOWN, {} },
    },
    {
        0x0199e242,
        { "AutoFunctionsROILeft", TCAM_PROPERTY_TYPE_UNKNOWN, {} },
    },
    {
        0x0199e243,
        { "AutoFunctionsROITop", TCAM_PROPERTY_TYPE_UNKNOWN, {} },
    },
    {
        0x0199e244,
        { "AutoFunctionsROIWidth", TCAM_PROPERTY_TYPE_UNKNOWN, {} },
    },
    {
        0x0199e245,
        { "AutoFunctionsROIHeight", TCAM_PROPERTY_TYPE_UNKNOWN, {} },
    },
    {
        0x0199e246,
        { "BalanceWhiteMode", TCAM_PROPERTY_TYPE_UNKNOWN, {} },
    },
    {
        0x0199e247,
        { "BalanceWhiteAutoPreset", TCAM_PROPERTY_TYPE_UNKNOWN, {} },
    },
    {
        0x0199e248,
        { "BalanceWhiteComponentGreen", TCAM_PROPERTY_TYPE_UNKNOWN, {} },
    },
    {
        0x0199e249,
        { "BalanceWhiteTemperaturePreset", TCAM_PROPERTY_TYPE_UNKNOWN, {} },
    },
    {
        0x0199e250,
        { "BalanceWhiteTemperature", TCAM_PROPERTY_TYPE_UNKNOWN, {} },
    },
    {
        0x0199e258,
        { "AutoFunctionsROIPreset", TCAM_PROPERTY_TYPE_UNKNOWN, {} },
    },
    {
        0x0199e256,
        { "ExposureAutoUpperLimit", TCAM_PROPERTY_TYPE_DOUBLE, {} },
    },
    {
        0x0199e254,
        {
            "ExposureAutoUpperLimitAuto",
            TCAM_PROPERTY_TYPE_UNKNOWN,
            {},
        },
    },
    {
        0x0199e261, // TriggerGlobalResetRelease
        {
            "SensorShutterMode",
            TCAM_PROPERTY_TYPE_UNKNOWN,
            {},
        },
    },
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
