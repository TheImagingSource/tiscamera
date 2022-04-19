

#include "SoftwareProperties.h"

#include "SoftwarePropertiesImpl.h"
#include "logging.h"

#include <algorithm>
#include <chrono>
#include <memory>
#include <tcamprop1.0_base/tcamprop_property_info_list.h>

using namespace tcam;

using sp = tcam::property::emulated::software_prop;

tcam::property::SoftwareProperties::SoftwareProperties(
    const std::vector<std::shared_ptr<tcam::property::IPropertyBase>>& dev_properties)
    : m_properties(dev_properties), p_state(auto_alg::make_state_ptr())
{
    auto ptr = find_property(m_properties, "SensorWidth");
    if (!ptr)
    {
        SPDLOG_ERROR(
            "Unable to determine sensor size. This will cause problems for some properties.");
    }
    else
    {
        sensor_dimensions_.width =
            std::dynamic_pointer_cast<IPropertyInteger>(ptr)->get_value().value();
    }

    auto ptr2 = find_property(m_properties, "SensorHeight");
    if (!ptr2)
    {
        SPDLOG_ERROR(
            "Unable to determine sensor size. This will cause problems for some properties.");
    }
    else
    {
        sensor_dimensions_.height =
            std::dynamic_pointer_cast<IPropertyInteger>(ptr2)->get_value().value();
    }
}

std::shared_ptr<tcam::property::SoftwareProperties> tcam::property::SoftwareProperties::create(
    const std::vector<std::shared_ptr<tcam::property::IPropertyBase>>& dev_properties,
    bool has_bayer)
{
    auto ptr = std::make_shared<SoftwareProperties>(dev_properties);
    ptr->generate_public_properties(has_bayer);
    return ptr;
}

static uint64_t time_now_in_us() noexcept
{
    return std::chrono::duration_cast<std::chrono::microseconds>(
               std::chrono::high_resolution_clock::now().time_since_epoch())
        .count();
}


void tcam::property::SoftwareProperties::auto_pass(const img::img_descriptor& image)
{
    auto_alg::auto_pass_params tmp_params;
    {
        std::scoped_lock lock(m_property_mtx);

        tmp_params = m_auto_params;

        m_auto_params.focus_onepush_params.is_run_cmd = false;
        tmp_params.exposure.max = m_exposure_auto_upper_limit;

        if (m_active_brightness_roi)
        {
            tmp_params.brightness_roi = { m_brightness_left,
                                          m_brightness_top,
                                          m_brightness_left + m_brightness_width,
                                          m_brightness_top + m_brightness_height };
        }
        else
        {
            tmp_params.brightness_roi = {};
        }
        tmp_params.focus_onepush_params.run_cmd_params.roi = {
            m_focus_left, m_focus_top, m_focus_left + m_focus_width, m_focus_top + m_focus_height
        };
    }

    tmp_params.frame_number = m_frame_counter++;
    tmp_params.time_point = time_now_in_us();

    auto auto_pass_ret = auto_alg::auto_pass(*p_state, image, tmp_params);

    if (auto_pass_ret.exposure_changed)
    {
        m_auto_params.exposure.val = auto_pass_ret.exposure_value;
        auto set_exp = m_dev_exposure->set_value(auto_pass_ret.exposure_value);
        if (!set_exp)
        {
            SPDLOG_ERROR("Unable to set exposure: {}", set_exp.error().message());
        }
    }

    if (auto_pass_ret.gain_changed)
    {
        m_auto_params.gain.value = auto_pass_ret.gain_value;
        auto set_gain = m_dev_gain->set_value(auto_pass_ret.gain_value);
        if (!set_gain)
        {
            SPDLOG_ERROR("Unable to set gain: {}", set_gain.error().message());
        }
    }

    if (auto_pass_ret.iris_changed)
    {
        m_auto_params.iris.val = auto_pass_ret.iris_value;
        auto set_iris = m_dev_iris->set_value(auto_pass_ret.iris_value);
        if (!set_iris)
        {
            SPDLOG_ERROR("Unable to set iris: {}", set_iris.error().message());
        }
    }

    if (auto_pass_ret.focus_changed)
    {
        m_auto_params.focus_onepush_params.device_focus_val = auto_pass_ret.focus_value;
        auto set_foc = m_dev_focus->set_value(auto_pass_ret.focus_value);
        if (!set_foc)
        {
            SPDLOG_ERROR("Unable to set focus: {}", set_foc.error().message());
        }
    }

    if (auto_pass_ret.wb.wb_changed)
    {
        m_auto_params.wb.channels = auto_pass_ret.wb.channels;
        m_auto_params.wb.one_push_enabled = auto_pass_ret.wb.one_push_still_running;

        // SPDLOG_DEBUG("WB r: {}", auto_pass_ret.wb.channels.r);
        // SPDLOG_DEBUG("WB g: {}", auto_pass_ret.wb.channels.g);
        // SPDLOG_DEBUG("WB b: {}", auto_pass_ret.wb.channels.b);

        if (m_wb.is_dev_wb())
        {
            auto res = set_whitebalance_channel(emulated::software_prop::BalanceWhiteRed,
                                                auto_pass_ret.wb.channels.r);

            if (!res)
            {
                SPDLOG_DEBUG("Setting whitebalance caused an error: {}",
                             res.as_failure().error().message());
            }
            res = set_whitebalance_channel(emulated::software_prop::BalanceWhiteGreen,
                                           auto_pass_ret.wb.channels.g);
            if (!res)
            {
                SPDLOG_DEBUG("Setting whitebalance caused an error: {}",
                             res.as_failure().error().message());
            }

            res = set_whitebalance_channel(emulated::software_prop::BalanceWhiteBlue,
                                           auto_pass_ret.wb.channels.b);
            if (!res)
            {
                SPDLOG_DEBUG("Setting whitebalance caused an error: {}",
                             res.as_failure().error().message());
            }
        }
    }
}

void tcam::property::SoftwareProperties::generate_public_properties(bool has_bayer)
{
    m_auto_params = {};

    generate_exposure_auto();
    generate_gain_auto();
    generate_iris_auto();

    generate_auto_functions_roi();

    generate_focus_auto();

    { // BalanceWhite stuff
        if (has_bayer)
        {
            generate_balance_white_auto();
        }
    }

    generate_color_transformation();

    m_properties = m_properties;
}

outcome::result<int64_t> tcam::property::SoftwareProperties::get_int(
    emulated::software_prop prop_id)
{
    std::scoped_lock lock(m_property_mtx);

    switch (prop_id)
    {
        case emulated::software_prop::ExposureTime:
        case emulated::software_prop::ExposureAutoLowerLimit:
        case emulated::software_prop::ExposureAutoUpperLimit:
        case emulated::software_prop::Gain:
        case emulated::software_prop::GainAutoLowerLimit:
        case emulated::software_prop::GainAutoUpperLimit:
        case emulated::software_prop::BalanceWhiteRed:
        case emulated::software_prop::BalanceWhiteGreen:
        case emulated::software_prop::BalanceWhiteBlue:
        case emulated::software_prop::ColorTransformRedToRed:
        case emulated::software_prop::ColorTransformGreenToRed:
        case emulated::software_prop::ColorTransformBlueToRed:
        case emulated::software_prop::ColorTransformRedToGreen:
        case emulated::software_prop::ColorTransformGreenToGreen:
        case emulated::software_prop::ColorTransformBlueToGreen:
        case emulated::software_prop::ColorTransformRedToBlue:
        case emulated::software_prop::ColorTransformGreenToBlue:
        case emulated::software_prop::ColorTransformBlueToBlue:

            return tcam::status::PropertyNotImplemented;

        case emulated::software_prop::ExposureAuto:
            return m_auto_params.exposure.auto_enabled ? 1 : 0;
        case emulated::software_prop::ExposureAutoUpperLimitAuto:
            return m_exposure_auto_upper_limit_auto;
        case emulated::software_prop::ExposureAutoReference:
            return m_auto_params.exposure_reference.val;
        case emulated::software_prop::ExposureAutoHighlightReduction:
            return m_auto_params.enable_highlight_reduction ? 1 : 0;

        case emulated::software_prop::GainAuto:
            return m_auto_params.gain.auto_enabled ? 1 : 0;

        case emulated::software_prop::AutoFunctionsROIEnable:
            return m_active_brightness_roi ? 1 : 0;
        case emulated::software_prop::AutoFunctionsROIPreset:
            return static_cast<int>(m_brightness_roi_mode);
        case emulated::software_prop::AutoFunctionsROILeft:
            return m_brightness_left;
        case emulated::software_prop::AutoFunctionsROITop:
            return m_brightness_top;
        case emulated::software_prop::AutoFunctionsROIWidth:
            return m_brightness_width;
        case emulated::software_prop::AutoFunctionsROIHeight:
            return m_brightness_height;
        case emulated::software_prop::Iris:
            return m_auto_params.iris.val;
        case emulated::software_prop::IrisAuto:
            return m_auto_params.iris.auto_enabled ? 1 : 0;
        case emulated::software_prop::Focus:
            return m_auto_params.focus_onepush_params.device_focus_val;
        case emulated::software_prop::FocusAuto:
            return m_auto_params.focus_onepush_params.is_run_cmd;
        case emulated::software_prop::FocusAutoTop:
            return m_focus_top;
        case emulated::software_prop::FocusAutoLeft:
            return m_focus_left;
        case emulated::software_prop::FocusAutoHeight:
            return m_focus_height;
        case emulated::software_prop::FocusAutoWidth:
            return m_focus_width;
        case emulated::software_prop::BalanceWhiteAuto:
        {
            if (m_auto_params.wb.auto_enabled)
            {
                return 1;
            }
            if (m_auto_params.wb.one_push_enabled)
            {
                return 2;
            }
            return 0;
        }
        case emulated::software_prop::ClaimBalanceWhiteSoftware:
            return m_wb.m_wb_is_claimed;
        case emulated::software_prop::ColorTransformEnable:
        {
            auto res = m_dev_color_transform_enable->get_value();
            if (res.has_failure())
            {
                return res.as_failure();
            }
            return res.value();
        }
    }
    SPDLOG_WARN("Not implemented. ID: {}", prop_id);
    return tcam::status::PropertyNotImplemented;
}

outcome::result<void> tcam::property::SoftwareProperties::set_int(emulated::software_prop prop_id,
                                                                  int64_t new_val)
{
    std::scoped_lock lock(m_property_mtx);

    switch (prop_id)
    {
        case emulated::software_prop::ExposureTime:
        case emulated::software_prop::ExposureAutoLowerLimit:
        case emulated::software_prop::ExposureAutoUpperLimit:
        case emulated::software_prop::Gain:
        case emulated::software_prop::GainAutoLowerLimit:
        case emulated::software_prop::GainAutoUpperLimit:
        case emulated::software_prop::BalanceWhiteRed:
        case emulated::software_prop::BalanceWhiteGreen:
        case emulated::software_prop::BalanceWhiteBlue:
        case emulated::software_prop::ColorTransformRedToRed:
        case emulated::software_prop::ColorTransformGreenToRed:
        case emulated::software_prop::ColorTransformBlueToRed:
        case emulated::software_prop::ColorTransformRedToGreen:
        case emulated::software_prop::ColorTransformGreenToGreen:
        case emulated::software_prop::ColorTransformBlueToGreen:
        case emulated::software_prop::ColorTransformRedToBlue:
        case emulated::software_prop::ColorTransformGreenToBlue:
        case emulated::software_prop::ColorTransformBlueToBlue:
            return tcam::status::PropertyNotImplemented;

        case emulated::software_prop::ExposureAuto:
        {
            m_auto_params.exposure.auto_enabled = new_val;
            return outcome::success();
        }
        case emulated::software_prop::ExposureAutoUpperLimitAuto:
        {
            m_exposure_auto_upper_limit_auto = new_val;

            if (m_exposure_auto_upper_limit_auto)
            {
                if (m_format.get_framerate() != 0)
                {
                    m_exposure_auto_upper_limit = 1'000'000 / m_format.get_framerate();
                }
            }
            return outcome::success();
        }
        case emulated::software_prop::ExposureAutoReference:
        {
            m_auto_params.exposure_reference.val = new_val;
            return outcome::success();
        }
        case emulated::software_prop::ExposureAutoHighlightReduction:
        {
            m_auto_params.enable_highlight_reduction = new_val != 0;
            return outcome::success();
        }
        case emulated::software_prop::GainAuto:
        {
            m_auto_params.gain.auto_enabled = new_val;
            return outcome::success();
        }
        case emulated::software_prop::AutoFunctionsROIEnable:
        {
            m_active_brightness_roi = new_val;
            return outcome::success();
        }
        case emulated::software_prop::AutoFunctionsROIPreset:
        {
            set_auto_functions_preset_mode(static_cast<AutoFunctionsROIPreset_Modes>(new_val));
            return outcome::success();
        }
        case emulated::software_prop::AutoFunctionsROILeft:
        {
            m_brightness_left = new_val;
            m_brightness_roi_mode = AutoFunctionsROIPreset_Modes::custom;

            // reduce roi width to always remain within image boundaries
            // 0 check as proerties may be set before a format is known
            if (m_format.get_size().width != 0
                && m_brightness_left + m_brightness_width > (int)m_format.get_size().width)
            {
                m_brightness_width = m_format.get_size().width - m_brightness_left;
            }
            return outcome::success();
        }
        case emulated::software_prop::AutoFunctionsROITop:
        {
            m_brightness_top = new_val;
            m_brightness_roi_mode = AutoFunctionsROIPreset_Modes::custom;

            // reduce roi height to always remain within image boundaries
            // 0 check as proerties may be set before a format is known
            if (m_format.get_size().height != 0
                && m_brightness_top + m_brightness_height > (int)m_format.get_size().height)
            {
                m_brightness_height = m_format.get_size().height - m_brightness_top;
            }
            return outcome::success();
        }
        case emulated::software_prop::AutoFunctionsROIWidth:
        {
            m_brightness_width = new_val;
            m_brightness_roi_mode = AutoFunctionsROIPreset_Modes::custom;
            return outcome::success();
        }
        case emulated::software_prop::AutoFunctionsROIHeight:
        {
            m_brightness_height = new_val;
            m_brightness_roi_mode = AutoFunctionsROIPreset_Modes::custom;
            return outcome::success();
        }
        case emulated::software_prop::Iris:
        {
            if (m_auto_params.iris.auto_enabled)
            {
                return tcam::status::PropertyNotWriteable;
            }
            m_auto_params.iris.val = new_val;
            return m_dev_iris->set_value(new_val);
        }
        case emulated::software_prop::IrisAuto:
        {
            m_auto_params.iris.auto_enabled = new_val;
            return outcome::success();
        }
        case emulated::software_prop::Focus:
        {
            m_auto_params.focus_onepush_params.device_focus_val = new_val;
            return m_dev_focus->set_value(new_val);
        }
        case emulated::software_prop::FocusAuto:
        {
            m_auto_params.focus_onepush_params.is_run_cmd = new_val;
            m_auto_params.focus_onepush_params.run_cmd_params.roi = { m_focus_left,
                                                                      m_focus_top,
                                                                      m_focus_left + m_focus_width,
                                                                      m_focus_top
                                                                          + m_focus_height };
            return outcome::success();
        }
        case emulated::software_prop::FocusAutoTop:
        {
            m_focus_top = new_val;

            // reduce roi height to always remain within image boundaries
            // 0 check as proerties may be set before a format is known
            if (m_format.get_size().height != 0
                && m_focus_top + m_focus_height > (int)m_format.get_size().height)
            {
                m_focus_height = m_format.get_size().height - m_focus_top;
            }
            return outcome::success();
        }
        case emulated::software_prop::FocusAutoLeft:
        {
            m_focus_left = new_val;

            // reduce roi width to always remain within image boundaries
            // 0 check as proerties may be set before a format is known
            if (m_format.get_size().width != 0
                && m_focus_left + m_focus_width > (int)m_format.get_size().width)
            {
                m_focus_width = m_format.get_size().width - m_focus_left;
            }
            return outcome::success();
        }
        case emulated::software_prop::FocusAutoWidth:
        {
            m_focus_width = new_val;
            return outcome::success();
        }
        case emulated::software_prop::FocusAutoHeight:
        {
            m_focus_height = new_val;
            return outcome::success();
        }
        case emulated::software_prop::BalanceWhiteAuto:
        {
            if (new_val == 0 || new_val == 1)
            {
                m_auto_params.wb.auto_enabled = new_val != 0;
                m_auto_params.wb.one_push_enabled = false;
            }
            else
            {
                m_auto_params.wb.auto_enabled = false;
                m_auto_params.wb.one_push_enabled = true;
            }
            return outcome::success();
        }
        case emulated::software_prop::ClaimBalanceWhiteSoftware:
        {
            m_wb.m_wb_is_claimed = new_val;
            return outcome::success();
        }
        case emulated::software_prop::ColorTransformEnable:
        {
            return m_dev_color_transform_enable->set_value(new_val);
        }
    }
    SPDLOG_WARN("Not implemented. ID: {} value: {}", prop_id, new_val);
    return tcam::status::PropertyNotImplemented;
}


outcome::result<double> tcam::property::SoftwareProperties::get_double(
    emulated::software_prop prop_id)
{
    std::scoped_lock lock(m_property_mtx);

    switch (prop_id)
    {
        case emulated::software_prop::ExposureAuto:
        case emulated::software_prop::ExposureAutoUpperLimitAuto:
        case emulated::software_prop::ExposureAutoReference:
        case emulated::software_prop::ExposureAutoHighlightReduction:
        case emulated::software_prop::GainAuto:
        case emulated::software_prop::AutoFunctionsROIEnable:
        case emulated::software_prop::AutoFunctionsROIPreset:
        case emulated::software_prop::AutoFunctionsROILeft:
        case emulated::software_prop::AutoFunctionsROITop:
        case emulated::software_prop::AutoFunctionsROIWidth:
        case emulated::software_prop::AutoFunctionsROIHeight:
        case emulated::software_prop::Iris:
        case emulated::software_prop::IrisAuto:
        case emulated::software_prop::Focus:
        case emulated::software_prop::FocusAuto:
        case emulated::software_prop::FocusAutoTop:
        case emulated::software_prop::FocusAutoLeft:
        case emulated::software_prop::FocusAutoHeight:
        case emulated::software_prop::FocusAutoWidth:
        case emulated::software_prop::BalanceWhiteAuto:
        case emulated::software_prop::ClaimBalanceWhiteSoftware:
        case emulated::software_prop::ColorTransformEnable:
            return tcam::status::PropertyNotImplemented;

        case emulated::software_prop::ExposureTime:
        {
            if (!m_auto_params.exposure.auto_enabled)
            {
                return m_dev_exposure->get_value();
            }
            return m_auto_params.exposure.val;
        }
        case emulated::software_prop::ExposureAutoLowerLimit:
        {
            return m_auto_params.exposure.min;
        }
        case emulated::software_prop::ExposureAutoUpperLimit:
        {
            return m_exposure_auto_upper_limit;
        }
        case emulated::software_prop::Gain:
        {
            if (!m_auto_params.gain.auto_enabled)
            {
                return m_dev_gain->get_value();
            }
            return m_auto_params.gain.value;
        }
        case emulated::software_prop::GainAutoLowerLimit:
        {
            return m_auto_params.gain.min;
        }
        case emulated::software_prop::GainAutoUpperLimit:
        {
            return m_auto_params.gain.max;
        }
        case emulated::software_prop::BalanceWhiteRed:
        {
            return get_whitebalance_channel(emulated::software_prop::BalanceWhiteRed);
        }
        case emulated::software_prop::BalanceWhiteGreen:
        {
            return get_whitebalance_channel(emulated::software_prop::BalanceWhiteGreen);
        }
        case emulated::software_prop::BalanceWhiteBlue:
        {
            return get_whitebalance_channel(emulated::software_prop::BalanceWhiteBlue);
        }
        case emulated::software_prop::ColorTransformRedToRed:
        case emulated::software_prop::ColorTransformGreenToRed:
        case emulated::software_prop::ColorTransformBlueToRed:
        case emulated::software_prop::ColorTransformRedToGreen:
        case emulated::software_prop::ColorTransformGreenToGreen:
        case emulated::software_prop::ColorTransformBlueToGreen:
        case emulated::software_prop::ColorTransformRedToBlue:
        case emulated::software_prop::ColorTransformGreenToBlue:
        case emulated::software_prop::ColorTransformBlueToBlue:
            return get_device_color_transform(prop_id);
    }

    SPDLOG_WARN("not implemented {}", prop_id);
    return tcam::status::PropertyNotImplemented;
}


outcome::result<void> tcam::property::SoftwareProperties::set_double(
    emulated::software_prop prop_id,
    double new_val)
{
    std::scoped_lock lock(m_property_mtx);

    switch (prop_id)
    {
        case emulated::software_prop::ExposureAuto:
        case emulated::software_prop::ExposureAutoUpperLimitAuto:
        case emulated::software_prop::ExposureAutoReference:
        case emulated::software_prop::ExposureAutoHighlightReduction:
        case emulated::software_prop::GainAuto:
        case emulated::software_prop::AutoFunctionsROIEnable:
        case emulated::software_prop::AutoFunctionsROIPreset:
        case emulated::software_prop::AutoFunctionsROILeft:
        case emulated::software_prop::AutoFunctionsROITop:
        case emulated::software_prop::AutoFunctionsROIWidth:
        case emulated::software_prop::AutoFunctionsROIHeight:
        case emulated::software_prop::Iris:
        case emulated::software_prop::IrisAuto:
        case emulated::software_prop::Focus:
        case emulated::software_prop::FocusAuto:
        case emulated::software_prop::FocusAutoTop:
        case emulated::software_prop::FocusAutoLeft:
        case emulated::software_prop::FocusAutoWidth:
        case emulated::software_prop::FocusAutoHeight:
        case emulated::software_prop::BalanceWhiteAuto:
        case emulated::software_prop::ClaimBalanceWhiteSoftware:
        case emulated::software_prop::ColorTransformEnable:
            return tcam::status::PropertyNotImplemented;

        case emulated::software_prop::ExposureTime:
        {
            if (m_auto_params.exposure.auto_enabled)
            {
                return tcam::status::PropertyNotWriteable;
            }
            m_auto_params.exposure.val = new_val;
            return m_dev_exposure->set_value(new_val);
        }
        case emulated::software_prop::ExposureAutoLowerLimit:
        {
            m_auto_params.exposure.min = new_val;
            return outcome::success();
        }
        case emulated::software_prop::ExposureAutoUpperLimit:
        {
            if (m_exposure_auto_upper_limit_auto)
            {
                return tcam::status::PropertyNotWriteable;
            }
            m_exposure_auto_upper_limit = new_val;
            return outcome::success();
        }
        case emulated::software_prop::Gain:
        {
            if (m_auto_params.gain.auto_enabled)
            {
                return tcam::status::PropertyNotWriteable;
            }
            m_auto_params.gain.value = new_val;
            return m_dev_gain->set_value(new_val);
        }
        case emulated::software_prop::GainAutoLowerLimit:
        {
            m_auto_params.gain.min = new_val;
            return outcome::success();
        }
        case emulated::software_prop::GainAutoUpperLimit:
        {
            m_auto_params.gain.max = new_val;
            return outcome::success();
        }
        case emulated::software_prop::BalanceWhiteRed:
        {
            return set_whitebalance_channel(prop_id, new_val);
        }
        case emulated::software_prop::BalanceWhiteGreen:
        {
            return set_whitebalance_channel(prop_id, new_val);
        }
        case emulated::software_prop::BalanceWhiteBlue:
        {
            return set_whitebalance_channel(prop_id, new_val);
        }
        case emulated::software_prop::ColorTransformRedToRed:
        case emulated::software_prop::ColorTransformGreenToRed:
        case emulated::software_prop::ColorTransformBlueToRed:
        case emulated::software_prop::ColorTransformRedToGreen:
        case emulated::software_prop::ColorTransformGreenToGreen:
        case emulated::software_prop::ColorTransformBlueToGreen:
        case emulated::software_prop::ColorTransformRedToBlue:
        case emulated::software_prop::ColorTransformGreenToBlue:
        case emulated::software_prop::ColorTransformBlueToBlue:
        {
            return set_device_color_transform(prop_id, new_val);
        }
    }
    SPDLOG_WARN("not implemented {}", prop_id);
    return tcam::status::PropertyNotImplemented;
}


tcam::property::PropertyFlags tcam::property::SoftwareProperties::get_flags(
    tcam::property::emulated::software_prop id) const
{
    std::scoped_lock lock(m_property_mtx);

    const auto default_flags =
        (PropertyFlags::Available | PropertyFlags::Implemented | PropertyFlags::External);

    auto add_locked = [default_flags](bool lock)
    {
        return lock ? (default_flags | PropertyFlags::Locked) : default_flags;
    };

    switch (id)
    {
        case emulated::software_prop::ExposureTime:
            return add_locked(m_auto_params.exposure.auto_enabled);
        case emulated::software_prop::ExposureAuto:
            return default_flags;
        case emulated::software_prop::ExposureAutoReference:
            return default_flags;
        case emulated::software_prop::ExposureAutoLowerLimit:
            return default_flags;
        case emulated::software_prop::ExposureAutoUpperLimit:
            return add_locked(m_exposure_auto_upper_limit_auto);
        case emulated::software_prop::ExposureAutoUpperLimitAuto:
            return default_flags;
        case emulated::software_prop::ExposureAutoHighlightReduction:
            return default_flags;

        case emulated::software_prop::Gain:
            return add_locked(m_auto_params.gain.auto_enabled);
        case emulated::software_prop::GainAuto:
            return default_flags;
        case emulated::software_prop::GainAutoLowerLimit:
            return default_flags;
        case emulated::software_prop::GainAutoUpperLimit:
            return default_flags;
        case emulated::software_prop::AutoFunctionsROIEnable:
        case emulated::software_prop::AutoFunctionsROIPreset:
        case emulated::software_prop::AutoFunctionsROILeft:
        case emulated::software_prop::AutoFunctionsROITop:
        case emulated::software_prop::AutoFunctionsROIWidth:
        case emulated::software_prop::AutoFunctionsROIHeight:
            return default_flags;
        case emulated::software_prop::Iris:
            return add_locked(m_auto_params.iris.auto_enabled);
        case emulated::software_prop::IrisAuto:
            return default_flags;
        case emulated::software_prop::Focus:
            return default_flags;
        case emulated::software_prop::FocusAuto:
            return default_flags;
        case emulated::software_prop::FocusAutoTop:
            return default_flags;
        case emulated::software_prop::FocusAutoLeft:
            return default_flags;
        case emulated::software_prop::FocusAutoWidth:
            return default_flags;
        case emulated::software_prop::FocusAutoHeight:
            return default_flags;
        case emulated::software_prop::BalanceWhiteAuto:
        {
            if (m_wb.m_wb_is_claimed)
            {
                return default_flags;
            }
            return PropertyFlags::Implemented;
        }
        case emulated::software_prop::BalanceWhiteRed:
        case emulated::software_prop::BalanceWhiteGreen:
        case emulated::software_prop::BalanceWhiteBlue:
        {
            if (m_wb.m_is_software_auto_wb)
            {
                return add_locked(m_auto_params.wb.auto_enabled);
            }

            // this should not occur
            return add_locked(m_auto_params.wb.auto_enabled);
        }
        case emulated::software_prop::ClaimBalanceWhiteSoftware:
            return default_flags | PropertyFlags::Hidden;
        case emulated::software_prop::ColorTransformRedToRed:
        case emulated::software_prop::ColorTransformGreenToRed:
        case emulated::software_prop::ColorTransformBlueToRed:
        case emulated::software_prop::ColorTransformRedToGreen:
        case emulated::software_prop::ColorTransformGreenToGreen:
        case emulated::software_prop::ColorTransformBlueToGreen:
        case emulated::software_prop::ColorTransformRedToBlue:
        case emulated::software_prop::ColorTransformGreenToBlue:
        case emulated::software_prop::ColorTransformBlueToBlue:
        {
            auto res = m_dev_color_transform_enable->get_value();
            if (!res)
            {
                return add_locked(true);
            }
            return add_locked(!res.value());
        }
        case emulated::software_prop::ColorTransformEnable:
            return default_flags;
    }
    return PropertyFlags::None;
}


void tcam::property::SoftwareProperties::update_to_new_format(const tcam::VideoFormat& new_format)
{
    m_frame_counter = 0;
    m_format = new_format;

    auto auto_upper = get_int(sp::ExposureAutoUpperLimitAuto);

    if (auto_upper && auto_upper.value() == 1)
    {
        m_exposure_auto_upper_limit = 1000000 / m_format.get_framerate();
    }

    tcamprop1::prop_range_integer x_range = { 0, m_format.get_size().width, ROI_STEP_SIZE };
    tcamprop1::prop_range_integer y_range = { 0, m_format.get_size().height, ROI_STEP_SIZE };

    if (m_prop_brightness_top)
    {
        m_prop_brightness_top->set_range(y_range);
        m_prop_brightness_left->set_range(x_range);
        m_prop_brightness_width->set_range(x_range);
        m_prop_brightness_height->set_range(y_range);

        // when not using custom settings
        // recalculate all values to adhere to the new ranges
        if (m_brightness_roi_mode != AutoFunctionsROIPreset_Modes::custom)
        {
            set_auto_functions_preset_mode(m_brightness_roi_mode);
        }
    }

    if (m_prop_focus_top)
    {
        m_prop_focus_top->set_range(y_range);
        m_prop_focus_left->set_range(x_range);
        m_prop_focus_width->set_range(x_range);
        m_prop_focus_height->set_range(y_range);
    }
}

void property::SoftwareProperties::add_prop_entry(prop_ptr_vec& v,
                                                  std::string_view name_to_insert_after,
                                                  const prop_ptr_vec& vec_to_add)
{
    auto f = std::find_if(v.begin(),
                          v.end(),
                          [name_to_insert_after](const auto& ptr)
                          { return ptr->get_name() == name_to_insert_after; });
    if (f != v.end())
    {
        ++f; // insert after this
    }
    v.insert(f, vec_to_add.begin(), vec_to_add.end());
}

void property::SoftwareProperties::remove_entry(prop_ptr_vec& v, std::string_view name)
{
    auto f = std::find_if(
        v.begin(), v.end(), [name](const auto& ptr) { return ptr->get_name() == name; });
    if (f == v.end())
    {
        return;
    }
    v.erase(f);
}

void property::SoftwareProperties::replace_entry(prop_ptr_vec& v,
                                                 const std::shared_ptr<IPropertyBase>& prop)
{
    auto f = std::find_if(v.begin(),
                          v.end(),
                          [name = prop->get_name()](const auto& ptr)
                          { return ptr->get_name() == name; });
    if (f == v.end())
    {
        v.push_back(prop);
        return;
    }
    *f = prop;
}
