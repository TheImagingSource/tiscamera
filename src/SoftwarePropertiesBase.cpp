

#include "SoftwarePropertiesBase.h"

using sp = tcam::property::emulated::software_prop;

namespace tcam::property::emulated
{

const std::string_view find_property_name(sp id)
{
    switch (id)
    {
        case sp::Invalid:
            return "Invalid";
        case sp::ExposureTime:
            return "ExposureTime";
        case sp::ExposureAuto:
            return "ExposureAuto";
        case sp::ExposureAutoLowerLimit:
            return "ExposureAutoLowerLimit";
        case sp::ExposureAutoUpperLimit:
            return "ExposureAutoUpperLimit";
        case sp::ExposureAutoReference:
            return "ExposureAutoReference";
        case sp::ExposureAutoUpperLimitAuto:
            return "ExposureAutoUpperLimitAuto";
        case sp::Gain:
            return "Gain";
        case sp::GainAuto:
            return "GainAuto";
        case sp::GainAutoLowerLimit:
            return "GainAutoLowerLimit";
        case sp::GainAutoUpperLimit:
            return "GainAutoUpperLimit";
        case sp::Iris:
            return "Iris";
        case sp::IrisAuto:
            return "IrisAuto";
        case sp::Focus:
            return "Focus";
        case sp::FocusAuto:
            return "FocusAuto";
        case sp::HighlightReduction:
            return "HighlightReduction";
        case sp::WB:
            return "WB";
        case sp::WB_AUTO:
            return "WB_AUTO";
        case sp::WB_RED:
            return "WB_RED";
        case sp::WB_GREEN:
            return "WB_GREEN";
        case sp::WB_BLUE:
            return "WB_BLUE";
        case sp::WB_ONCE:
            return "WB_ONCE";
        case sp::Denoise:
            return "Denoise";
        case sp::Sharpness:
            return "Sharpness";
        case sp::Brightness:
            return "Brightness";
        case sp::Contrast:
            return "Contrast";
        case sp::Saturation:
            return "Saturation";
        case sp::Hue:
            return "Hue";
        case sp::Gamma:
            return "Gamma";

        default:
            return "no_such_property";
    }
}

} // namespace tcam::property::emulated
