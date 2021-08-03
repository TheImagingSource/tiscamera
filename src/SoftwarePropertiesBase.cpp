

#include "SoftwarePropertiesBase.h"

using sp = tcam::property::emulated::software_prop;

const std::string_view tcam::property::emulated::find_property_name(sp id)
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
            return "BalanceWhite";
        case sp::WB_AUTO:
            return "BalanceWhiteAuto";
        case sp::WB_RED:
            return "BalanceWhiteRed";
        case sp::WB_GREEN:
            return "BalanceWhiteGreen";
        case sp::WB_BLUE:
            return "BalanceWhiteBlue";
        case sp::WB_ONCE:
            return "BalanceWhiteOnce";
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
