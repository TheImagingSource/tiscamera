
#include "tcamprop1.0_base/tcamprop_property_info.h"
#include "tcamprop1.0_base/tcamprop_property_info_list.h"

namespace lst = tcamprop1::prop_list;

using namespace tcamprop1;

template<class T>
static constexpr auto to_( const T& prop_ref ) noexcept
{
    return prop_static_info_find_result{ T::property_type, &prop_ref };
}

static const constexpr tcamprop1::prop_static_info_find_result static_prop_list[] =
{
    to_( lst::Gain ),
    to_( lst::GainAuto ),
    to_( lst::GainAutoLowerLimit ),
    to_( lst::GainAutoUpperLimit ),
    to_( lst::ExposureTime ),
    to_( lst::ExposureAuto ),
    to_( lst::ExposureAutoReference ),
    to_( lst::ExposureAutoLowerLimit ),
    to_( lst::ExposureAutoUpperLimit ),
    to_( lst::ExposureAutoUpperLimitAuto ),
    to_( lst::ExposureAutoHighlightReduction ),
    to_( lst::BalanceWhiteAuto ),
    to_( lst::BalanceWhiteMode ),
    to_( lst::BalanceWhiteAutoPreset ),
    to_( lst::BalanceWhiteTemperaturePreset ),
    to_( lst::BalanceWhiteTemperature ),
    to_( lst::BalanceWhiteRed ),
    to_( lst::BalanceWhiteGreen ),
    to_( lst::BalanceWhiteBlue ),
    to_( lst::ClaimBalanceWhiteSoftware ),
    to_( lst::ClaimHDRGain ),
    to_( lst::HDRGain ),
    to_( lst::HDRGainAuto ),
    to_( lst::HDRGainAutoReference ),
    to_( lst::BlackLevel ),
    to_( lst::OffsetX ),
    to_( lst::OffsetY ),
    to_( lst::OffsetAutoCenter ),
    to_( lst::ReverseX ),
    to_( lst::ReverseY ),
    to_( lst::Iris ),
    to_( lst::IrisAuto ),

    to_( lst::Focus ),
    to_( lst::FocusAuto ),
    to_( lst::AutoFocusROIEnable ),
    to_( lst::AutoFocusROILeft ),
    to_( lst::AutoFocusROITop ),
    to_( lst::AutoFocusROIWidth ),
    to_( lst::AutoFocusROIHeight ),

    to_( lst::Zoom ),
    to_( lst::IRCutFilterEnable ),

    to_( lst::AutoFunctionsROIEnable ),
    to_( lst::AutoFunctionsROIPreset ),
    to_( lst::AutoFunctionsROIHeight ),
    to_( lst::AutoFunctionsROIWidth ),
    to_( lst::AutoFunctionsROITop ),
    to_( lst::AutoFunctionsROILeft ),

    to_( lst::Denoise ),
    to_( lst::Sharpness ),
    to_( lst::SoftwareBrightness ),
    to_( lst::Contrast ),
    to_( lst::Gamma ),
    to_( lst::Saturation ),
    to_( lst::Hue ),
    to_( lst::TonemappingEnable ),
    to_( lst::TonemappingGlobalBrightness ),
    to_( lst::TonemappingIntensity ),
    to_( lst::ColorTransformationEnable ),
    to_( lst::ColorTransformation_Value_Gain00 ),
    to_( lst::ColorTransformation_Value_Gain01 ),
    to_( lst::ColorTransformation_Value_Gain02 ),
    to_( lst::ColorTransformation_Value_Gain10 ),
    to_( lst::ColorTransformation_Value_Gain11 ),
    to_( lst::ColorTransformation_Value_Gain12 ),
    to_( lst::ColorTransformation_Value_Gain20 ),
    to_( lst::ColorTransformation_Value_Gain21 ),
    to_( lst::ColorTransformation_Value_Gain22 ),
    to_( lst::StrobeEnable ),
    to_( lst::StrobePolarity ),
    to_( lst::StrobeOperation ),
    to_( lst::StrobeDuration ),
    to_( lst::StrobeDelay ),
    to_( lst::TriggerMode ),
    to_( lst::TriggerSoftware ),
    to_( lst::TriggerSource ),
    //to_( lst::TriggerPolarity ),
    //to_( lst::TriggerExposureMode ),
    //to_( lst::TriggerBurstCount ),
    to_( lst::TriggerOperation ),
    to_( lst::TriggerActivation ),
    to_( lst::TriggerSelector ),
    to_( lst::TriggerOverlap ),
    to_( lst::TriggerMask ),
    to_( lst::TriggerDenoise ),
    to_( lst::TriggerDelay ),
    to_( lst::TriggerDebouncer ),
    to_( lst::IMXLowLatencyTriggerMode ),
    to_( lst::GPIn ),
    to_( lst::GPOut ),
    to_( lst::GPIO ),

    to_( lst::AcquisitionBurstFrameCount ),
    to_( lst::AcquisitionBurstInterval ),

    //to_( lst::ActionQueueSize ),
    //to_( lst::ActionSchedulerCancel ),
    //to_( lst::ActionSchedulerCommit ),
    //to_( lst::ActionSchedulerInterval ),
    //to_( lst::ActionSchedulerStatus ),
    //to_( lst::ActionSchedulerTime ),

    to_( lst::LUTEnable ),
    to_( lst::LUTIndex ),
    to_( lst::LUTSelector ),
    to_( lst::LUTValue ),

    to_( lst::MultiFrameSetOutputModeCustomGain ),
    to_( lst::MultiFrameSetOutputModeEnable ),
    to_( lst::MultiFrameSetOutputModeExposureTime0 ),
    to_( lst::MultiFrameSetOutputModeExposureTime1 ),
    to_( lst::MultiFrameSetOutputModeExposureTime2 ),
    to_( lst::MultiFrameSetOutputModeExposureTime3 ),
    to_( lst::MultiFrameSetOutputModeFrameCount ),
    to_( lst::MultiFrameSetOutputModeGain0 ),
    to_( lst::MultiFrameSetOutputModeGain1 ),
    to_( lst::MultiFrameSetOutputModeGain2 ),
    to_( lst::MultiFrameSetOutputModeGain3 ),

    to_( lst::ExpandOutputRange ),
    to_( lst::ShowInfoOverlay ),

    to_( lst::SensorWidth ),
    to_( lst::SensorHeight ),

#if 0
    to_( lst::test ),
#endif
};

auto tcamprop1::find_prop_static_info( std::string_view name ) noexcept -> prop_static_info_find_result
{
    for( auto&& e : static_prop_list ) {
        if( e.info_ptr->name == name ) {
            return e;
        }
    }
    return {};
}
