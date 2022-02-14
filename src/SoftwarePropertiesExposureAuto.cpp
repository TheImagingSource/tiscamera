
#include "SoftwareProperties.h"
#include "logging.h"
#include <tcamprop1.0_base/tcamprop_property_info_list.h>
using namespace tcam;

using sp = tcam::property::emulated::software_prop;

namespace
{

namespace prop_lst = tcamprop1::prop_list;

} // namespace

namespace tcam::property
{
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

    add_prop_entry(sp::ExposureTime, &prop_lst::ExposureTime, prop_range);
    add_prop_entry(sp::ExposureAuto,
                   &prop_lst::ExposureAuto,
                   emulated::to_range(prop_lst::enum_entries_off_auto),
                   1);
    add_prop_entry(sp::ExposureAutoReference,
                   &prop_lst::ExposureAutoReference,
                   emulated::prop_range_integer_def { 0, 255, 1, 128 });
    add_prop_entry(
        sp::ExposureAutoLowerLimit, &prop_lst::ExposureAutoLowerLimit, prop_range_lower_limit);
    add_prop_entry(
        sp::ExposureAutoUpperLimit, &prop_lst::ExposureAutoUpperLimit, prop_range_upper_limit);
    add_prop_entry(sp::ExposureAutoUpperLimitAuto,
                   &prop_lst::ExposureAutoUpperLimitAuto,
                   true);
    add_prop_entry(
        sp::ExposureAutoHighlightReduction, &prop_lst::ExposureAutoHighlightReduction, false);
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

    add_prop_entry(sp::Gain, &prop_lst::Gain, prop_range);
    add_prop_entry(
        sp::GainAuto, &prop_lst::GainAuto, emulated::to_range(prop_lst::enum_entries_off_auto), 1);
    add_prop_entry(sp::GainAutoLowerLimit, &prop_lst::GainAutoLowerLimit, prop_range_lower_limit);
    add_prop_entry(sp::GainAutoUpperLimit, &prop_lst::GainAutoUpperLimit, prop_range_upper_limit);
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
        sp::IrisAuto, &prop_lst::IrisAuto, emulated::to_range(prop_lst::enum_entries_off_auto), 1);
    add_prop_entry(sp::Iris, &prop_lst::Iris, emulated::to_range(*m_dev_iris));
}

} // namespace tcam::property