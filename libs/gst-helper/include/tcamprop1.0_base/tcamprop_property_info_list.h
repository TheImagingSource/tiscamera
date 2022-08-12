
/**
 * This file contains the(/many) known properties and their static information tidbits
 *
 * If you add one here, add the property to the list in src/tcamprop1.0_base/tcamprop_property_list_impl.cpp
 *
 * Guidelines:
 *  - The variable should be the same as the GenICam name
 *  - When adding a property here, add all its data here.
 *  - Not all properties belong here, fully camera specified properties do not need to be replicated here
 *  - enum entries do not need to be exhaustive
 */

#pragma once

#include "tcamprop_property_info.h"

#include <array>

namespace tcamprop1::prop_list
{
    constexpr const std::array<std::string_view, 2> enum_entries_off_auto = { "Off", "Continuous" };
    constexpr const std::array<std::string_view, 3> enum_entries_off_auto_once = { "Off", "Continuous", "Once" };
    constexpr const std::array<std::string_view, 2> enum_entries_off_on = { "Off", "On" };
    constexpr const std::array<std::string_view, 2> enum_entries_off_once = { "Off", "Once" };

    constexpr const std::array<std::string_view, 6> AutoFunctionsROIPreset_EnumEntries = {
        "Full Sensor", "Custom Rectangle", "Center 50%", "Center 25%", "Bottom Half", "Top Half",
    };

    extern const prop_static_info_float Gain;
    extern const prop_static_info_enumeration GainAuto;
    extern const prop_static_info_float GainAutoLowerLimit;
    extern const prop_static_info_float GainAutoUpperLimit;

    extern const prop_static_info_float ExposureTime;
    extern const prop_static_info_enumeration ExposureAuto;
    extern const prop_static_info_integer ExposureAutoReference;
    extern const prop_static_info_float ExposureAutoLowerLimit;
    extern const prop_static_info_float ExposureAutoUpperLimit;
    extern const prop_static_info_boolean ExposureAutoUpperLimitAuto;
    extern const prop_static_info_boolean ExposureAutoHighlightReduction;

    extern const prop_static_info_enumeration BalanceWhiteAuto;

    extern const prop_static_info_enumeration BalanceWhiteMode;
    extern const prop_static_info_enumeration BalanceWhiteAutoPreset;
    extern const prop_static_info_enumeration BalanceWhiteTemperaturePreset;

    extern const prop_static_info_integer BalanceWhiteTemperature;
    extern const prop_static_info_float BalanceWhiteRed;
    extern const prop_static_info_float BalanceWhiteGreen;
    extern const prop_static_info_float BalanceWhiteBlue;

    extern const prop_static_info_boolean ClaimBalanceWhiteSoftware;
    extern const prop_static_info_boolean ClaimHDRGain;
    extern const prop_static_info_float HDRGain;
    extern const prop_static_info_boolean HDRGainAuto;
    extern const prop_static_info_float HDRGainAutoReference;

    extern const prop_static_info_float BlackLevel;
    extern const prop_static_info_integer OffsetX;
    extern const prop_static_info_integer OffsetY;
    extern const prop_static_info_enumeration OffsetAutoCenter;
    extern const prop_static_info_boolean ReverseX;
    extern const prop_static_info_boolean ReverseY;

    extern const prop_static_info_integer Iris;
    extern const prop_static_info_enumeration IrisAuto;

    extern const prop_static_info_integer Focus;
    extern const prop_static_info_enumeration FocusAuto;
    extern const prop_static_info_boolean AutoFocusROIEnable;
    extern const prop_static_info_integer AutoFocusROILeft;
    extern const prop_static_info_integer AutoFocusROITop;
    extern const prop_static_info_integer AutoFocusROIWidth;
    extern const prop_static_info_integer AutoFocusROIHeight;

    extern const prop_static_info_integer Zoom;

    extern const prop_static_info_boolean IRCutFilterEnable;

    extern const prop_static_info_boolean AutoFunctionsROIEnable;
    extern const prop_static_info_enumeration AutoFunctionsROIPreset;
    extern const prop_static_info_integer AutoFunctionsROIHeight;
    extern const prop_static_info_integer AutoFunctionsROIWidth;
    extern const prop_static_info_integer AutoFunctionsROITop;
    extern const prop_static_info_integer AutoFunctionsROILeft;

    extern const prop_static_info_integer Denoise;
    extern const prop_static_info_integer Sharpness;

    extern const prop_static_info_float SoftwareBrightness;
    extern const prop_static_info_integer Contrast;
    extern const prop_static_info_float Gamma;
    extern const prop_static_info_float Saturation;
    extern const prop_static_info_float Hue;

    extern const prop_static_info_boolean TonemappingEnable;
    extern const prop_static_info_float TonemappingGlobalBrightness;
    extern const prop_static_info_float TonemappingIntensity;

    extern const prop_static_info_boolean ColorTransformationEnable;
    extern const prop_static_info_float ColorTransformation_Value_Gain00;
    extern const prop_static_info_float ColorTransformation_Value_Gain01;
    extern const prop_static_info_float ColorTransformation_Value_Gain02;
    extern const prop_static_info_float ColorTransformation_Value_Gain10;
    extern const prop_static_info_float ColorTransformation_Value_Gain11;
    extern const prop_static_info_float ColorTransformation_Value_Gain12;
    extern const prop_static_info_float ColorTransformation_Value_Gain20;
    extern const prop_static_info_float ColorTransformation_Value_Gain21;
    extern const prop_static_info_float ColorTransformation_Value_Gain22;

    // GenICam standard property
    extern const prop_static_info_integer SensorWidth;
    extern const prop_static_info_integer SensorHeight;

    // strobe stuff
    constexpr const std::array<std::string_view, 2> StrobePolarity_enum_entries = { "ActiveHigh", "ActiveLow" };
    constexpr const std::array<std::string_view, 2> StrobeOperation_enum_entries = { "Exposure", "FixedDuration"};

    extern const prop_static_info_enumeration StrobeEnable; // enum_entries_off_on
    extern const prop_static_info_enumeration StrobePolarity;
    extern const prop_static_info_enumeration StrobeOperation;
    extern const prop_static_info_integer StrobeDuration;
    extern const prop_static_info_integer StrobeDelay;

    // Trigger stuff
    constexpr const std::array<std::string_view, 2> TriggerActivation_enum_entries = { "Rising Edge", "Falling Edge" };
    constexpr const std::array<std::string_view, 4> TriggerSource_enum_entries = { "Line1", "Software", "Action0", "Any" };
    constexpr const std::array<std::string_view, 2> TriggerSelector_enum_entries = { "Frame Start", "Exposure Active" };
    constexpr const std::array<std::string_view, 2> TriggerOperation_enum_entries = { "Default", "Global Reset Release" };
    constexpr const std::array<std::string_view, 2> TriggerOverlap_enum_entries = { "Off", "ReadOut" };

    extern const prop_static_info_enumeration TriggerMode;  // enum_entries_off_on
    extern const prop_static_info_command TriggerSoftware;

    extern const prop_static_info_enumeration TriggerActivation;
    //extern const prop_static_info_enumeration TriggerPolarity;        // replaced with TriggerActivation

    extern const prop_static_info_enumeration TriggerSource;

    extern const prop_static_info_enumeration TriggerSelector;
    //extern const prop_static_info_enumeration TriggerExposureMode;    // replaced with TriggerSelector?
    //extern const prop_static_info_integer TriggerBurstCount;          // replaced with AcquisitionBurstFrameCount
    extern const prop_static_info_enumeration TriggerOperation;
    extern const prop_static_info_enumeration TriggerOverlap;
    extern const prop_static_info_integer AcquisitionBurstFrameCount;
    extern const prop_static_info_integer AcquisitionBurstInterval;

    extern const prop_static_info_float TriggerMask;
    extern const prop_static_info_float TriggerDenoise;
    extern const prop_static_info_float TriggerDelay;
    extern const prop_static_info_float TriggerDebouncer;

    extern const prop_static_info_boolean IMXLowLatencyTriggerMode;

    extern const prop_static_info_integer GPIn;
    extern const prop_static_info_integer GPOut;
    extern const prop_static_info_integer GPIO;

    extern const prop_static_info_boolean LUTEnable;
    extern const prop_static_info_integer LUTIndex;
    extern const prop_static_info_enumeration LUTSelector;
    extern const prop_static_info_integer LUTValue;

    extern const prop_static_info_boolean MultiFrameSetOutputModeCustomGain;
    extern const prop_static_info_boolean MultiFrameSetOutputModeEnable;
    extern const prop_static_info_float MultiFrameSetOutputModeExposureTime0;
    extern const prop_static_info_float MultiFrameSetOutputModeExposureTime1;
    extern const prop_static_info_float MultiFrameSetOutputModeExposureTime2;
    extern const prop_static_info_float MultiFrameSetOutputModeExposureTime3;
    extern const prop_static_info_enumeration MultiFrameSetOutputModeFrameCount;
    extern const prop_static_info_float MultiFrameSetOutputModeGain0;
    extern const prop_static_info_float MultiFrameSetOutputModeGain1;
    extern const prop_static_info_float MultiFrameSetOutputModeGain2;
    extern const prop_static_info_float MultiFrameSetOutputModeGain3;

    extern const prop_static_info_boolean ExpandOutputRange;
    extern const prop_static_info_boolean ShowInfoOverlay;

/* Disable these, because this is a device-specific property, and should always come out of the GenICam document of the device itself.
    extern const prop_static_info_integer ActionQueueSize;
    extern const prop_static_info_command ActionSchedulerCancel;
    extern const prop_static_info_command ActionSchedulerCommit;
    extern const prop_static_info_integer ActionSchedulerInterval;
    extern const prop_static_info_enumeration ActionSchedulerStatus;
    extern const prop_static_info_integer ActionSchedulerTime;
*/

    /* Missing here but present in Tims list
        String          DeviceUserID
        Enumeration     DeviceTemperatureSelector
        Float           DeviceTemperature

        Enumeration     UserSetSelector
        Command         UserSetLoad
        Command         UserSetSave
        Enumeration     UserSetDefault

        Integer         BinningHorizontal
        Integer         BinningVertical
        Integer         DecimationHorizontal
        Integer         DecimationVertical

        Integer         LutValueAll?

        Boolean         IMXLowLatencyTriggerMode
        Integer         AcquisitionBurstInterval
    * */

} // namespace tcamprop1::prop_list
