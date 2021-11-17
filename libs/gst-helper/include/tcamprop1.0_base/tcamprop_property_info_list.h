
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

    constexpr const auto Gain = make_Float( 
        "Gain", 
        "Exposure", "Gain", 
        "Controls the selected gain as an absolute physical value. This is an amplification factor applied to the video signal.", 
        "dB"
    );
    constexpr const auto GainAuto = make_Enumeration( 
        "GainAuto", 
        "Exposure", "Gain Auto", 
        "Sets the automatic gain control (AGC) mode. The exact algorithm used to implement AGC is device-specific."
    );
    constexpr const auto GainAutoLowerLimit = make_Float( 
        "GainAutoLowerLimit", 
        "Exposure", "Gain Auto Lower Limit", 
        "Lower limit of the GainAuto function.", 
        "dB", FloatRepresentation_t::Linear
    );
    constexpr const auto GainAutoUpperLimit = make_Float( 
        "GainAutoUpperLimit", 
        "Exposure", "Gain Auto Upper Limit", 
        "Upper limit of the GainAuto function.", 
        "dB", FloatRepresentation_t::Linear
    );

    constexpr const auto Exposure = make_Float( 
        "ExposureTime", 
        "Exposure", "Exposure Time", 
        "Sets the Exposure time when ExposureMode is Timed and ExposureAuto is Off. This controls the duration where the photosensitive cells are exposed to light.", 
        "us", FloatRepresentation_t::Logarithmic
    );
    constexpr const auto ExposureAuto = make_Enumeration( 
        "ExposureAuto", 
        "Exposure", "Exposure Auto", 
        "Sets the automatic exposure control mode."
    );
    constexpr const auto ExposureAutoReference = make_Integer( 
        "ExposureAutoReference", 
        "Exposure", "Exposure Auto Reference", 
        "Configures the target image brightness of the ExposureAuto/GainAuto algorithm."
    );
    constexpr const auto ExposureAutoLowerLimit = make_Float( 
        "ExposureAutoLowerLimit", 
        "Exposure", "Exposure Auto Lower Limit", 
        "Lower limit of the ExposureAuto function.", 
        "us", FloatRepresentation_t::Logarithmic, Visibility_t::Guru
    );
    constexpr const auto ExposureAutoUpperLimit = make_Float( 
        "ExposureAutoUpperLimit", 
        "Exposure", "Exposure Auto Upper Limit", 
        "Upper limit of the ExposureAuto function.", 
        "us", FloatRepresentation_t::Logarithmic, Visibility_t::Guru
    );
    constexpr const auto ExposureAutoUpperLimitAuto = make_Enumeration( 
        "ExposureAutoUpperLimitAuto",
        "Exposure", "Exposure Auto Upper Limit Auto", 
        "Automatically sets the upper limit to match the Acquisition Frame Rate.", Visibility_t::Expert
    );
    constexpr const auto ExposureAutoHighlightReduction = make_Boolean(  // #TODO the camera has a typo in this name
        "ExposureAutoHighlightReduction",
        "Exposure", "Exposure Auto Highlight Reduction",
        "Lets the ExposureAuto/GainAuto algorithm try to avoid over-exposures."
    );

    constexpr const auto BalanceWhiteAuto = make_Enumeration( 
        "BalanceWhiteAuto", 
        "Color", "Auto White Balance", 
        "Controls the mode for automatic white balancing between the color channels. The white balancing ratios are automatically adjusted."
    );
    
    constexpr const auto BalanceWhiteMode = make_Enumeration(
        "BalanceWhiteMode",
        "Color", "Balance White Mode",
        "#TODO"
    );
    constexpr const auto BalanceWhiteAutoPreset = make_Enumeration(
        "BalanceWhiteAutoPreset",
        "Color", "Balance White Auto Preset",
        "#TODO"
    );
    constexpr const auto BalanceWhiteTemperaturePreset = make_Enumeration(
        "BalanceWhiteTemperaturePreset",
        "Color", "Balance White Temperature Preset",
        "#TODO"
    );

    constexpr const auto BalanceWhiteTemperature = make_Integer(
        "BalanceWhiteTemperature",
        "Color", "Balance White Temperature",
        "#TODO"
    );

    constexpr const auto BalanceWhiteRed = make_Float( 
        "BalanceWhiteRed", 
        "Color", "White Balance Red", 
        "Adjusts the White Balance for the Red color channel."
    );
    constexpr const auto BalanceWhiteGreen = make_Float( 
        "BalanceWhiteGreen", 
        "Color", "White Balance Green", 
        "Adjusts the White Balance for the Green color channel."
    );
    constexpr const auto BalanceWhiteBlue = make_Float( 
        "BalanceWhiteBlue", 
        "Color", "White Balance Blue", 
        "Adjusts the White Balance for the Blue color channel."
    );

    constexpr const auto ClaimBalanceWhiteSoftware = make_Boolean(
        "ClaimBalanceWhiteSoftware", 
        "Color", {}, {}, Visibility_t::Invisible
    );
    constexpr const auto ClaimHDRGain = make_Boolean(
        "ClaimHDRGain", 
        "Image", {}, {}, Visibility_t::Invisible
    );
    constexpr const auto HDRGain = make_Float(
        "HDRGain", 
        "Image", "HDR Gain",
        "#TODO"
    );
    constexpr const auto HDRGainAuto = make_Boolean(
        "HDRGainAuto", 
        "Image", "Auto HDR Gain",
        "Enables the automatic HDR Gain value selection."
    );
    constexpr const auto HDRGainAutoReference = make_Float(
        "HDRGainAutoReference", 
        "Image", "Auto HDR Gain reference",
        "Changes the reference point used when Auto HDR Gain is enabled.", {}, {}, Visibility_t::Expert
    );

    constexpr const auto BlackLevel = make_Float(
        "BlackLevel", 
        "Exposure", "Black Level",
        "Controls the analog black level as an absolute physical value. This represents a DC offset applied to the video signal."
    );

    constexpr const auto OffsetX = make_Integer(
        "OffsetX", 
        "Partial Scan", "Offset X",
        "Horizontal offset from the origin to the region of interest (in pixels)."
    );
    constexpr const auto OffsetY = make_Integer(
        "OffsetY", 
        "Partial Scan", "Offset Y",
        "Vertical offset from the origin to the region of interest (in pixels)."
    );
    constexpr const auto OffsetAutoCenter = make_Enumeration(
        "OffsetAutoCenter", 
        "Partial Scan", "Offset Auto Center",
        "Enables automatic centering of the region of interest area to the sensor when using smaller video dimensions."
    );
    constexpr const auto ReverseX = make_Boolean(
        "ReverseX", 
        "Image", "Reverse X",
        "Flip horizontally the image sent by the device. The Region of interest is applied after the flipping.",
        Visibility_t::Expert
    );
    constexpr const auto ReverseY = make_Boolean(
        "ReverseY", 
        "Image", "Reverse Y",
        "Flip vertically the image sent by the device. The Region of interest is applied after the flipping.",
        Visibility_t::Expert
    );
    constexpr const auto TriggerMode = make_Enumeration(
        "TriggerMode", 
        "Special", "Trigger Mode",
        "Controls if the selected trigger is active."
    );
    constexpr const auto TriggerSource = make_Enumeration(
        "TriggerSource", 
        "Special", "Trigger Source",
        "Specifies the internal signal or physical input Line to use as the trigger source."
    );
    constexpr const auto TriggerSoftware = make_Command(
        "TriggerSoftware",
        "Special", "Trigger Software",
        "Generates an internal trigger. TriggerSource must be set to Software."
    );
    constexpr const auto TriggerPolarity = make_Enumeration(
        "TriggerPolarity",
        "Special", "Trigger Polarity",
        "#TODO"
    );
    constexpr const auto TriggerExposureMode = make_Enumeration(
        "TriggerExposureMode",
        "Special", "Trigger Exposure Mode",
        "#TODO"
    );
    constexpr const auto TriggerBurstCount = make_Integer(
        "TriggerBurstCount",
        "Special",
        "Trigger Burst Count",
        "#TODO"
    );
    constexpr const std::array<std::string_view, 2> TriggerOperation_EnumEntries = {
        "Default", "GlobalResetRelease"
    };

    constexpr const auto TriggerOperation = make_Enumeration(
        "TriggerOperation",
        "Special", "Trigger Operation",
        "#TODO"
    );
    // #TODO revisit this
    //constexpr const auto strobe_mode = make_Enumeration( // maybe BAD??
    //    "StrobeMode", 
    //    "Special", "Strobe Mode",
    //    "#TODO"
    //);

    constexpr const auto Iris = make_Integer(
        "Iris",
        "Lens", "Iris",
        "Changes the Iris setting of the lens."
    );

    constexpr const auto IrisAuto = make_Enumeration( 
        "IrisAuto",
        "Lens", "Auto Iris",
        "Enables the automatic Iris setting selection." // #TODO maybe improve this
    );

    constexpr const auto Focus = make_Integer(
        "Focus",
        "Lens", "Focus",
        "Changes the Focus setting of the lens."
    );

    constexpr const auto FocusAuto = make_Enumeration(
        "FocusAuto",
        "Lens", "Auto Focus",
        "Enables the automatic Focus setting selection." // #TODO maybe improve this
    );

    constexpr const auto AutoFocusROIEnable = make_Enumeration(
        "AutoFocusROIEnable",
        "Lens", "Auto Focus ROI Enable",
        "Enables the automatic Focus setting selection." // #TODO maybe improve this
    );

    constexpr const auto AutoFocusROILeft = make_Integer(
        "AutoFocusROILeft",
        "Lens", "AutoFocusROILeft",
        "#TODO" // #TODO maybe improve this
    );
    constexpr const auto AutoFocusROITop = make_Integer(
        "AutoFocusROITop",
        "Lens", "AutoFocusROITop",
        "#TODO" // #TODO maybe improve this
    );
    constexpr const auto AutoFocusROIWidth = make_Integer(
        "AutoFocusROIWidth",
        "Lens", "AutoFocusROIWidth",
        "#TODO" // #TODO maybe improve this
    );
    constexpr const auto AutoFocusROIHeight = make_Integer(
        "AutoFocusROIHeight",
        "Lens", "AutoFocusROIHeight",
        "#TODO" // #TODO maybe improve this
    );

    constexpr const auto Zoom = make_Integer(
        "Zoom",
        "Lens", "Zoom",
        "Changes the zoom setting of the lens."
    );

    constexpr const auto IRCutFilterEnable = make_Integer(
        "IRCutFilterEnable",
        "Lens", "IRCut Filter Enable",
        "Enables the IRCutFilter in from of the sensor."
    );

    const constexpr std::array<std::string_view, 6> AutoFunctionsROIPreset_EnumEntries = {
        "Full Sensor", "Custom Rectangle", "Center 50%", "Center 25%", "Bottom Half", "Top Half",
    };

    constexpr const auto AutoFunctionsROIEnable = make_Boolean(
        "AutoFunctionsROIEnable",
        "Auto ROI", "Enable Auto Functions ROI",
        "Enable the region of interest for auto functions."
    );
    constexpr const auto AutoFunctionsROIPreset = make_Enumeration(
        "AutoFunctionsROIPreset",
        "Auto ROI", "Auto Functions ROI Preset",
        "Select a predefined region of interest for auto functions"
    );
    constexpr const auto AutoFunctionsROIHeight = make_Integer(
        "AutoFunctionsROIHeight",
        "Auto ROI", "Auto Functions ROI Height",
        "Vertical size of the auto functions region of interest."
    );
    constexpr const auto AutoFunctionsROIWidth = make_Integer(
        "AutoFunctionsROIWidth",
        "Auto ROI", "Auto Functions ROI Width",
        "Horizontal size of the auto functions region of interest."
    );
    constexpr const auto AutoFunctionsROITop = make_Integer(
        "AutoFunctionsROITop",
        "Auto ROI", "Auto Functions ROI Top",
        "Vertical offset of the auto functions region of interest."
    );
    constexpr const auto AutoFunctionsROILeft = make_Integer(
        "AutoFunctionsROILeft",
        "Auto ROI", "Auto Functions ROI Left",
        "Horizontal offset of the auto functions region of interest."
    );

    constexpr const auto Denoise = make_Integer(
        "Denoise",
        "Image", "Denoise",
        "Controls the strength of the denoise algorithm."
    );
    constexpr const auto Sharpness = make_Integer(
        "Sharpness",
        "Image", "Sharpness",
        "Controls the strength of the sharpness algorithm."
    );

    constexpr const auto SoftwareBrightness = make_Float(
        "SoftwareBrightness",
        "Image", "Software Brightness",
        "Controls the value which gets added to the digital brightness."
    );
    constexpr const auto Contrast = make_Integer(
        "Contrast",
        "Image", "Contrast",
        "Controls the strength of the contrast algorithm."
    );
    constexpr const auto Gamma = make_Float(
        "Gamma",
        "Image", "Gamma",
        "Controls the gamma correction of pixel intensity. This is typically used to compensate for non-linearity of the display system (such as CRT)."
    );
    constexpr const auto Saturation = make_Float(
        "Saturation",
        "Color", "Saturation",
        "Controls the saturation of the image.",
        "%"
    );
    constexpr const auto Hue = make_Float(
        "Hue",
        "Color", "Hue",
        "Controls the hue of the image.",
        "°"
    );
    constexpr const auto TonemappingEnable = make_Boolean(
        "TonemappingEnable",
        "WDR", "Tonemapping Enable",
        "Enables the tonemapping algorithm."
    );
    constexpr const auto TonemappingGlobalBrightness = make_Float(
        "TonemappingGlobalBrightness",
        "WDR", "Tonemapping Global Brightness",
        "Changes the brightness reference used for a individual pixel, which is interpolated between local and global."
    );
    constexpr const auto TonemappingIntensity = make_Float(
        "TonemappingIntensity",
        "WDR", "Tonemapping Intensity",
        "Adjusts the intensity of the tonemapping algorithm."
    );

    //constexpr const auto    ColorTransformationSelector = make_Enumeration(
    //    "ColorTransformationSelector",
    //    "Color", 
    //);

    const constexpr auto ColorTransformationEnable = make_Boolean( 
        "ColorTransformationEnable",
        "Color Correction", "Color Transformation Enable",
        "Activates the selected Color Transformation module."
    );

    const constexpr auto ColorTransformation_Value_Gain00 = make_Float(
        "ColorTransformation_Value_Gain00",
        "Color Correction", "Color Transformation Value Gain00",
        "Changes the color transformation for one factor on a pixel.", Visibility_t::Guru
    );

    const constexpr auto ColorTransformation_Value_Gain01 = make_Float(
        "ColorTransformation_Value_Gain01",
        "Color Correction", "Color Transformation Value Gain01",
        "Changes the color transformation for one factor on a pixel.", Visibility_t::Guru
    );

    const constexpr auto ColorTransformation_Value_Gain02 = make_Float(
        "ColorTransformation_Value_Gain02",
        "Color Correction", "Color Transformation Value Gain02",
        "Changes the color transformation for one factor on a pixel.", Visibility_t::Guru
    );

    const constexpr auto ColorTransformation_Value_Gain10 = make_Float(
        "ColorTransformation_Value_Gain10",
        "Color Correction", "Color Transformation Value Gain10",
        "Changes the color transformation for one factor on a pixel.", Visibility_t::Guru
    );

    const constexpr auto ColorTransformation_Value_Gain11 = make_Float(
        "ColorTransformation_Value_Gain11",
        "Color Correction", "Color Transformation Value Gain11",
        "Changes the color transformation for one factor on a pixel.", Visibility_t::Guru
    );

    const constexpr auto ColorTransformation_Value_Gain12 = make_Float(
        "ColorTransformation_Value_Gain12",
        "Color Correction", "Color Transformation Value Gain12",
        "Changes the color transformation for one factor on a pixel.", Visibility_t::Guru
    );

    const constexpr auto ColorTransformation_Value_Gain20 = make_Float(
        "ColorTransformation_Value_Gain20",
        "Color Correction", "Color Transformation Value Gain20",
        "Changes the color transformation for one factor on a pixel.", Visibility_t::Guru
    );

    const constexpr auto ColorTransformation_Value_Gain21 = make_Float(
        "ColorTransformation_Value_Gain21",
        "Color Correction", "Color Transformation Value Gain21",
        "Changes the color transformation for one factor on a pixel.", Visibility_t::Guru
    );

    const constexpr auto ColorTransformation_Value_Gain22 = make_Float(
        "ColorTransformation_Value_Gain22",
        "Color Correction", "Color Transformation Value Gain22",
        "Changes the color transformation for one factor on a pixel.", Visibility_t::Guru
    );


    //const constexpr auto MultiFrameSetOutputModeEnable = make_Boolean(
    //    "MultiFrameSetOutputModeEnable",
    //    "Color Correction", "MultiFrameSet Output Mode Enable",
    //    "#TODO"
    //);

    // Off/On
    constexpr const auto StrobeEnable = make_Enumeration(
        "StrobeEnable",
        "Special", "Strobe Enable",
        "Enables the strobe controls."
    );

    constexpr const auto StrobePolarity = make_Enumeration(
        "StrobePolarity",
        "Special", "Strobe Polarity",
        "Controls the polarity of the strobe signal."
    );

    constexpr const auto StrobeOperation = make_Enumeration(
        "StrobeOperation",
        "Special", "Strobe Operation",
        "#TODO" // #TODO maybe improve this
    );

    constexpr const auto StrobeDuration = make_Integer(
        "StrobeDuration",
        "Special", "Strobe Duration",
        "#TODO" // #TODO maybe improve this
    );

    constexpr const auto StrobeDelay = make_Integer(
        "StrobeDelay",
        "Special", "Strobe Delay",
        "#TODO" // #TODO maybe improve this
    );

    // GenICam standard property
    constexpr const auto SensorWidth = make_Integer(
        "SensorWidth",
        "Sensor", "Sensor Width",
        "Effective width of the sensor in pixels.", {}, IntRepresentation_t::PureNumber, Visibility_t::Expert, Access_t::RO
    );

    // GenICam standard property
    constexpr const auto SensorHeight = make_Integer(
        "SensorHeight",
        "Sensor", "Sensor Height",
        "Effective height of the sensor in pixels.", {}, IntRepresentation_t::PureNumber, Visibility_t::Expert, Access_t::RO
    );

    constexpr const auto TriggerActivation = make_Enumeration(
        "TriggerActivation",
        "Special", "Trigger Activation",
        "The Trigger Polarity parameter controls whether a trigger event is accepted on the rising or falling edge of the signal connected to the TRIGGER_IN line."
    );

    constexpr const auto TriggerSelector = make_Enumeration(
        "TriggerSelector",
        "Special", "Trigger Selector",
        "#TODO"
    );

    constexpr const auto TriggerOverlap = make_Enumeration(
        "TriggerOverlap",
        "Special", "Trigger Overlap",
        "Specifies the type trigger overlap permitted with the previous frame or line. This defines when a valid trigger will be accepted (or latched) for a new frame or a new line."
    );

    constexpr const auto TriggerMask = make_Float(
        "TriggerMask",
        "Special", "Trigger Mask",
        "Specifies the time for which trigger pulses are ignored after accepting a trigger signal."
    );

    constexpr const auto TriggerDenoise = make_Float(
        "TriggerDenoise",
        "Special", "Trigger Denoise",
        "#TODO"
    );

    constexpr const auto TriggerDelay = make_Float(
        "TriggerDelay",
        "Special", "Trigger Delay",
        "Specifies the delay in microseconds (us) to apply after the trigger reception before activating it.",
        "Âµs"
    );

    constexpr const auto TriggerDebouncer = make_Float(
        "TriggerDebouncer",
        "Special", "Trigger Debouncer",
        "#TODO"
    );

    constexpr const auto GPIn = make_Integer(
        "GPIn",
        "DigitalIO", "General Purpose Input",
        "#TODO"
    );

    constexpr const auto GPOut = make_Integer(
        "GPOut",
        "DigitalIO", "General Purpose Output",
        "#TODO"
    );

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

        Integer         GPIn
        Integer         GPOut
        Boolean         LUTEnable
        Integer         LUTIndex, LutValue, LutValueAll?
        
        MultiFrameSet* stuff
        Enumeration     TriggerSelector
        Boolean         IMXLowLatencyTriggerMode
        Float           TriggerDelay
        Float           TriggerDebouncer
        Float           TriggerMask
        Float           TriggerDenoise
        Enumeration     TriggerOverlap
        Integer         AcquisitionBurstFrameCount
        Integer         AcquisitionBurstInterval
    * */

} // namespace tcamprop1::prop_list
