

#include "SoftwareProperties.h"

#include "SoftwarePropertiesImpl.h"
#include "logging.h"

#include <algorithm>
#include <chrono>
#include <map>

using namespace tcam;
using namespace tcam::property;

using sp = tcam::property::emulated::software_prop;

namespace
{

static constexpr tcam_value_double balance_white_channel_range = { 0.0, 4.0, 0.1, 1.0, 1.0 };

static emulated::software_prop_desc prop_list[] = {
    { sp::ExposureTime, "ExposureTime", TCAM_PROPERTY_TYPE_DOUBLE },
    { sp::ExposureAuto, "ExposureAuto", { { { 0, "Off" }, { 1, "Continuous" } } }, 1 },
    { sp::ExposureAutoLowerLimit, "ExposureAutoLowerLimit", TCAM_PROPERTY_TYPE_DOUBLE },
    { sp::ExposureAutoUpperLimit, "ExposureAutoUpperLimit", TCAM_PROPERTY_TYPE_DOUBLE },
    { sp::ExposureAutoUpperLimitAuto,
      "ExposureAutoUpperLimitAuto",
      { { { 0, "Off" }, { 1, "On" } } },
      1 },
    { sp::ExposureAutoReference, "ExposureAutoReference", tcam_value_int { 0, 255, 1, 128, 128 } },

    { sp::Gain, "Gain", TCAM_PROPERTY_TYPE_DOUBLE },
    { sp::GainAuto, "GainAuto", { { { 0, "Off" }, { 1, "Continuous" } } }, 1 },
    { sp::GainAutoLowerLimit, "GainAutoLowerLimit", TCAM_PROPERTY_TYPE_DOUBLE },
    { sp::GainAutoUpperLimit, "GainAutoUpperLimit", TCAM_PROPERTY_TYPE_DOUBLE },

    { sp::Iris, "Iris", TCAM_PROPERTY_TYPE_INTEGER },
    { sp::IrisAuto, "IrisAuto", { { { 0, "Off" }, { 1, "Continuous" } } }, 1 },

    { sp::Focus, "Focus", TCAM_PROPERTY_TYPE_INTEGER },
    { sp::FocusAuto, "FocusAuto", { { { 0, "Off" }, { 1, "Once" } } }, 0 },
    { sp::WB_AUTO,
      "BalanceWhiteAuto",
      { { { 0, "Off" }, { 1, "Once" }, { 2, "Continuous" } } },
      2 },
    { sp::WB_RED, "BalanceWhiteRed", balance_white_channel_range },
    { sp::WB_GREEN, "BalanceWhiteGreen", balance_white_channel_range },
    { sp::WB_BLUE, "BalanceWhiteBlue", balance_white_channel_range },
    { sp::WB_CLAIM, "ClaimBalanceWhiteSoftware", TCAM_PROPERTY_TYPE_BOOLEAN, },
};


const emulated::software_prop_desc* find_property_desc(sp id)
{
    for (const auto& e : prop_list)
    {
        if (e.id_ == id)
        {
            return &e;
        }
    }
    return nullptr;
}


} // anonymous namespace


namespace tcam::property
{

SoftwareProperties::SoftwareProperties(
    const std::vector<std::shared_ptr<tcam::property::IPropertyBase>>& dev_properties,
    bool has_bayer)
    : m_device_properties(dev_properties)
{
    m_backend = std::make_shared<emulated::SoftwarePropertyBackend>(this);
    p_state = auto_alg::make_state_ptr();
    generate_public_properties(has_bayer);
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
        m_auto_params.wb.channels.r = auto_pass_ret.wb.channels.r;
        m_auto_params.wb.channels.g = auto_pass_ret.wb.channels.g;
        m_auto_params.wb.channels.b = auto_pass_ret.wb.channels.b;

        // SPDLOG_DEBUG("WB r: {}", auto_pass_ret.wb.channels.r * 64.0f);
        // SPDLOG_DEBUG("WB g: {}", auto_pass_ret.wb.channels.g * 64.0f);
        // SPDLOG_DEBUG("WB b: {}", auto_pass_ret.wb.channels.b * 64.0f);
        // SPDLOG_DEBUG("");

        if (m_wb.is_dev_wb())
        {
            auto res = set_device_wb(emulated::software_prop::WB_RED, auto_pass_ret.wb.channels.r);

            if (!res)
            {
                SPDLOG_ERROR("Setting whitebalance caused an error: {}", res.as_failure().error().message());
                return;
            }

            res = set_device_wb(emulated::software_prop::WB_GREEN, auto_pass_ret.wb.channels.g);

            if (!res)
            {
                SPDLOG_ERROR("Setting whitebalance caused an error: {}", res.as_failure().error().message());
                return;
            }

            res = set_device_wb(emulated::software_prop::WB_BLUE, auto_pass_ret.wb.channels.b);

            if (!res)
            {
                SPDLOG_ERROR("Setting whitebalance caused an error: {}", res.as_failure().error().message());
                return;
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

    auto iter_exp = find_property(m_device_properties, "ExposureAuto");

    if (!iter_exp)
    {
        generate_exposure();
    }

    auto iter_gain = find_property(m_device_properties, "GainAuto");

    if (!iter_gain)
    {
        generate_gain();
    }

    auto iter_iris = find_property(m_device_properties, "Iris");
    auto iter_iris_auto = find_property(m_device_properties, "IrisAuto");

    if (iter_iris && !iter_iris_auto)
    {
        generate_iris();
    }

    auto iter_focus = find_property(m_device_properties, "Focus");
    auto iter_focus_auto = find_property(m_device_properties, "FocusAuto");

    if (iter_focus && !iter_focus_auto)
    {
        generate_focus();
    }

    if (has_bayer)
    {
        auto iter_wb = find_property(m_device_properties, "BalanceWhiteAuto");
        if (!iter_wb)
        {
            generate_whitebalance();
            SPDLOG_INFO("generating");
        }
        else
        {
            auto base_selector =
                tcam::property::find_property(m_device_properties, "BalanceRatioSelector");
            auto base_raw = tcam::property::find_property(m_device_properties, "BalanceRatioRaw");

            if (!base_raw)
            {
                base_raw = tcam::property::find_property(m_device_properties, "BalanceRatio");
            }

            if (base_selector && base_raw)
            {
                // this means we have whitebalance in the device but no nice properties
                convert_whitebalance();
            }
        }
    }

    // as a final step compare generated properties to device properties
    // pass along everything we do not need to intercept
    // some may not be intercepted but replaced with a different interface
    // those are typically only identified by name

    auto contains = [&](const std::shared_ptr<IPropertyBase>& elem)
    {
        return std::any_of(m_properties.begin(), m_properties.end(),
                           [&](const std::shared_ptr<IPropertyBase>& x)
        {
                if (x->get_name() == elem->get_name())
                {
                    return true;
                }
                return false;
            });
    };

    std::vector<std::string> additional_properties =
        {
            "BalanceRatioRaw", "BalanceRatioSelector", "BalanceRatio",
        };

    for (auto& p : m_device_properties)
    {
        if (!contains(p)
            && std::find(additional_properties.begin(), additional_properties.end(), p->get_name())
                   == additional_properties.end())
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
        case emulated::software_prop::ExposureAuto:
        {
            return m_auto_params.exposure.auto_enabled;
        }
        case emulated::software_prop::ExposureAutoUpperLimitAuto:
        {
            return m_exposure_upper_auto;
        }
        case emulated::software_prop::ExposureAutoReference:
        {
            return m_auto_params.exposure_reference.val;
        }
        case emulated::software_prop::GainAuto:
        {
            return m_auto_params.gain.auto_enabled;
        }
        case emulated::software_prop::Iris:
        {
            return m_auto_params.iris.val;
        }
        case emulated::software_prop::IrisAuto:
        {
            return m_auto_params.iris.auto_enabled;
        }
        case emulated::software_prop::Focus:
        {
            return m_auto_params.focus_onepush_params.device_focus_val;
        }
        case emulated::software_prop::FocusAuto:
        {
            return m_auto_params.focus_onepush_params.is_run_cmd;
        }
        case emulated::software_prop::WB_AUTO:
        {
            if (m_auto_params.wb.one_push_enabled)
            {
                return 1;
            }
            if (m_auto_params.wb.auto_enabled)
            {
                return 2;
            }
            return 0;
        }
        case emulated::software_prop::WB_CLAIM:
        {
            return m_wb_is_claimed;
        }
        default:
            return false;
    }
}

outcome::result<void> tcam::property::SoftwareProperties::set_int(emulated::software_prop prop_id,
                                                                  int64_t new_val)
{
    std::scoped_lock lock(m_property_mtx);

    switch (prop_id)
    {
        case emulated::software_prop::ExposureAuto:
        {
            m_auto_params.exposure.auto_enabled = new_val;

            set_locked(emulated::software_prop::ExposureTime, new_val);
            return outcome::success();
        }
        case emulated::software_prop::ExposureAutoUpperLimitAuto:
        {
            m_exposure_upper_auto = new_val;

            if (m_exposure_upper_auto )
            {
                if( m_format.get_framerate() != 0 ) {
                    m_exposure_auto_upper_limit = 1'000'000 / m_format.get_framerate();
                }
            }
            set_locked(emulated::software_prop::ExposureAutoUpperLimit, new_val);

            return outcome::success();
        }
        case emulated::software_prop::ExposureAutoReference:
        {
            m_auto_params.exposure_reference.val = new_val;
            return outcome::success();
        }
        case emulated::software_prop::GainAuto:
        {
            m_auto_params.gain.auto_enabled = new_val;
            set_locked(emulated::software_prop::Gain, new_val);
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
            set_locked(emulated::software_prop::Iris, new_val);

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
        case emulated::software_prop::WB_AUTO:
        {
            if (new_val == 0 || new_val == 2)
            {
                m_auto_params.wb.auto_enabled = new_val;

                set_locked(emulated::software_prop::WB_RED, new_val);
                set_locked(emulated::software_prop::WB_GREEN, new_val);
                set_locked(emulated::software_prop::WB_BLUE, new_val);
            }
            else
            {
                m_auto_params.wb.one_push_enabled = true;
            }
            return outcome::success();
        }
        case emulated::software_prop::WB_CLAIM:
        {
            m_wb_is_claimed = new_val;
            return outcome::success();
        }
        default:
        {
            SPDLOG_WARN("Not implemented. ID: {} value: {}", prop_id, new_val);
            return tcam::status::NotImplemented;
        }
    }
}


outcome::result<double> tcam::property::SoftwareProperties::get_double(
    emulated::software_prop prop_id)
{
    std::scoped_lock lock(m_property_mtx);

    switch (prop_id)
    {
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
        case emulated::software_prop::WB_RED:
        {
           if (m_wb.is_dev_wb())
            {
                return get_device_wb(emulated::software_prop::WB_RED);
            }
            else
            {
                return m_auto_params.wb.channels.r;
            }
        }
        case emulated::software_prop::WB_GREEN:
        {
            if (m_wb.is_dev_wb())
            {
                return get_device_wb(emulated::software_prop::WB_GREEN);
            }
            else
            {
                return m_auto_params.wb.channels.g;
            }
        }
        case emulated::software_prop::WB_BLUE:
        {
            if (m_wb.is_dev_wb())
            {
                return get_device_wb(emulated::software_prop::WB_BLUE);
            }
            else
            {
                return m_auto_params.wb.channels.b;
            }
        }
        default:
        {
            SPDLOG_WARN("not implemented '{}' {}", find_property_name(prop_id), prop_id);
            return 0.0;
        }
    }
}


outcome::result<void> tcam::property::SoftwareProperties::set_double(
    emulated::software_prop prop_id,
    double new_val)
{
    std::scoped_lock lock(m_property_mtx);

    switch (prop_id)
    {
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
            if (!m_exposure_upper_auto)
            {
                m_exposure_auto_upper_limit = new_val;
            }
            else
            {
                SPDLOG_WARN("ExposureAutoUpperLimitAuto is still active.");
                return tcam::status::PropertyIsLocked;
            }
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
        case emulated::software_prop::WB_RED:
        {
            m_auto_params.wb.channels.r = new_val;
            if (m_wb.is_dev_wb())
            {
                return set_device_wb(prop_id, new_val);
            }
            return outcome::success();
        }
        case emulated::software_prop::WB_GREEN:
        {
            m_auto_params.wb.channels.g = new_val;
            if (m_wb.is_dev_wb())
            {
                return set_device_wb(prop_id, new_val);
            }
            return outcome::success();
        }
        case emulated::software_prop::WB_BLUE:
        {
            m_auto_params.wb.channels.b = new_val;
            if (m_wb.is_dev_wb())
            {
                return set_device_wb(prop_id, new_val);
            }
            return outcome::success();
        }
        default:
        {
            SPDLOG_WARN("not implemented {} {}", find_property_name(prop_id), prop_id);

            return tcam::status::NotImplemented;
        }
    }
}


tcam::property::PropertyFlags SoftwareProperties::get_flags(emulated::software_prop id)
{
    std::scoped_lock lock(m_property_mtx);

    switch (id)
    {
        case emulated::software_prop::WB_RED:
        case emulated::software_prop::WB_GREEN:
        case emulated::software_prop::WB_BLUE:
        {
            if (m_wb.m_dev_wb_ratio)
            {
                return m_wb.m_dev_wb_ratio->get_flags();
            }
            return PropertyFlags::None;
        }
        default:
        {
            return PropertyFlags::None;
        }
    }
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


void tcam::property::SoftwareProperties::generate_exposure()
{
    auto exp_base = tcam::property::find_property(m_device_properties, "ExposureTime");
    if (!exp_base)
    {
        SPDLOG_ERROR("Unable to identify exposure interface.");
        return;
    }

    m_dev_exposure = std::static_pointer_cast<tcam::property::IPropertyFloat>(exp_base);

    m_auto_params.exposure.granularity = m_dev_exposure->get_range().stp;

    m_auto_params.exposure.min = m_dev_exposure->get_range().min;
    m_auto_params.exposure.max = m_dev_exposure->get_range().max;

    auto exp_val = m_dev_exposure->get_value();
    if (exp_val)
    {
        m_auto_params.exposure.val = exp_val.value();
    }
    else
    {
        SPDLOG_ERROR("Unable to retrieve ExposureTime value: {}", exp_val.error().message());
    }

    m_auto_params.exposure.auto_enabled = true;

    m_exposure_auto_upper_limit = m_auto_params.exposure.max;

    auto desc_ref = find_property_desc(sp::ExposureAutoReference);

    m_brightness_reference.min = desc_ref->range_i_.min;
    m_brightness_reference.max = desc_ref->range_i_.max;
    m_brightness_reference.val = desc_ref->range_i_.value;

    enable_property_double(sp::ExposureTime, m_dev_exposure);
    enable_property(sp::ExposureAuto);
    enable_property_double(sp::ExposureAutoLowerLimit, m_dev_exposure);
    enable_property_double(sp::ExposureAutoUpperLimit, m_dev_exposure);
    enable_property(sp::ExposureAutoReference);
    enable_property(sp::ExposureAutoUpperLimitAuto);

    set_locked(emulated::software_prop::ExposureTime, true);
    set_locked(emulated::software_prop::ExposureAutoUpperLimit, true);
}


void tcam::property::SoftwareProperties::generate_gain()
{
    auto gain_base = tcam::property::find_property(m_device_properties, "Gain");

    if (!gain_base)
    {
        SPDLOG_ERROR("Unable to identify gain interface.");
        return;
    };

    m_dev_gain = std::dynamic_pointer_cast<tcam::property::IPropertyFloat>(gain_base);

    m_auto_params.gain.auto_enabled = true;
    m_auto_params.gain.min = m_dev_gain->get_range().min;
    m_auto_params.gain.max = m_dev_gain->get_range().max;
    auto gain_val = m_dev_gain->get_value();
    if (gain_val)
    {
        m_auto_params.gain.value = gain_val.value();
    }
    else
    {
        SPDLOG_ERROR("Unable to retrieve Gain: {}", gain_val.error().message());
    }

    enable_property_double(sp::Gain, m_dev_gain);
    enable_property(sp::GainAuto);
    enable_property_double(sp::GainAutoLowerLimit, m_dev_gain);
    enable_property_double(sp::GainAutoUpperLimit, m_dev_gain);

    set_locked(emulated::software_prop::Gain, true);
}


void tcam::property::SoftwareProperties::generate_iris()
{
    auto base = tcam::property::find_property(m_device_properties, "Iris");

    if (!base)
    {
        SPDLOG_ERROR("Unable to identify iris interface.");
        return;
    };

    m_dev_iris = std::dynamic_pointer_cast<tcam::property::IPropertyInteger>(base);

    m_auto_params.iris.min = m_dev_iris->get_range().min;
    m_auto_params.iris.max = m_dev_iris->get_range().max;

    auto iris_val = m_dev_iris->get_value();
    if (iris_val)
    {
        m_auto_params.iris.val = iris_val.value();
    }
    else
    {
        SPDLOG_ERROR("Unable to retrieve Iris value: {}", iris_val.error().message());
    }
    // TODO: granularity/step

    enable_property_int(sp::Iris, m_dev_iris);
    enable_property(sp::IrisAuto);
}


void SoftwareProperties::generate_focus()
{
    auto base = tcam::property::find_property(m_device_properties, "Focus");

    if (!base)
    {
        SPDLOG_ERROR("Unable to identify focus interface.");
        return;
    };

    m_dev_focus = std::dynamic_pointer_cast<tcam::property::IPropertyInteger>(base);

    m_auto_params.focus_onepush_params.enable_focus = true;
    m_auto_params.focus_onepush_params.run_cmd_params.focus_range_min = m_dev_focus->get_range().min;
    m_auto_params.focus_onepush_params.run_cmd_params.focus_range_max = m_dev_focus->get_range().max;

    enable_property(sp::FocusAuto);
    enable_property_int(sp::Focus, m_dev_focus);
}


void SoftwareProperties::convert_whitebalance()
{
    auto base_r = tcam::property::find_property(m_device_properties, "BalanceWhiteRed");
    auto base_g = tcam::property::find_property(m_device_properties, "BalanceWhiteGreen");
    auto base_b = tcam::property::find_property(m_device_properties, "BalanceWhiteBlue");
    auto base_selector = tcam::property::find_property(m_device_properties, "BalanceRatioSelector");
    auto base_raw = tcam::property::find_property(m_device_properties, "BalanceRatioRaw");

    if (!base_raw)
    {
        base_raw = tcam::property::find_property(m_device_properties, "BalanceRatio");
    }

    m_wb = {};

    if (base_r && base_g && base_b)
    {
        m_wb.type = wb_type::DevChannel;

        m_wb.m_dev_wb_r = std::dynamic_pointer_cast<tcam::property::IPropertyFloat>(base_r);
        m_wb.m_dev_wb_g = std::dynamic_pointer_cast<tcam::property::IPropertyFloat>(base_g);
        m_wb.m_dev_wb_b = std::dynamic_pointer_cast<tcam::property::IPropertyFloat>(base_b);

        enable_property(sp::WB_RED);
        enable_property(sp::WB_GREEN);
        enable_property(sp::WB_BLUE);

        m_wb_is_claimed = true;
        m_auto_params.wb.is_software_whitebalance = false;
    }
    else if (base_raw && base_selector)
    {
        m_auto_params.wb.is_software_whitebalance = false;
        m_wb_is_claimed = true;

        m_wb.m_dev_wb_ratio = std::dynamic_pointer_cast<tcam::property::IPropertyFloat>(base_raw);
        m_wb.m_dev_wb_selector = std::dynamic_pointer_cast<tcam::property::IPropertyEnum>(base_selector);

        if (!m_wb.m_dev_wb_ratio || !m_wb.m_dev_wb_selector)
        {
            SPDLOG_ERROR("Unable to correctly identify balance white properties. Balance White will not work correctly.");
            return;
        }

        m_wb.type = wb_type::DevSelector;

        enable_property(sp::WB_RED, true);
        enable_property(sp::WB_GREEN, true);
        enable_property(sp::WB_BLUE, true);
    }

    m_auto_params.wb.auto_enabled = false;
}


void SoftwareProperties::generate_whitebalance()
{
    auto base_r = tcam::property::find_property(m_device_properties, "BalanceWhiteRed");
    auto base_g = tcam::property::find_property(m_device_properties, "BalanceWhiteGreen");
    auto base_b = tcam::property::find_property(m_device_properties, "BalanceWhiteBlue");
    auto base_selector = tcam::property::find_property(m_device_properties, "BalanceRatioSelector");
    auto base_raw = tcam::property::find_property(m_device_properties, "BalanceRatioRaw");

    if (!base_raw)
    {
        base_raw = tcam::property::find_property(m_device_properties, "BalanceRatio");
    }

    m_wb = {};

    if (base_r && base_g && base_b)
    {
        m_wb.type = wb_type::DevChannel;

        m_wb.m_dev_wb_r = std::dynamic_pointer_cast<tcam::property::IPropertyFloat>(base_r);
        m_wb.m_dev_wb_g = std::dynamic_pointer_cast<tcam::property::IPropertyFloat>(base_g);
        m_wb.m_dev_wb_b = std::dynamic_pointer_cast<tcam::property::IPropertyFloat>(base_b);

        enable_property_double(sp::WB_RED, m_wb.m_dev_wb_r);
        enable_property_double(sp::WB_GREEN, m_wb.m_dev_wb_g);
        enable_property_double(sp::WB_BLUE, m_wb.m_dev_wb_b);

        m_wb_is_claimed = true;
        m_auto_params.wb.is_software_whitebalance = false;
    }
    else if (base_raw && base_selector)
    {
        m_auto_params.wb.is_software_whitebalance = false;
        m_wb_is_claimed = true;

        m_wb.m_dev_wb_ratio = std::dynamic_pointer_cast<tcam::property::IPropertyFloat>(base_raw);
        m_wb.m_dev_wb_selector = std::dynamic_pointer_cast<tcam::property::IPropertyEnum>(base_selector);

        m_wb.type = wb_type::DevSelector;

        enable_property(sp::WB_RED);
        enable_property(sp::WB_GREEN);
        enable_property(sp::WB_BLUE);
    }
    else
    {
        m_auto_params.wb.is_software_whitebalance = true;
        m_wb_is_claimed = false;

        enable_property(sp::WB_CLAIM);

        enable_property(sp::WB_RED);
        enable_property(sp::WB_GREEN);
        enable_property(sp::WB_BLUE);
    }
    enable_property(sp::WB_AUTO);

    set_locked(sp::WB_RED, true);
    set_locked(sp::WB_GREEN, true);
    set_locked(sp::WB_BLUE, true);

    m_auto_params.wb.auto_enabled = true;
}


outcome::result<double> SoftwareProperties::get_device_wb(emulated::software_prop prop_id)
{
    if (prop_id != emulated::software_prop::WB_RED
        && prop_id != emulated::software_prop::WB_GREEN
        && prop_id != emulated::software_prop::WB_BLUE)
    {
        SPDLOG_ERROR("Not a whitebalance property");
        return tcam::status::NotSupported;
    }

    if (m_wb.type == wb_type::DevChannel)
    {
        outcome::result<double> dev_val = [&]() -> outcome::result<double> {
            if (prop_id == emulated::software_prop::WB_RED)
            {
                return m_wb.m_dev_wb_r->get_value();
            }
            else if (prop_id == emulated::software_prop::WB_GREEN)
            {
                return m_wb.m_dev_wb_g->get_value();
            }
            else if (prop_id == emulated::software_prop::WB_BLUE)
            {
                return m_wb.m_dev_wb_b->get_value();
            }

            return tcam::status::NotSupported;
        }();

        if (dev_val)
        {
            return dev_val.value();
        }
        return dev_val.as_failure();
    }
    else if (m_wb.type == wb_type::DevSelector)
    {
        if (prop_id == emulated::software_prop::WB_RED)
        {
            const std::string tmp = std::string(m_wb.m_dev_wb_selector->get_value().value());

            auto ret = m_wb.m_dev_wb_selector->set_value_str("Red");
            if (!ret)
            {
                return ret.as_failure();
            }

            auto dev_value = m_wb.m_dev_wb_ratio->get_value();
            if (dev_value)
            {
                return dev_value.value();
            }

            ret = m_wb.m_dev_wb_selector->set_value_str(tmp);
            return dev_value.as_failure();
        }
        else if (prop_id == emulated::software_prop::WB_GREEN)
        {
            const std::string tmp = std::string(m_wb.m_dev_wb_selector->get_value().value());

            auto ret = m_wb.m_dev_wb_selector->set_value_str("Green");
            if (!ret)
            {
                return ret.as_failure();
            }

            auto dev_value = m_wb.m_dev_wb_ratio->get_value();
            if (dev_value)
            {
                return dev_value.value();
            }

            ret = m_wb.m_dev_wb_selector->set_value_str(tmp);
            return dev_value.as_failure();
        }
        else if (prop_id == emulated::software_prop::WB_BLUE)
        {
            const std::string tmp = std::string(m_wb.m_dev_wb_selector->get_value().value());

            auto ret = m_wb.m_dev_wb_selector->set_value_str("Blue");
            if (!ret)
            {
                return ret.as_failure();
            }

            auto dev_value = m_wb.m_dev_wb_ratio->get_value();
            if (dev_value)
            {
                return dev_value.value();
            }

            ret = m_wb.m_dev_wb_selector->set_value_str(tmp);
            return dev_value.as_failure();
        }
    }

    return tcam::status::NotSupported;
}


outcome::result<void> SoftwareProperties::set_device_wb(emulated::software_prop prop_id, double new_value_tmp)
{
    if (prop_id != emulated::software_prop::WB_RED
        && prop_id != emulated::software_prop::WB_GREEN
        && prop_id != emulated::software_prop::WB_BLUE)
    {
        SPDLOG_ERROR("Not a whitebalance property");
        return tcam::status::NotSupported;
    }

    if (m_wb.type == wb_type::DevChannel)
    {
        if (prop_id == emulated::software_prop::WB_RED)
        {
            return m_wb.m_dev_wb_r->set_value(new_value_tmp);
        }
        else if (prop_id == emulated::software_prop::WB_GREEN)
        {
            return m_wb.m_dev_wb_g->set_value(new_value_tmp);
        }
        else if (prop_id == emulated::software_prop::WB_BLUE)
        {
            return m_wb.m_dev_wb_b->set_value(new_value_tmp);
        }
    }
    else if (m_wb.type == wb_type::DevSelector)
    {
        if (prop_id == emulated::software_prop::WB_RED)
        {
            const std::string tmp = std::string(m_wb.m_dev_wb_selector->get_value().value());

            auto ret = m_wb.m_dev_wb_selector->set_value_str("Red");
            if (!ret)
            {
                return ret;
            }

            ret = m_wb.m_dev_wb_ratio->set_value(new_value_tmp);
            if (!ret)
            {
                return ret;
            }

            ret = m_wb.m_dev_wb_selector->set_value_str(tmp);
            return ret;
        }
        else if (prop_id == emulated::software_prop::WB_GREEN)
        {
            const std::string tmp = std::string(m_wb.m_dev_wb_selector->get_value().value());

            auto ret = m_wb.m_dev_wb_selector->set_value_str("Green");
            if (!ret)
            {
                return ret;
            }

            ret = m_wb.m_dev_wb_ratio->set_value(new_value_tmp);
            if (!ret)
            {
                return ret;
            }

            ret = m_wb.m_dev_wb_selector->set_value_str(tmp);
            return ret;
        }
        else if (prop_id == emulated::software_prop::WB_BLUE)
        {
            const std::string tmp = std::string(m_wb.m_dev_wb_selector->get_value().value());

            auto ret = m_wb.m_dev_wb_selector->set_value_str("Blue");
            if (!ret)
            {
                return ret;
            }

            ret = m_wb.m_dev_wb_ratio->set_value(new_value_tmp);
            if (!ret)
            {
                return ret;
            }

            ret = m_wb.m_dev_wb_selector->set_value_str(tmp);
            return ret;
        }
    }
    else
    {
        SPDLOG_ERROR("Device has no properties for whitebalance. Only software whitebalance is possible!");
    }
    return outcome::success();
}


void SoftwareProperties::set_locked(emulated::software_prop prop_id, bool is_locked)
{
    auto name = find_property_name(prop_id);

    auto prop_base = find_property(m_properties, name);

    if (!prop_base)
    {
        SPDLOG_ERROR("Unabled to find property with name {}", name);
        return;
    }

    PropertyFlags flags = prop_base->get_flags();
    if (is_locked)
    {
        flags |= PropertyFlags::Locked;
    }
    else
    {
        flags &= PropertyFlags::Locked;
    }

    switch (prop_base->get_type())
    {
        case TCAM_PROPERTY_TYPE_INTEGER:
        {
            dynamic_cast<emulated::SoftwarePropertyIntegerImpl*>(prop_base.get())->set_flags(flags);
            break;
        }
        case TCAM_PROPERTY_TYPE_DOUBLE:
        {
            dynamic_cast<emulated::SoftwarePropertyDoubleImpl*>(prop_base.get())->set_flags(flags);
            break;
        }
        case TCAM_PROPERTY_TYPE_ENUMERATION:
        {
            dynamic_cast<emulated::SoftwarePropertyEnumImpl*>(prop_base.get())->set_flags(flags);
            break;
        }
        default:
        {
            SPDLOG_ERROR("Not implemented");
            break;
        }
    }
}


void tcam::property::SoftwareProperties::enable_property(sp prop_id, bool device_flags)
{
    auto desc = find_property_desc(prop_id);

    if (!desc)
    {
        SPDLOG_INFO("No desc found {}", prop_id);
        return;
    }

    emulated::software_prop_desc d = *desc;
    d.device_flags = device_flags;

    switch (desc->type_)
    {
        case TCAM_PROPERTY_TYPE_INTEGER:
        {
            m_properties.push_back(
                std::make_shared<emulated::SoftwarePropertyIntegerImpl>(d, m_backend));
            break;
        }
        case TCAM_PROPERTY_TYPE_DOUBLE:
        {
            m_properties.push_back(
                std::make_shared<emulated::SoftwarePropertyDoubleImpl>(d, m_backend));
            break;
        }
        case TCAM_PROPERTY_TYPE_BOOLEAN:
        {
            m_properties.push_back(std::make_shared<emulated::SoftwarePropertyBoolImpl>(d, m_backend));
            break;
        }
        case TCAM_PROPERTY_TYPE_ENUMERATION:
        {

            m_properties.push_back(
                std::make_shared<emulated::SoftwarePropertyEnumImpl>(d, m_backend));
            auto res = set_int(prop_id, desc->default_value_);
            if (!res)
            {
                SPDLOG_ERROR("Error while setting enum value: {}", res.error().message());
            }

            break;
        }
        default:
        {
            SPDLOG_WARN("Not implemented. {}", desc->name_);
            break;
        }
    }
}


void tcam::property::SoftwareProperties::enable_property_double(sp prop_id,
                                                                std::shared_ptr<IPropertyFloat> prop)
{
    auto desc = find_property_desc(prop_id);

    if (!desc)
    {
        SPDLOG_INFO("No desc found {}", prop_id);
        return;
    }

    switch (desc->type_)
    {
        case TCAM_PROPERTY_TYPE_DOUBLE:
        {
            m_properties.push_back(
                std::make_shared<emulated::SoftwarePropertyDoubleImpl>(*desc, prop, m_backend));
            break;
        }
        default:
        {
            SPDLOG_WARN("Not implemented. {}", desc->name_);
            break;
        }
    }
}



void tcam::property::SoftwareProperties::enable_property_double(sp prop_id,
                                                                std::shared_ptr<IPropertyInteger> prop)
{
    auto desc = find_property_desc(prop_id);

    if (!desc)
    {
        SPDLOG_INFO("No desc found {}", prop_id);
        return;
    }

    switch (desc->type_)
    {
        case TCAM_PROPERTY_TYPE_DOUBLE:
        {
            m_properties.push_back(
                std::make_shared<emulated::SoftwarePropertyDoubleImpl>(*desc, prop, m_backend));
            break;
        }
        default:
        {
            SPDLOG_WARN("Not implemented. {}", desc->name_);
            break;
        }
    }
}


void tcam::property::SoftwareProperties::enable_property_int(sp prop_id,
                                                             std::shared_ptr<IPropertyInteger> prop)
{
    auto desc = find_property_desc(prop_id);

    if (!desc)
    {
        SPDLOG_INFO("No desc found {}", prop_id);
        return;
    }

    switch (desc->type_)
    {
        case TCAM_PROPERTY_TYPE_INTEGER:
        {
            m_properties.push_back(
                std::make_shared<emulated::SoftwarePropertyIntegerImpl>(*desc, prop, m_backend));
            break;
        }
        default:
        {
            SPDLOG_WARN("Not implemented. {}", desc->name_);
            break;
        }
    }
}

} // namespace tcam::property
