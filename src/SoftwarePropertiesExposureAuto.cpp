
#include "SoftwareProperties.h"
#include "logging.h"

#include <tcamprop1.0_base/tcamprop_property_info_list.h>

using namespace tcam;

using sp = tcam::property::emulated::software_prop;


void tcam::property::SoftwareProperties::generate_exposure_auto()
{
    auto has_exposure_auto = find_property(m_properties, "ExposureAuto") != nullptr;
    if (has_exposure_auto)
    {
        return;
    }

    m_dev_exposure =
        tcam::property::find_property<tcam::property::IPropertyFloat>(m_properties, "ExposureTime");
    if (!m_dev_exposure)
    {
        return;
    }

    if (auto exp_val = m_dev_exposure->get_value(); exp_val)
    {
        m_auto_params.exposure.val = exp_val.value();
    }
    else
    {
        SPDLOG_WARN("Unable to retrieve value for property '{}', due to {}",
                    m_dev_exposure->get_name(),
                    exp_val.error().message());
    }

    SPDLOG_INFO("Adding software ExposureAuto.");

    m_auto_params.exposure.granularity = m_dev_exposure->get_range().stp;

    m_auto_params.exposure.min = m_dev_exposure->get_range().min;
    m_auto_params.exposure.max = m_dev_exposure->get_range().max;

    m_auto_params.exposure.auto_enabled = true;

    m_exposure_auto_upper_limit = m_auto_params.exposure.max;

    const auto prop_range = emulated::to_range(*m_dev_exposure);
    const auto prop_range_lower_limit =
        emulated::prop_range_float_def { prop_range.range, prop_range.range.min };
    const auto prop_range_upper_limit =
        emulated::prop_range_float_def { prop_range.range, prop_range.range.max };

    auto new_exposure_time =
        make_prop_entry(sp::ExposureTime, &tcamprop1::prop_list::ExposureTime, prop_range);

    prop_ptr_vec new_list;

    add_prop_entry(new_list,
                   sp::ExposureAuto,
                   &tcamprop1::prop_list::ExposureAuto,
                   emulated::to_range(tcamprop1::prop_list::enum_entries_off_auto),
                   1);
    add_prop_entry(new_list,
                   sp::ExposureAutoReference,
                   &tcamprop1::prop_list::ExposureAutoReference,
                   emulated::prop_range_integer_def { 0, 255, 1, 128 });
    add_prop_entry(new_list,
                   sp::ExposureAutoLowerLimit,
                   &tcamprop1::prop_list::ExposureAutoLowerLimit,
                   prop_range_lower_limit);
    add_prop_entry(new_list,
                   sp::ExposureAutoUpperLimit,
                   &tcamprop1::prop_list::ExposureAutoUpperLimit,
                   prop_range_upper_limit);
    add_prop_entry(new_list,
                   sp::ExposureAutoUpperLimitAuto,
                   &tcamprop1::prop_list::ExposureAutoUpperLimitAuto,
                   true);
    add_prop_entry(new_list,
                   sp::ExposureAutoHighlightReduction,
                   &tcamprop1::prop_list::ExposureAutoHighlightReduction,
                   false);

    replace_entry(m_properties, new_exposure_time);
    add_prop_entry(m_properties, new_exposure_time->get_name(), new_list);
}

void tcam::property::SoftwareProperties::generate_gain_auto()
{
    auto has_exposure_auto = find_property(m_properties, "GainAuto") != nullptr;
    if (has_exposure_auto)
    {
        return;
    }

    m_dev_gain = tcam::property::find_property<IPropertyFloat>(m_properties, "Gain");
    if (!m_dev_gain)
    {
        return;
    }

    SPDLOG_INFO("Adding software GainAuto.");

    if (auto gain_val = m_dev_gain->get_value(); gain_val)
    {
        m_auto_params.gain.value = gain_val.value();
    }
    else
    {
        SPDLOG_ERROR("Unable to retrieve value for property '{}', due to {}",
                     m_dev_gain->get_name(),
                     gain_val.error().message());
    }

    m_auto_params.gain.auto_enabled = true;
    m_auto_params.gain.min = m_dev_gain->get_range().min;
    m_auto_params.gain.max = m_dev_gain->get_range().max;

    const auto prop_range = emulated::to_range(*m_dev_gain);
    const auto prop_range_lower_limit =
        emulated::prop_range_float_def { prop_range.range, prop_range.range.min };
    const auto prop_range_upper_limit =
        emulated::prop_range_float_def { prop_range.range, prop_range.range.max };


    prop_ptr_vec new_list;

    auto new_gain = make_prop_entry(sp::Gain, &tcamprop1::prop_list::Gain, prop_range);
    add_prop_entry(new_list,
                   sp::GainAuto,
                   &tcamprop1::prop_list::GainAuto,
                   emulated::to_range(tcamprop1::prop_list::enum_entries_off_auto),
                   1);
    add_prop_entry(new_list,
                   sp::GainAutoLowerLimit,
                   &tcamprop1::prop_list::GainAutoLowerLimit,
                   prop_range_lower_limit);
    add_prop_entry(new_list,
                   sp::GainAutoUpperLimit,
                   &tcamprop1::prop_list::GainAutoUpperLimit,
                   prop_range_upper_limit);

    replace_entry(m_properties, new_gain);
    add_prop_entry(m_properties, new_gain->get_name(), new_list);
}


void tcam::property::SoftwareProperties::generate_iris_auto()
{
    auto dev_iris_prop = find_property<IPropertyInteger>(m_properties, "Iris");
    auto has_iris_auto = find_property(m_properties, "IrisAuto") != nullptr;
    if (!dev_iris_prop || has_iris_auto)
    {
        return;
    }

    auto iris_range = dev_iris_prop->get_range();

    // cameras can has Iris behavior that prohibits IrisAuto
    // example would be the AFU420
    // the iris on that camera is either open or closed
    // check for that before adding IrisAuto
    if (iris_range.min == 0 && iris_range.max == 1) // is iris range [0;1]
    {
        return;
    }

    m_dev_iris = dev_iris_prop;


    m_auto_params.iris.min = iris_range.min;
    m_auto_params.iris.max = iris_range.max;

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

    auto new_iris =
        make_prop_entry(sp::Iris, &tcamprop1::prop_list::Iris, emulated::to_range(*m_dev_iris));
    auto new_iris_auto =
        make_prop_entry(sp::IrisAuto,
                        &tcamprop1::prop_list::IrisAuto,
                        emulated::to_range(tcamprop1::prop_list::enum_entries_off_auto),
                        1);

    replace_entry(m_properties, new_iris);
    add_prop_entry(m_properties, new_iris->get_name(), { new_iris_auto });
}


void tcam::property::SoftwareProperties::generate_auto_functions_roi()
{
    if (find_property<IPropertyInteger>(m_properties, "AutoFunctionsROITop") != nullptr)
    {
        return;
    }
    SPDLOG_INFO("Adding software AutoFunctionsROI");

    prop_ptr_vec new_list;

    add_prop_entry(
        new_list, sp::AutoFunctionsROIEnable, &tcamprop1::prop_list::AutoFunctionsROIEnable, 0);

    auto preset_entries = tcamprop1::prop_list::AutoFunctionsROIPreset_EnumEntries;
    std::vector<std::string_view> entries(preset_entries.begin(), preset_entries.end());

    add_prop_entry(new_list,
                   sp::AutoFunctionsROIPreset,
                   &tcamprop1::prop_list::AutoFunctionsROIPreset,
                   emulated::to_range(preset_entries),
                   0);

    tcamprop1::prop_range_integer y_range = { 0, sensor_dimensions_.height, ROI_STEP_SIZE };
    tcamprop1::prop_range_integer x_range = { 0, sensor_dimensions_.width, ROI_STEP_SIZE };
    const auto x_def = emulated::prop_range_integer_def { x_range, 0 };
    const auto y_def = emulated::prop_range_integer_def { y_range, 0 };

    add_prop_entry(
        new_list, sp::AutoFunctionsROILeft, &tcamprop1::prop_list::AutoFunctionsROILeft, x_def);
    add_prop_entry(
        new_list, sp::AutoFunctionsROITop, &tcamprop1::prop_list::AutoFunctionsROITop, y_def);
    add_prop_entry(
        new_list, sp::AutoFunctionsROIWidth, &tcamprop1::prop_list::AutoFunctionsROIWidth, x_def);
    add_prop_entry(
        new_list, sp::AutoFunctionsROIHeight, &tcamprop1::prop_list::AutoFunctionsROIHeight, y_def);

    add_prop_entry(m_properties, "AutoFunctionsRoiEnable", new_list);

    m_prop_brightness_top =
        std::dynamic_pointer_cast<tcam::property::emulated::SoftwarePropertyIntegerImpl>(
            find_property(new_list, "AutoFunctionsROITop"));
    m_prop_brightness_left =
        std::dynamic_pointer_cast<tcam::property::emulated::SoftwarePropertyIntegerImpl>(
            find_property(new_list, "AutoFunctionsROILeft"));
    m_prop_brightness_width =
        std::dynamic_pointer_cast<tcam::property::emulated::SoftwarePropertyIntegerImpl>(
            find_property(new_list, "AutoFunctionsROIWidth"));
    m_prop_brightness_height =
        std::dynamic_pointer_cast<tcam::property::emulated::SoftwarePropertyIntegerImpl>(
            find_property(new_list, "AutoFunctionsROIHeight"));
    m_prop_brightness_preset =
        std::dynamic_pointer_cast<tcam::property::emulated::SoftwarePropertyEnumImpl>(
            find_property(new_list, "AutoFunctionsROIPreset"));
    m_prop_brightness_enable =
        std::dynamic_pointer_cast<tcam::property::emulated::SoftwarePropertyBoolImpl>(
            find_property(new_list, "AutoFunctionsROIEnable"));
}


void tcam::property::SoftwareProperties::set_auto_functions_preset_mode(
    AutoFunctionsROIPreset_Modes mode)
{
    m_brightness_roi_mode = mode;
    switch (mode)
    {
        case AutoFunctionsROIPreset_Modes::full:
        {
            m_brightness_top = 0;
            m_brightness_left = 0;
            m_brightness_width = m_format.get_size().width;
            m_brightness_height = m_format.get_size().height;
            break;
        }
        case AutoFunctionsROIPreset_Modes::custom:
        {
            break;
        }
        case AutoFunctionsROIPreset_Modes::center_50:
        {
            m_brightness_top = m_format.get_size().height / 4;
            m_brightness_left = m_format.get_size().width / 4;
            m_brightness_height = m_format.get_size().height / 2;
            m_brightness_width = m_format.get_size().width / 2;
            break;
        }
        case AutoFunctionsROIPreset_Modes::center_25:
        {
            m_brightness_top = (m_format.get_size().height / 8) * 3;
            m_brightness_left = (m_format.get_size().width / 8) * 3;
            m_brightness_height = m_format.get_size().height / 4;
            m_brightness_width = m_format.get_size().width / 4;
            break;
        }
        case AutoFunctionsROIPreset_Modes::bottom_half:
        {
            m_brightness_top = m_format.get_size().height / 2;
            m_brightness_left = 0;
            m_brightness_width = m_format.get_size().width;
            m_brightness_height = m_format.get_size().height / 2;
            break;
        }
        case AutoFunctionsROIPreset_Modes::top_half:
        {
            m_brightness_top = 0;
            m_brightness_left = 0;
            m_brightness_height = m_format.get_size().height / 2;
            m_brightness_width = m_format.get_size().width;
            break;
        }
        default:
        {
            return; // when this is a bad parameter, just return and do not set variable
        }
    }
    // ensure that all values are always legal
    m_brightness_top -= m_brightness_top % ROI_STEP_SIZE;
    m_brightness_left -= m_brightness_left % ROI_STEP_SIZE;
    m_brightness_width -= m_brightness_width % ROI_STEP_SIZE;
    m_brightness_height -= m_brightness_height % ROI_STEP_SIZE;
}


void tcam::property::SoftwareProperties::generate_focus_auto()
{
    auto dev_focus = tcam::property::find_property<IPropertyInteger>(m_properties, "Focus");
    auto has_focus_auto = find_property(m_properties, "FocusAuto") != nullptr;
    if (!dev_focus || has_focus_auto)
    {
        return;
    }
    m_dev_focus = dev_focus;

    if (auto val = m_dev_focus->get_value(); val)
    {
        m_auto_params.focus_onepush_params.device_focus_val = val.value();
    }
    else
    {
        SPDLOG_ERROR("Unable to retrieve value: {}", val.error().message());
    }

    SPDLOG_INFO("Adding Software FocusAuto.");

    m_auto_params.focus_onepush_params.enable_focus = true;
    m_auto_params.focus_onepush_params.run_cmd_params.focus_range_min =
        m_dev_focus->get_range().min;
    m_auto_params.focus_onepush_params.run_cmd_params.focus_range_max =
        m_dev_focus->get_range().max;

    auto new_focus =
        make_prop_entry(sp::Focus, &tcamprop1::prop_list::Focus, emulated::to_range(*m_dev_focus));
    auto new_focus_auto =
        make_prop_entry(sp::FocusAuto,
                        &tcamprop1::prop_list::FocusAuto,
                        emulated::to_range(tcamprop1::prop_list::enum_entries_off_once),
                        0);

    replace_entry(m_properties, new_focus);
    add_prop_entry(m_properties, new_focus->get_name(), { new_focus_auto });

    tcamprop1::prop_range_integer top_range = { 0, sensor_dimensions_.height, ROI_STEP_SIZE };
    tcamprop1::prop_range_integer width_range = { 0, sensor_dimensions_.width, ROI_STEP_SIZE };

    const auto top_def = emulated::prop_range_integer_def { top_range, 0 };
    const auto width_def = emulated::prop_range_integer_def { width_range, 0 };

    auto focus_roi_top =
        make_prop_entry(sp::FocusAutoTop, &tcamprop1::prop_list::AutoFocusROITop, top_def);
    auto focus_roi_left =
        make_prop_entry(sp::FocusAutoLeft, &tcamprop1::prop_list::AutoFocusROILeft, width_def);
    auto focus_roi_height =
        make_prop_entry(sp::FocusAutoHeight, &tcamprop1::prop_list::AutoFocusROIHeight, top_def);
    auto focus_roi_width =
        make_prop_entry(sp::FocusAutoWidth, &tcamprop1::prop_list::AutoFocusROIWidth, width_def);

    add_prop_entry(m_properties,
                   new_focus_auto->get_name(),
                   { focus_roi_left, focus_roi_top, focus_roi_width, focus_roi_height });

    m_prop_focus_top =
        std::dynamic_pointer_cast<tcam::property::emulated::SoftwarePropertyIntegerImpl>(
            focus_roi_top);
    m_prop_focus_left =
        std::dynamic_pointer_cast<tcam::property::emulated::SoftwarePropertyIntegerImpl>(
            focus_roi_left);
    m_prop_focus_width =
        std::dynamic_pointer_cast<tcam::property::emulated::SoftwarePropertyIntegerImpl>(
            focus_roi_width);
    m_prop_focus_height =
        std::dynamic_pointer_cast<tcam::property::emulated::SoftwarePropertyIntegerImpl>(
            focus_roi_height);
}
