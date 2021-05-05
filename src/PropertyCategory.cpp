

#include "PropertyCategory.h"

#include "string_view"

namespace
{
    struct tcam_category_table_entry
    {
        std::string category;
        std::string group;
        std::string property;
    };

    const tcam_category_table_entry category_table [] =
    {
        {"Exposure", "Exposure", "ExposureTime"},
        {"Exposure", "Exposure", "ExposureAuto"},
        {"Exposure", "Exposure", "ExposureAutoReference"},
        {"Exposure", "Exposure", "HighlightReduction"},
        {"Exposure", "Exposure", "ExposureAutoUpperLimitAuto"},
        {"Exposure", "Exposure", "ExposureAutoUpperLimit"},
        {"Exposure", "Exposure", "ExposureAutoLowerLimit"},
        {"Exposure", "Exposure", "ExposureAutoHighlightReduction"},
        {"Exposure", "Exposure", "ExposureAutoHighlighReduction"}, // backward compatible workaraound
        {"Exposure", "Gain", "Gain"},
        {"Exposure", "Gain", "GainAuto"},
        {"Exposure", "Gain", "GainAutoUpperLimit"},
        {"Exposure", "Gain", "GainAutoLowerLimit"},
        {"Exposure", "Brightness", "Brightness"},
        {"Exposure", "Contrast", "Contrast"},
        {"Exposure", "BlackLevel", "BlackLevel"},
        {"Exposure", "Shutter", "Shutter"},
        {"Exposure", "HDR", "HDR"},
        {"Color", "Saturation", "Saturation"},
        {"Color", "Hue", "Hue"},
        {"Color", "BalanceWhiteMode", "BalanceWhiteAuto"},
        {"Color", "BalanceWhiteMode", "BalanceWhiteRed"},
        {"Color", "BalanceWhiteMode", "BalanceWhiteGreen"},
        {"Color", "BalanceWhiteMode", "BalanceWhiteBlue"},
        {"Color", "BalanceWhiteMode", "BalanceWhiteTemperature"},
        {"Color", "BalanceWhiteMode", "BalanceWhiteAutoPreset"},
        {"Color", "BalanceWhiteMode", "BalanceWhiteMode"},
        {"Color", "BalanceWhiteMode", "BalanceWhitePreset"},
        {"Color", "BalanceWhiteMode", "BalanceWhiteTemperature"},
        {"Color", "BalanceWhiteMode", "BalanceRatioSelector"},
        {"Color", "BalanceWhiteMode", "BalanceRatio"},
        {"Color", "BalanceWhiteMode", "BalanceWhiteAutoPreset"},
        {"Color", "BalanceWhiteMode", "BalanceWhiteTemperaturePreset"},
        {"Lens", "IRCutFilter", "IRCutFilter"},
        {"Lens", "Iris", "Iris"},
        {"Lens", "Focus", "Focus"},
        {"Lens", "Zoom", "Zoom"},
        {"Lens", "Focus", "FocusAuto"},
        {"Lens", "FocusAuto", "FocusROITop"},
        {"Lens", "FocusAuto", "FocusROILeft"},
        {"Lens", "FocusAuto", "FocusROIWidth"},
        {"Lens", "FocusAuto", "FocusROIHeight"},
        {"Lens", "OISMode", "OISMode"},
        {"Lens", "OISMode", "OISItemPositionX"},
        {"Lens", "OISMode", "OISItemPositionY"},
        {"Image", "Gamma", "Gamma"},
        {"Image", "Sharpness", "Sharpness"},
        {"Image", "ReverseX", "ReverseX"},
        {"Image", "ReverseY", "ReverseY"},
        {"Special", "TriggerMode", "TriggerMode"},
        {"Special", "TriggerMode", "TriggerSource"},
        {"Special", "TriggerMode", "TriggerActivation"},
        {"Special", "TriggerMode", "TriggerSoftware"},
        {"Special", "TriggerMode", "TriggerDenoise"},
        {"Special", "TriggerMode", "TriggerMask"},
        {"Special", "TriggerMode", "TriggerDebouncer"},
        {"Special", "TriggerMode", "TriggerDelay"},
        {"Special", "TriggerMode", "TriggerSelector"},
        {"Special", "TriggerMode", "TriggerOperation"},
        {"Special", "TriggerMode", "TriggerPolarity"},
        {"Special", "TriggerMode", "TriggerExposureMode"},
        {"Special", "TriggerMode", "TriggerBurstCount"},
        {"Special", "TriggerMode", "TriggerDebounceTime"},
        {"Special", "TriggerMode", "TriggerMaskTime"},
        {"Special", "TriggerMode", "TriggerNoiseSuppressionTime"},
        {"Special", "TriggerMode", "TriggerOverlap"},
        {"Special", "TriggerMode", "SensorShutterMode"},
        {"Special", "GPIO", "GPIO"},
        {"Special", "GPIO", "GPIn"},
        {"Special", "GPIO", "GPOut"},
        {"AutoROI", "AutoFunctionsROIControl", "AutoFunctionsROIControl"},
        {"AutoROI", "AutoFunctionsROIControl", "AutoFunctionsROILeft"},
        {"AutoROI", "AutoFunctionsROIControl", "AutoFunctionsROITop"},
        {"AutoROI", "AutoFunctionsROIControl", "AutoFunctionsROIWidth"},
        {"AutoROI", "AutoFunctionsROIControl", "AutoFunctionsROIHeight"},
        {"AutoROI", "AutoFunctionsROIControl", "AutoFunctionsROIPreset"},
        {"AutoROI", "AutoFunctionsROIControl", "AutoFunctionsROIEnable"},
        {"Special", "StrobeEnable", "StrobeEnable"},
        {"Special", "StrobeEnable", "StrobeExposure"},
        {"Special", "StrobeEnable", "StrobeDuration"},
        {"Special", "StrobeEnable", "StrobeOperation"},
        {"Special", "StrobeEnable", "StrobePolarity"},
        {"Special", "StrobeEnable", "StrobeDelay"},
        {"Special", "StrobeEnable", "StrobeSecondDelay"},
        {"Special", "StrobeEnable", "StrobeSecondDuration"},
        {"Special", "StrobeEnable", "StrobeMode"},
        {"Special", "TriggerMode", "IMXLowLatencyMode"},
        {"Special", "TriggerMode", "TriggerGlobalResetRelease"},
        {"PartialScan", "OffsetAuto", "OffsetX"},
        {"PartialScan", "OffsetAuto", "OffsetY"},
        {"PartialScan", "OffsetAuto", "OffsetAuto"},
        {"PartialScan", "OverrideScanningMode", "OverrideScanningMode"},
        {"Unknown", "Skipping", "Skipping"},
        {"Unknown", "Binning", "Binning"},
        {"Unknown", "Binning", "BinningHorizontal"},
        {"Unknown", "Binning", "BinningVertical"},
        {"Device", "DeviceType", "DeviceType"},
        {"Device", "DeviceTemperature", "DeviceTemperature"},
        {"Device", "DeviceTemperature", "DeviceTemperatureSelector"},
        {"Device", "Device", "Device"},
        {"Device", "", "TimestampLatchValue"},
        {"Device", "", "TimestampLatch"},
        {"Device", "", "TimestampReset"},
        {"Device", "", "DeviceScanType"},
        {"Device", "", "DeviceVendorName"},
        {"Device", "", "DeviceModelName"},
        {"Device", "", "DeviceVersion"},
        {"Device", "", "DeviceSerialNumber"},
        {"Device", "", "DeviceUserID"},
        {"Device", "", "DeviceSFNCVersionMajor"},
        {"Device", "", "DeviceSFNCVersionMinor"},
        {"Device", "", "DeviceSFNCVersionSubMinor"},
        {"Device", "", "DeviceTLType"},
        {"Device", "", "DeviceTLVersionMajor"},
        {"Device", "", "DeviceTLVersionMinor"},
        {"Device", "", "DeviceTLVersionSubMinor"},
        {"Device", "", "DeviceLinkSelector"},
        {"Device", "", "DeviceLinkHeartbeatTimeout"},
        {"Device", "", "DeviceStreamChannelCount"},
        {"Device", "", "DeviceStreamChannelSelector"},
        {"Device", "", "DeviceStreamChannelType"},
        {"Device", "", "DeviceStreamChannelLink"},
        {"Device", "", "DeviceStreamChannelEndianness"},
        {"Device", "", "DeviceStreamChannelPacketSize"},
        {"Device", "", "DeviceEventChannelCount"},
        {"Device", "", "DeviceReset"},
        {"Device", "", "DeviceFactoryReset"},
        {"ActionControl", "", "ActionDeviceKey"},
        {"ActionControl", "", "ActionSelector"},
        {"ActionControl", "", "ActionGroupMask"},
        {"ActionControl", "", "ActionGroupKey"},
        {"ActionControl", "", "ActionQueueSize"},
        {"ActionControl", "", "ActionScheduler"},
        {"ActionControl", "", "ActionSchedulerSize"},
        {"ActionControl", "", "ActionSchedulerCancel"},
        {"ActionControl", "", "ActionSchedulerTime"},
        {"ActionControl", "", "ActionSchedulerInterval"},
        {"ActionControl", "", "ActionSchedulerStatus"},
        {"ActionControl", "", "ActionSchedulerCommit"},
        {"LUTControl", "", "LUTSelector"},
        {"LUTControl", "", "LUTEnable"},
        {"LUTControl", "", "LUTIndex"},
        {"LUTControl", "", "LUTValue"},
        {"LUTControl", "", "LUTValueAll"},
    };

}

namespace tcam::property
{

    std::string get_display_category(std::string_view property)
    {
        for (const auto& entry : category_table)
        {
            if (entry.property == property)
            {
                return entry.category;
            }
        }
        return "Unknown";
    }

    std::string get_display_group(std::string_view property)
    {
        for (const auto& entry : category_table)
        {
            if (entry.property == property)
            {
                return entry.group;
            }
        }
        return std::string(property);
    }

}
