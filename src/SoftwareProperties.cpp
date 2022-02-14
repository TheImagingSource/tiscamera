

#include "SoftwareProperties.h"

#include "SoftwarePropertiesImpl.h"
#include "logging.h"

#include <algorithm>
#include <chrono>
#include <tcamprop1.0_base/tcamprop_property_info_list.h>

using namespace tcam;

using sp = tcam::property::emulated::software_prop;

namespace
{
namespace prop_lst = tcamprop1::prop_list;

} // namespace

namespace tcam::property
{

SoftwareProperties::SoftwareProperties( const std::vector<std::shared_ptr<tcam::property::IPropertyBase>>& dev_properties )
:     m_device_properties( dev_properties ), p_state( auto_alg::make_state_ptr() )

{}

std::shared_ptr<SoftwareProperties> SoftwareProperties::create(
    const std::vector<std::shared_ptr<tcam::property::IPropertyBase>>& dev_properties,
    bool has_bayer)
{
    auto ptr = std::make_shared<SoftwareProperties>( dev_properties );
    ptr->generate_public_properties( has_bayer);
    return ptr;
}

static uint64_t time_now_in_us() noexcept
{
    return std::chrono::duration_cast<std::chrono::microseconds>(
               std::chrono::high_resolution_clock::now().time_since_epoch())
        .count();
}


void SoftwareProperties::auto_pass(const img::img_descriptor& image)
{
    auto_alg::auto_pass_params tmp_params;
    {
        std::scoped_lock lock(m_property_mtx);

        tmp_params = m_auto_params;

        m_auto_params.focus_onepush_params.is_run_cmd = false;
        tmp_params.exposure.max = m_exposure_auto_upper_limit;
    }

    // TODO: get from ImageBuffer statistics
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

        // SPDLOG_DEBUG("WB r: {}", auto_pass_ret.wb.channels.r * 64.0f);
        // SPDLOG_DEBUG("WB g: {}", auto_pass_ret.wb.channels.g * 64.0f);
        // SPDLOG_DEBUG("WB b: {}", auto_pass_ret.wb.channels.b * 64.0f);
        // SPDLOG_DEBUG("");

        if (m_wb.is_dev_wb())
        {
            auto res = set_device_wb(emulated::software_prop::BalanceWhiteRed,
                                     auto_pass_ret.wb.channels.r);

            if (!res)
            {
                SPDLOG_DEBUG("Setting whitebalance caused an error: {}",
                             res.as_failure().error().message());
            }
            res = set_device_wb(emulated::software_prop::BalanceWhiteGreen,
                                auto_pass_ret.wb.channels.g);
            if (!res)
            {
                SPDLOG_DEBUG("Setting whitebalance caused an error: {}",
                             res.as_failure().error().message());
            }

            res = set_device_wb(emulated::software_prop::BalanceWhiteBlue,
                                auto_pass_ret.wb.channels.b);
            if (!res)
            {
                SPDLOG_DEBUG("Setting whitebalance caused an error: {}",
                             res.as_failure().error().message());
            }
        }
    }
    else
    {
        // SPDLOG_DEBUG("WB not active");
    }
}


void tcam::property::SoftwareProperties::generate_public_properties(bool has_bayer)
{
    m_auto_params = {};

    auto has_exposure_auto = find_property(m_device_properties, "ExposureAuto") != nullptr;
    if (!has_exposure_auto)
    {
        generate_exposure_auto();
    }

    auto has_gain_auto = find_property(m_device_properties, "GainAuto") != nullptr;
    if (!has_gain_auto)
    {
        generate_gain_auto();
    }

    auto has_iris = find_property(m_device_properties, "Iris") != nullptr;
    auto has_iris_auto = find_property(m_device_properties, "IrisAuto") != nullptr;
    if (has_iris && !has_iris_auto)
    {
        // cameras can has Iris behavior that prohibits IrisAuto
        // example would be the AFU420
        // the iris on that camera is either open or closed
        // check for that before adding IrisAuto
        auto valid_iris_range = [=] ()
        {
            m_dev_iris = tcam::property::find_property<IPropertyInteger>(m_device_properties, "Iris");
            if (!m_dev_iris)
            {
                return false;
            }

            auto min = m_dev_iris->get_range().min;
            auto max = m_dev_iris->get_range().max;

            if (min == 0 && max == 1)
            {
                return false;
            }
            return true;
        };

        if (valid_iris_range())
        {
            generate_iris_auto();
        }
    }

    auto has_foucs = find_property(m_device_properties, "Focus") != nullptr;
    auto has_focus_auto = find_property(m_device_properties, "FocusAuto") != nullptr;
    if (has_foucs && !has_focus_auto)
    {
        generate_focus_auto();
    }

    { // BalanceWhite stuff
        if (has_bayer)
        {
            bool has_wb_auto = find_property(m_device_properties, "BalanceWhiteAuto") != nullptr;
            if (!has_wb_auto)
            {
                generate_balance_white_auto();
            }
        }
    }

    { // ColorTransformation stuff
        auto enable = tcam::property::find_property(m_device_properties, "ColorTransformationEnable");

        auto selector = tcam::property::find_property(m_device_properties, "ColorTransformationSelector");
        auto value = tcam::property::find_property(m_device_properties, "ColorTransformationValue");
        auto value_selector = tcam::property::find_property(m_device_properties, "ColorTransformationValueSelector");

        if (enable && value && value_selector)
        {
            generate_color_transformation();
        }
    }

    // as a final step compare generated properties to device properties
    // pass along everything we do not need to intercept
    // some may not be intercepted but replaced with a different interface
    // those are typically only identified by name

    auto contains = [](const auto& props, const std::shared_ptr<IPropertyBase>& elem)
    {
        return std::any_of(props.begin(),
                           props.end(),
                           [&](const auto& ptr) { return ptr->get_name() == elem->get_name(); });
    };

    auto contains2 = [](const auto& props, const std::shared_ptr<IPropertyBase>& elem)
    {
        return std::any_of(
            props.begin(), props.end(), [&](auto name) { return name == elem->get_name(); });
    };

    std::array<std::string_view, 5> prop_black_list = {
        "BalanceRatioRaw",
        "BalanceRatioSelector",
        "BalanceRatio",
        "ColorTransformationValue",
        "ColorTransformationValueSelector",
    };

    for (auto& p : m_device_properties)
    {
        if (!contains(m_properties, p) && !contains2(prop_black_list, p))
        {
            m_properties.push_back(p);
        }
    }
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

            return tcam::status::NotImplemented;

        case emulated::software_prop::ExposureAuto:
            return m_auto_params.exposure.auto_enabled ? 1 : 0;
        case emulated::software_prop::ExposureAutoUpperLimitAuto:
            return m_exposure_upper_auto;
        case emulated::software_prop::ExposureAutoReference:
            return m_auto_params.exposure_reference.val;
        case emulated::software_prop::ExposureAutoHighlightReduction:
            return m_auto_params.enable_highlight_reduction ? 1 : 0;

        case emulated::software_prop::GainAuto:
            return m_auto_params.gain.auto_enabled ? 1 : 0;

        case emulated::software_prop::Iris:
            return m_auto_params.iris.val;
        case emulated::software_prop::IrisAuto:
            return m_auto_params.iris.auto_enabled ? 1 : 0;
        case emulated::software_prop::Focus:
            return m_auto_params.focus_onepush_params.device_focus_val;
        case emulated::software_prop::FocusAuto:
            return m_auto_params.focus_onepush_params.is_run_cmd;
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
            return m_wb_is_claimed;
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
    return tcam::status::NotImplemented;
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
            return tcam::status::NotImplemented;

        case emulated::software_prop::ExposureAuto:
        {
            m_auto_params.exposure.auto_enabled = new_val;
            return outcome::success();
        }
        case emulated::software_prop::ExposureAutoUpperLimitAuto:
        {
            m_exposure_upper_auto = new_val;

            if (m_exposure_upper_auto)
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
        case emulated::software_prop::Iris:
        {
            if (m_auto_params.iris.auto_enabled)
            {
                return tcam::status::PropertyIsLocked;
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
            m_wb_is_claimed = new_val;
            return outcome::success();
        }
        case emulated::software_prop::ColorTransformEnable:
        {
            return m_dev_color_transform_enable->set_value(new_val);

        }
    }
    SPDLOG_WARN("Not implemented. ID: {} value: {}", prop_id, new_val);
    return tcam::status::NotImplemented;
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
        case emulated::software_prop::Iris:
        case emulated::software_prop::IrisAuto:
        case emulated::software_prop::Focus:
        case emulated::software_prop::FocusAuto:
        case emulated::software_prop::BalanceWhiteAuto:
        case emulated::software_prop::ClaimBalanceWhiteSoftware:
        case emulated::software_prop::ColorTransformEnable:
            return tcam::status::NotImplemented;

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
            if (m_wb.is_dev_wb())
            {
                return get_device_wb(emulated::software_prop::BalanceWhiteRed);
            }
            return m_auto_params.wb.channels.r;
        }
        case emulated::software_prop::BalanceWhiteGreen:
        {
            if (m_wb.is_dev_wb())
            {
                return get_device_wb(emulated::software_prop::BalanceWhiteGreen);
            }
            return m_auto_params.wb.channels.g;
        }
        case emulated::software_prop::BalanceWhiteBlue:
        {
            if (m_wb.is_dev_wb())
            {
                return get_device_wb(emulated::software_prop::BalanceWhiteBlue);
            }
            return m_auto_params.wb.channels.b;
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
    return tcam::status::NotImplemented;
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
        case emulated::software_prop::Iris:
        case emulated::software_prop::IrisAuto:
        case emulated::software_prop::Focus:
        case emulated::software_prop::FocusAuto:
        case emulated::software_prop::BalanceWhiteAuto:
        case emulated::software_prop::ClaimBalanceWhiteSoftware:
        case emulated::software_prop::ColorTransformEnable:
            return tcam::status::NotImplemented;

        case emulated::software_prop::ExposureTime:
        {
            if (m_auto_params.exposure.auto_enabled)
            {
                return tcam::status::PropertyIsLocked;
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
            if (m_exposure_upper_auto)
            {
                return tcam::status::PropertyIsLocked;
            }
            m_exposure_auto_upper_limit = new_val;
            return outcome::success();
        }
        case emulated::software_prop::Gain:
        {
            if (m_auto_params.gain.auto_enabled)
            {
                return tcam::status::PropertyIsLocked;
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
            m_auto_params.wb.channels.r = new_val;
            if (m_wb.is_dev_wb())
            {
                return set_device_wb(prop_id, new_val);
            }
            return outcome::success();
        }
        case emulated::software_prop::BalanceWhiteGreen:
        {
            m_auto_params.wb.channels.g = new_val;
            if (m_wb.is_dev_wb())
            {
                return set_device_wb(prop_id, new_val);
            }
            return outcome::success();
        }
        case emulated::software_prop::BalanceWhiteBlue:
        {
            m_auto_params.wb.channels.b = new_val;
            if (m_wb.is_dev_wb())
            {
                return set_device_wb(prop_id, new_val);
            }
            return outcome::success();
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
    return tcam::status::NotImplemented;
}


tcam::property::PropertyFlags SoftwareProperties::get_flags(emulated::software_prop id) const
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
            return add_locked(m_exposure_upper_auto);
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

        case emulated::software_prop::Iris:
            return add_locked(m_auto_params.iris.auto_enabled);
        case emulated::software_prop::IrisAuto:
            return default_flags;
        case emulated::software_prop::Focus:
            return default_flags;
        case emulated::software_prop::FocusAuto:
            return default_flags;

        case emulated::software_prop::BalanceWhiteAuto:
        {
            if (m_wb_is_claimed)
            {
                return default_flags;
            }
            return PropertyFlags::Implemented;
        }
        case emulated::software_prop::BalanceWhiteRed:
        case emulated::software_prop::BalanceWhiteGreen:
        case emulated::software_prop::BalanceWhiteBlue:
        {
            if(m_is_software_auto_wb)
            {
                return add_locked(m_auto_params.wb.auto_enabled);
            }

            assert( m_is_software_auto_wb );
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


void SoftwareProperties::update_to_new_format(const tcam::VideoFormat& new_format)
{
    m_frame_counter = 0;
    m_format = new_format;

    if (get_int(sp::ExposureAutoUpperLimitAuto))
    {
        m_exposure_auto_upper_limit = 1000000 / m_format.get_framerate();
    }
}

void SoftwareProperties::generate_focus_auto()
{
    m_dev_focus = tcam::property::find_property<IPropertyInteger>(m_device_properties, "Focus");
    if (!m_dev_focus)
    {
        return;
    }

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

    add_prop_entry(sp::FocusAuto,
                   &prop_lst::FocusAuto,
                   emulated::to_range(prop_lst::enum_entries_off_once),
                   0);
    add_prop_entry(sp::Focus, &prop_lst::Focus, emulated::to_range(*m_dev_focus));
}

} // namespace tcam::property
