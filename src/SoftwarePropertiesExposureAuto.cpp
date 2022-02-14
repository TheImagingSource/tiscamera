
#include "SoftwareProperties.h"
#include "logging.h"
#include <tcamprop1.0_base/tcamprop_property_info_list.h>
using namespace tcam;

using sp = tcam::property::emulated::software_prop;


void tcam::property::SoftwareProperties::generate_exposure_auto()
{
    m_dev_exposure = tcam::property::find_property<tcam::property::IPropertyFloat>(m_device_properties, "ExposureTime");
    if (!m_dev_exposure)
    {
        SPDLOG_ERROR("Unable to identify exposure interface.");
        return;
    }

    SPDLOG_INFO("Adding software ExposureAuto.");

    m_auto_params.exposure.granularity = m_dev_exposure->get_range().stp;

    m_auto_params.exposure.min = m_dev_exposure->get_range().min;
    m_auto_params.exposure.max = m_dev_exposure->get_range().max;

    if (auto exp_val = m_dev_exposure->get_value(); exp_val)
    {
        m_auto_params.exposure.val = exp_val.value();
    }
    else
    {
        SPDLOG_ERROR("Unable to retrieve value for property '{}', due to {}", m_dev_exposure->get_name(), exp_val.error().message());
    }

    m_auto_params.exposure.auto_enabled = true;

    m_exposure_auto_upper_limit = m_auto_params.exposure.max;

    const auto prop_range = emulated::to_range(*m_dev_exposure);
    const auto prop_range_lower_limit =
        emulated::prop_range_float_def { prop_range.range, prop_range.range.min };
    const auto prop_range_upper_limit =
        emulated::prop_range_float_def { prop_range.range, prop_range.range.max };

    add_prop_entry(sp::ExposureTime, &tcamprop1::prop_list::ExposureTime, prop_range);
    add_prop_entry(sp::ExposureAuto,
                   &tcamprop1::prop_list::ExposureAuto,
                   emulated::to_range(tcamprop1::prop_list::enum_entries_off_auto),
                   1);
    add_prop_entry(sp::ExposureAutoReference,
                   &tcamprop1::prop_list::ExposureAutoReference,
                   emulated::prop_range_integer_def { 0, 255, 1, 128 });
    add_prop_entry(
        sp::ExposureAutoLowerLimit, &tcamprop1::prop_list::ExposureAutoLowerLimit, prop_range_lower_limit);
    add_prop_entry(
        sp::ExposureAutoUpperLimit, &tcamprop1::prop_list::ExposureAutoUpperLimit, prop_range_upper_limit);
    add_prop_entry(sp::ExposureAutoUpperLimitAuto,
                   &tcamprop1::prop_list::ExposureAutoUpperLimitAuto,
                   true);
    add_prop_entry(
        sp::ExposureAutoHighlightReduction, &tcamprop1::prop_list::ExposureAutoHighlightReduction, false);
}

void tcam::property::SoftwareProperties::generate_gain_auto()
{
    m_dev_gain = tcam::property::find_property<IPropertyFloat>(m_device_properties, "Gain");
    if (!m_dev_gain)
    {
        SPDLOG_ERROR("Unable to fetch gain interface.");
        return;
    }

    m_auto_params.gain.auto_enabled = true;
    m_auto_params.gain.min = m_dev_gain->get_range().min;
    m_auto_params.gain.max = m_dev_gain->get_range().max;

    if (auto gain_val = m_dev_gain->get_value(); gain_val)
    {
        m_auto_params.gain.value = gain_val.value();
    }
    else
    {
        SPDLOG_ERROR("Unable to retrieve value for property '{}', due to {}",
                     m_dev_gain->get_name(),
                     gain_val.error().message());
        return;
    }

    SPDLOG_INFO("Adding software GainAuto.");

    const auto prop_range = emulated::to_range(*m_dev_gain);
    const auto prop_range_lower_limit =
        emulated::prop_range_float_def { prop_range.range, prop_range.range.min };
    const auto prop_range_upper_limit =
        emulated::prop_range_float_def { prop_range.range, prop_range.range.max };

    add_prop_entry(sp::Gain, &tcamprop1::prop_list::Gain, prop_range);
    add_prop_entry(
        sp::GainAuto, &tcamprop1::prop_list::GainAuto, emulated::to_range(tcamprop1::prop_list::enum_entries_off_auto), 1);
    add_prop_entry(sp::GainAutoLowerLimit, &tcamprop1::prop_list::GainAutoLowerLimit, prop_range_lower_limit);
    add_prop_entry(sp::GainAutoUpperLimit, &tcamprop1::prop_list::GainAutoUpperLimit, prop_range_upper_limit);
}


void tcam::property::SoftwareProperties::generate_iris_auto()
{
    m_dev_iris = tcam::property::find_property<IPropertyInteger>(m_device_properties, "Iris");
    if (!m_dev_iris)
    {
        return;
    }

    m_auto_params.iris.min = m_dev_iris->get_range().min;
    m_auto_params.iris.max = m_dev_iris->get_range().max;

    if (auto val = m_dev_iris->get_value(); val)
    {
        m_auto_params.iris.val = val.value();
    }
    else
    {
        SPDLOG_ERROR("Unable to retrieve Iris value: {}", val.error().message());
    }

    SPDLOG_INFO("Adding software IrisAuto.");

    // #TODO: granularity/step

    add_prop_entry(
        sp::IrisAuto, &tcamprop1::prop_list::IrisAuto, emulated::to_range(tcamprop1::prop_list::enum_entries_off_auto), 1);
    add_prop_entry(sp::Iris, &tcamprop1::prop_list::Iris, emulated::to_range(*m_dev_iris));
}
