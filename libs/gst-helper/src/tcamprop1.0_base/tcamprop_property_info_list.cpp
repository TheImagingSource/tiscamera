
#include "tcamprop1.0_base/tcamprop_property_info_list.h"

namespace lst = tcamprop1::prop_list;

using namespace tcamprop1;

const prop_static_info_float lst::Gain = make_Float(
    "Gain",
    "Exposure", "Gain",
    "Controls the selected gain as an absolute physical value. This is an amplification factor applied to the video signal.",
    "dB"
);

const prop_static_info_enumeration lst::GainAuto = make_Enumeration(
    "GainAuto",
    "Exposure", "Gain Auto",
    "Sets the automatic gain control (AGC) mode. The exact algorithm used to implement AGC is device-specific."
);
const prop_static_info_float lst::GainAutoLowerLimit = make_Float(
    "GainAutoLowerLimit",
    "Exposure", "Gain Auto Lower Limit",
    "Lower limit of the GainAuto function.",
    "dB", FloatRepresentation_t::Linear
);
const prop_static_info_float lst::GainAutoUpperLimit = make_Float(
    "GainAutoUpperLimit",
    "Exposure", "Gain Auto Upper Limit",
    "Upper limit of the GainAuto function.",
    "dB", FloatRepresentation_t::Linear
);

const prop_static_info_float lst::ExposureTime = make_Float(
    "ExposureTime",
    "Exposure", "Exposure Time",
    "Sets the Exposure time when ExposureMode is Timed and ExposureAuto is Off. This controls the duration where the photosensitive cells are exposed to light.",
    "µs", FloatRepresentation_t::Logarithmic
);
const prop_static_info_enumeration lst::ExposureAuto = make_Enumeration(
    "ExposureAuto",
    "Exposure", "Exposure Auto",
    "Sets the automatic exposure control mode."
);
const prop_static_info_integer lst::ExposureAutoReference = make_Integer(
    "ExposureAutoReference",
    "Exposure", "Exposure Auto Reference",
    "Configures the target image brightness of the ExposureAuto/GainAuto algorithm."
);
const prop_static_info_float lst::ExposureAutoLowerLimit = make_Float(
    "ExposureAutoLowerLimit",
    "Exposure", "Exposure Auto Lower Limit",
    "Lower limit of the ExposureAuto function.",
    "µs", FloatRepresentation_t::Logarithmic, Visibility_t::Guru
);
const prop_static_info_float lst::ExposureAutoUpperLimit = make_Float(
    "ExposureAutoUpperLimit",
    "Exposure", "Exposure Auto Upper Limit",
    "Upper limit of the ExposureAuto function.",
    "µs", FloatRepresentation_t::Logarithmic, Visibility_t::Guru
);
const prop_static_info_enumeration lst::ExposureAutoUpperLimitAuto = make_Enumeration(
    "ExposureAutoUpperLimitAuto",
    "Exposure", "Exposure Auto Upper Limit Auto",
    "Automatically sets the upper limit to match the Acquisition Frame Rate.", Visibility_t::Expert
);
const prop_static_info_boolean lst::ExposureAutoHighlightReduction = make_Boolean(
    "ExposureAutoHighlightReduction",
    "Exposure", "Exposure Auto Highlight Reduction",
    "Lets the ExposureAuto/GainAuto algorithm try to avoid over-exposures."
);

const prop_static_info_enumeration lst::BalanceWhiteAuto = make_Enumeration(
    "BalanceWhiteAuto",
    "Color", "Auto White Balance",
    "Controls the mode for automatic white balancing between the color channels. The white balancing ratios are automatically adjusted."
);

const prop_static_info_enumeration lst::BalanceWhiteMode = make_Enumeration(
    "BalanceWhiteMode",
    "Color", "Balance White Mode",
    "Controls the WhiteBalance mode selected"
);
const prop_static_info_enumeration lst::BalanceWhiteAutoPreset = make_Enumeration(
    "BalanceWhiteAutoPreset",
    "Color", "Balance White Auto Preset",
    "Use to select a specific whitebalance preset."
);
const prop_static_info_enumeration lst::BalanceWhiteTemperaturePreset = make_Enumeration(
    "BalanceWhiteTemperaturePreset",
    "Color", "Balance White Temperature Preset",
    "Use to select a specific temperature preset."
);

const prop_static_info_integer lst::BalanceWhiteTemperature = make_Integer(
    "BalanceWhiteTemperature",
    "Color", "Balance White Temperature",
    "Adjusts the White Balance controls according to the light temperature."
);

const prop_static_info_float lst::BalanceWhiteRed = make_Float(
    "BalanceWhiteRed",
    "Color", "White Balance Red",
    "Adjusts the White Balance for the Red color channel."
);
const prop_static_info_float lst::BalanceWhiteGreen = make_Float(
    "BalanceWhiteGreen",
    "Color", "White Balance Green",
    "Adjusts the White Balance for the Green color channel."
);
const prop_static_info_float lst::BalanceWhiteBlue = make_Float(
    "BalanceWhiteBlue",
    "Color", "White Balance Blue",
    "Adjusts the White Balance for the Blue color channel."
);

const prop_static_info_boolean lst::ClaimBalanceWhiteSoftware = make_Boolean(
    "ClaimBalanceWhiteSoftware",
    "Color", {}, {}, Visibility_t::Invisible
);
const prop_static_info_boolean lst::ClaimHDRGain = make_Boolean(
    "ClaimHDRGain",
    "Exposure", {}, {}, Visibility_t::Invisible
);
const prop_static_info_float lst::HDRGain = make_Float(
    "HDRGain",
    "Exposure", "HDR Gain",
    "Controls the HDR window selection."
);
const prop_static_info_boolean lst::HDRGainAuto = make_Boolean(
    "HDRGainAuto",
    "Exposure", "Auto HDR Gain",
    "Enables the automatic HDR Gain value selection."
);
const prop_static_info_float lst::HDRGainAutoReference = make_Float(
    "HDRGainAutoReference",
    "Exposure", "Auto HDR Gain reference",
    "Changes the reference point used when Auto HDR Gain is enabled.", {}, {}, Visibility_t::Expert
);

const prop_static_info_float lst::BlackLevel = make_Float(
    "BlackLevel",
    "Exposure", "Black Level",
    "Controls the analog black level as an absolute physical value. This represents a DC offset applied to the video signal."
);

const prop_static_info_integer lst::OffsetX = make_Integer(
    "OffsetX",
    "Partial Scan", "Offset X",
    "Horizontal offset from the origin to the region of interest (in pixels)."
);
const prop_static_info_integer lst::OffsetY = make_Integer(
    "OffsetY",
    "Partial Scan", "Offset Y",
    "Vertical offset from the origin to the region of interest (in pixels)."
);
const prop_static_info_enumeration lst::OffsetAutoCenter = make_Enumeration(
    "OffsetAutoCenter",
    "Partial Scan", "Offset Auto Center",
    "Enables automatic centering of the region of interest area to the sensor when using smaller video dimensions."
);
const prop_static_info_boolean lst::ReverseX = make_Boolean(
    "ReverseX",
    "Image", "Reverse X",
    "Flip horizontally the image sent by the device. The Region of interest is applied after the flipping.",
    Visibility_t::Expert
);
const prop_static_info_boolean lst::ReverseY = make_Boolean(
    "ReverseY",
    "Image", "Reverse Y",
    "Flip vertically the image sent by the device. The Region of interest is applied after the flipping.",
    Visibility_t::Expert
);
const prop_static_info_integer lst::Iris = make_Integer(
    "Iris",
    "Lens", "Iris",
    "Changes the Iris setting of the lens."
);

const prop_static_info_enumeration lst::IrisAuto = make_Enumeration(
    "IrisAuto",
    "Lens", "Auto Iris",
    "Enables the automatic Iris setting selection." 
);

const prop_static_info_integer lst::Focus = make_Integer(
    "Focus",
    "Lens", "Focus",
    "Changes the Focus setting of the lens."
);

const prop_static_info_enumeration lst::FocusAuto = make_Enumeration(
    "FocusAuto",
    "Lens", "Auto Focus",
    "Enables the automatic Focus control."
);

const prop_static_info_enumeration lst::AutoFocusROIEnable = make_Enumeration(
    "AutoFocusROIEnable",
    "Lens", "Auto Focus ROI Enable",
    "Enables selection of the auto focus region."
);

const prop_static_info_integer lst::AutoFocusROILeft = make_Integer(
    "AutoFocusROILeft",
    "Lens", "Auto Focus ROI Left",
    "Horizontal offset of the auto functions region of interest."
);
const prop_static_info_integer lst::AutoFocusROITop = make_Integer(
    "AutoFocusROITop",
    "Lens", "Auto Focus ROI Top",
    "Vertical offset of the auto functions region of interest."
);
const prop_static_info_integer lst::AutoFocusROIWidth = make_Integer(
    "AutoFocusROIWidth",
    "Lens", "Auto Focus ROI Width",
    "Horizontal size of the auto focus region of interest."
);
const prop_static_info_integer lst::AutoFocusROIHeight = make_Integer(
    "AutoFocusROIHeight",
    "Lens", "Auto Focus ROI Height",
    "Vertical size of the auto focus region of interest."
);

const prop_static_info_integer lst::Zoom = make_Integer(
    "Zoom",
    "Lens", "Zoom",
    "Changes the zoom setting of the lens."
);

const prop_static_info_integer lst::IRCutFilterEnable = make_Integer(
    "IRCutFilterEnable",
    "Lens", "IRCut Filter Enable",
    "Enables the IRCutFilter in from of the sensor."
);

const prop_static_info_boolean lst::AutoFunctionsROIEnable = make_Boolean(
    "AutoFunctionsROIEnable",
    "Auto ROI", "Enable Auto Functions ROI",
    "Enable the region of interest for auto functions."
);
const prop_static_info_enumeration lst::AutoFunctionsROIPreset = make_Enumeration(
    "AutoFunctionsROIPreset",
    "Auto ROI", "Auto Functions ROI Preset",
    "Select a predefined region of interest for auto functions"
);
const prop_static_info_integer lst::AutoFunctionsROIHeight = make_Integer(
    "AutoFunctionsROIHeight",
    "Auto ROI", "Auto Functions ROI Height",
    "Vertical size of the auto functions region of interest."
);
const prop_static_info_integer lst::AutoFunctionsROIWidth = make_Integer(
    "AutoFunctionsROIWidth",
    "Auto ROI", "Auto Functions ROI Width",
    "Horizontal size of the auto functions region of interest."
);
const prop_static_info_integer lst::AutoFunctionsROITop = make_Integer(
    "AutoFunctionsROITop",
    "Auto ROI", "Auto Functions ROI Top",
    "Vertical offset of the auto functions region of interest."
);
const prop_static_info_integer lst::AutoFunctionsROILeft = make_Integer(
    "AutoFunctionsROILeft",
    "Auto ROI", "Auto Functions ROI Left",
    "Horizontal offset of the auto functions region of interest."
);

const prop_static_info_integer lst::Denoise = make_Integer(
    "Denoise",
    "Image", "Denoise",
    "Controls the strength of the denoise algorithm."
);
const prop_static_info_integer lst::Sharpness = make_Integer(
    "Sharpness",
    "Image", "Sharpness",
    "Controls the strength of the sharpness algorithm."
);

const prop_static_info_float lst::SoftwareBrightness = make_Float(
    "SoftwareBrightness",
    "Image", "Software Brightness",
    "Controls the value which gets added to the digital brightness."
);
const prop_static_info_integer lst::Contrast = make_Integer(
    "Contrast",
    "Image", "Contrast",
    "Controls the strength of the contrast algorithm."
);
const prop_static_info_float lst::Gamma = make_Float(
    "Gamma",
    "Image", "Gamma",
    "Controls the gamma correction of pixel intensity. This is typically used to compensate for non-linearity of the display system (such as CRT)."
);
const prop_static_info_float lst::Saturation = make_Float(
    "Saturation",
    "Color", "Saturation",
    "Controls the saturation of the image.",
    "%"
);
const prop_static_info_float lst::Hue = make_Float(
    "Hue",
    "Color", "Hue",
    "Controls the hue of the image.",
    "°"
);
const prop_static_info_boolean lst::TonemappingEnable = make_Boolean(
    "TonemappingEnable",
    "WDR", "Tonemapping Enable",
    "Enables the tonemapping algorithm."
);
const prop_static_info_float lst::TonemappingGlobalBrightness = make_Float(
    "TonemappingGlobalBrightness",
    "WDR", "Tonemapping Global Brightness",
    "Changes the brightness reference used for a individual pixel, which is interpolated between local and global."
);
const prop_static_info_float lst::TonemappingIntensity = make_Float(
    "TonemappingIntensity",
    "WDR", "Tonemapping Intensity",
    "Adjusts the intensity of the tonemapping algorithm."
);

const prop_static_info_boolean lst::ColorTransformationEnable = make_Boolean(
    "ColorTransformationEnable",
    "Color Correction", "Color Transformation Enable",
    "Activates the selected Color Transformation module."
);

const prop_static_info_float lst::ColorTransformation_Value_Gain00 = make_Float(
    "ColorTransformation_Value_Gain00",
    "Color Correction", "Color Transformation Value Gain00",
    "Changes the color transformation for one factor on a pixel.", Visibility_t::Guru
);

const prop_static_info_float lst::ColorTransformation_Value_Gain01 = make_Float(
    "ColorTransformation_Value_Gain01",
    "Color Correction", "Color Transformation Value Gain01",
    "Changes the color transformation for one factor on a pixel.", Visibility_t::Guru
);

const prop_static_info_float lst::ColorTransformation_Value_Gain02 = make_Float(
    "ColorTransformation_Value_Gain02",
    "Color Correction", "Color Transformation Value Gain02",
    "Changes the color transformation for one factor on a pixel.", Visibility_t::Guru
);

const prop_static_info_float lst::ColorTransformation_Value_Gain10 = make_Float(
    "ColorTransformation_Value_Gain10",
    "Color Correction", "Color Transformation Value Gain10",
    "Changes the color transformation for one factor on a pixel.", Visibility_t::Guru
);

const prop_static_info_float lst::ColorTransformation_Value_Gain11 = make_Float(
    "ColorTransformation_Value_Gain11",
    "Color Correction", "Color Transformation Value Gain11",
    "Changes the color transformation for one factor on a pixel.", Visibility_t::Guru
);

const prop_static_info_float lst::ColorTransformation_Value_Gain12 = make_Float(
    "ColorTransformation_Value_Gain12",
    "Color Correction", "Color Transformation Value Gain12",
    "Changes the color transformation for one factor on a pixel.", Visibility_t::Guru
);

const prop_static_info_float lst::ColorTransformation_Value_Gain20 = make_Float(
    "ColorTransformation_Value_Gain20",
    "Color Correction", "Color Transformation Value Gain20",
    "Changes the color transformation for one factor on a pixel.", Visibility_t::Guru
);

const prop_static_info_float lst::ColorTransformation_Value_Gain21 = make_Float(
    "ColorTransformation_Value_Gain21",
    "Color Correction", "Color Transformation Value Gain21",
    "Changes the color transformation for one factor on a pixel.", Visibility_t::Guru
);

const prop_static_info_float lst::ColorTransformation_Value_Gain22 = make_Float(
    "ColorTransformation_Value_Gain22",
    "Color Correction", "Color Transformation Value Gain22",
    "Changes the color transformation for one factor on a pixel.", Visibility_t::Guru
);

const prop_static_info_integer lst::SensorWidth = make_Integer(
    "SensorWidth",
    "Sensor", "Sensor Width",
    "Effective width of the sensor in pixels.", {}, IntRepresentation_t::PureNumber, Visibility_t::Expert, Access_t::RO
);

const prop_static_info_integer lst::SensorHeight = make_Integer(
    "SensorHeight",
    "Sensor", "Sensor Height",
    "Effective height of the sensor in pixels.", {}, IntRepresentation_t::PureNumber, Visibility_t::Expert, Access_t::RO
);


const prop_static_info_enumeration lst::StrobeEnable = make_Enumeration(
    "StrobeEnable",
    "DigitalIO", "Strobe Enable",
    "Enables the strobe controls."
);

const prop_static_info_enumeration lst::StrobePolarity = make_Enumeration(
    "StrobePolarity",
    "DigitalIO", "Strobe Polarity",
    "Controls the polarity of the strobe signal."
);

const prop_static_info_enumeration lst::StrobeOperation = make_Enumeration(
    "StrobeOperation",
    "DigitalIO", "Strobe Operation",
    "Specifies how the length of the strobe pulses is controlled."
);

const prop_static_info_integer lst::StrobeDuration = make_Integer(
    "StrobeDuration",
    "DigitalIO", "Strobe Duration",
    "Controls the length of the strobe pulses if Strobe Operation is set to use the fixed duration mode.",
    "µs"
);

const prop_static_info_integer lst::StrobeDelay = make_Integer(
    "StrobeDelay",
    "DigitalIO", "Strobe Delay",
    "Parameter can be used to add a small delay between the start of exposure and the strobe output pulse.",
    "µs"
);

const prop_static_info_enumeration lst::TriggerMode = make_Enumeration(
    "TriggerMode",
    "Special", "Trigger Mode",
    "Controls if the selected trigger is active."
);
const prop_static_info_command lst::TriggerSoftware = make_Command(
    "TriggerSoftware",
    "Special", "Trigger Software",
    "Generates an internal trigger. TriggerSource must be set to Software."
);
const prop_static_info_enumeration lst::TriggerSource = make_Enumeration(
    "TriggerSource",
    "Special", "Trigger Source",
    "Specifies the internal signal or physical input Line to use as the trigger source."
);
//const prop_static_info_enumeration lst::TriggerPolarity = make_Enumeration(
//    "TriggerPolarity",
//    "Special", "Trigger Polarity",
//    "#TODO"
//);
const prop_static_info_enumeration lst::TriggerSelector = make_Enumeration(
    "TriggerSelector",
    "Special", "Trigger Selector",
    "Controls the way in which the exposure time is controlled in trigger mode."
);
//const prop_static_info_enumeration lst::TriggerExposureMode = make_Enumeration(
//    "TriggerExposureMode",
//    "Special", "Trigger Exposure Mode",
//    "#TODO"
//);

const prop_static_info_integer lst::TriggerBurstCount = make_Integer(
    "TriggerBurstCount",
    "Special",
    "Trigger Burst Count",
    "Controls the count of frames to generate per trigger pulse."
);

const prop_static_info_enumeration lst::TriggerOperation = make_Enumeration(
    "TriggerOperation",
    "Special", "Trigger Operation",
    "Controls the operation mode of the sensor in trigger mode."
);

const prop_static_info_enumeration lst::TriggerActivation = make_Enumeration(
    "TriggerActivation",
    "Special", "Trigger Activation",
    "The Trigger Polarity parameter controls whether a trigger event is accepted on the rising or falling edge of the signal connected to the TRIGGER_IN line."
);

const prop_static_info_enumeration lst::TriggerOverlap = make_Enumeration(
    "TriggerOverlap",
    "Special", "Trigger Overlap",
    "Specifies the type trigger overlap permitted with the previous frame or line. This defines when a valid trigger will be accepted (or latched) for a new frame or a new line."
);

const prop_static_info_float lst::TriggerMask = make_Float(
    "TriggerMask",
    "Special", "Trigger Mask",
    "Specifies the time for which trigger pulses are ignored after accepting a trigger signal.",
    "µs"
);

const prop_static_info_float lst::TriggerDenoise = make_Float(
    "TriggerDenoise",
    "Special", "Trigger Denoise",
    "Specifies the time for which trigger input has to be high in order to be accepted as a trigger signal.",
    "µs"
);

const prop_static_info_float lst::TriggerDelay = make_Float(
    "TriggerDelay",
    "Special", "Trigger Delay",
    "Specifies the delay in microseconds (us) to apply after the trigger reception before activating it.",
    "µs"
);

const prop_static_info_float lst::TriggerDebouncer = make_Float(
    "TriggerDebouncer",
    "Special", "Trigger Debouncer",
    "Specifies the time for which trigger input has to be low in order accept the next trigger signal.",
    "µs"
);

const prop_static_info_boolean lst::IMXLowLatencyTriggerMode = make_Boolean(
    "IMXLowLatencyTriggerMode",
    "Special", "IMX Low-Latency Mode",
    "controls whether the sensor operates in low-latency trigger mode"
);

const prop_static_info_integer lst::GPIn = make_Integer(
    "GPIn",
    "DigitalIO", "General Purpose Input",
    "Status of the digital input pin."
);

const prop_static_info_integer lst::GPOut = make_Integer(
    "GPOut",
    "DigitalIO", "General Purpose Output",
    "Status of the digital output pin."
);