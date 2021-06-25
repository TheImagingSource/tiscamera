

#include "SoftwareProperties.h"

#include "SoftwarePropertiesImpl.h"
#include "algorithms/tcam-algorithm.h"
#include "logging.h"

#include <algorithm>
#include <chrono>
#include <map>

using namespace tcam::property;

using sp = tcam::property::emulated::software_prop;

namespace
{

static emulated::software_prop_desc prop_list[] = {
    { sp::ExposureTime, "ExposureTime", TCAM_PROPERTY_TYPE_DOUBLE },
    { sp::ExposureAuto, "ExposureAuto", { { { 0, "Off" }, { 1, "Continuous" } } }, 1 },
    { sp::ExposureAutoLowerLimit, "ExposureAutoLowerLimit", TCAM_PROPERTY_TYPE_DOUBLE },
    { sp::ExposureAutoUpperLimit, "ExposureAutoUpperLimit", TCAM_PROPERTY_TYPE_DOUBLE },
    { sp::ExposureAutoUpperLimitAuto, "ExposureAutoUpperLimitAuto", { { { 0, "Off" }, { 1, "On" } } }, 1 },
    { sp::ExposureAutoReference, "ExposureAutoReference", tcam_value_int { 0, 255, 1, 128, 128 } },

    { sp::Gain, "Gain", TCAM_PROPERTY_TYPE_DOUBLE },
    { sp::GainAuto, "GainAuto", { { { 0, "Off" }, { 1, "Continuous" } } }, 1 },
    { sp::GainAutoLowerLimit, "GainAutoLowerLimit", TCAM_PROPERTY_TYPE_DOUBLE },
    { sp::GainAutoUpperLimit, "GainAutoUpperLimit", TCAM_PROPERTY_TYPE_DOUBLE },

    { sp::Iris, "Iris", TCAM_PROPERTY_TYPE_INTEGER },
    { sp::IrisAuto, "IrisAuto", { { { 0, "Off" }, { 1, "Continuous" } } }, 1 },

    { sp::Focus, "Focus", TCAM_PROPERTY_TYPE_INTEGER },
    { sp::FocusAuto, "FocusAuto", { { { 0, "Off" }, { 1, "Once" } } }, 0 },
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
    const std::vector<std::shared_ptr<tcam::property::IPropertyBase>>& dev_properties, bool has_bayer)
    : m_device_properties(dev_properties)
{
    m_backend = std::make_shared<emulated::SoftwarePropertyBackend>(this);
    p_state = auto_alg::make_state_ptr();
    generate_public_properties(has_bayer);
}


static uint64_t time_now_in_us() noexcept
{
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}


static long frame_counter;

void SoftwareProperties::auto_pass(const img::img_descriptor& image)
{
    auto_alg::auto_pass_params tmp_params;
    {
        std::scoped_lock lock(m_property_mtx);

        // if( params_.whitebalance_enable )
        // {
        //     auto_params_.wb.channels = user_wb_;
        // } else {
        //     auto_params_.wb.channels = {};
        // }


        tmp_params = m_auto_params;

        m_auto_params.focus_onepush_params.is_run_cmd = false;
        tmp_params.exposure.max = m_exposure_auto_upper_limit;
    }

    // TODO: get from ImageBuffer statistics
    tmp_params.frame_number = frame_counter;
    frame_counter++;
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
}


void tcam::property::SoftwareProperties::generate_public_properties(bool /*has_bayer*/)
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

    auto iter_wb = find_property(m_device_properties, "BalanceWhiteAuto");

    if (!iter_wb)
    {
        //generate_whitebalance();
    }


    auto contains = [&](const std::shared_ptr<IPropertyBase>& elem) {
        return std::any_of(
            m_properties.begin(), m_properties.end(), [&](const std::shared_ptr<IPropertyBase>& x) {
                return x->get_name() == elem->get_name();
            });
    };

    for (auto& p : m_device_properties)
    {
        if (!contains(p))
        {
            //SPDLOG_INFO("Adding {}", p->get_name());
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

            if (m_exposure_upper_auto)
            {
                m_exposure_auto_upper_limit = 1'000'000 / m_format.get_framerate();
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
        default:
        {
            SPDLOG_WARN("Not implemented");
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
        default:
        {
            SPDLOG_WARN("not implemented {} {}", find_property_name(prop_id), prop_id);

            return tcam::status::NotImplemented;
        }
    }
}

void SoftwareProperties::update_to_new_format(const tcam::VideoFormat& new_format)
{
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
    };

    m_dev_exposure = std::static_pointer_cast<tcam::property::IPropertyFloat>(exp_base);

    m_auto_params.exposure.granularity = m_dev_exposure->get_step();
    ;
    m_auto_params.exposure.min = m_dev_exposure->get_min();
    m_auto_params.exposure.max = m_dev_exposure->get_max();

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
    m_auto_params.gain.min = m_dev_gain->get_min();
    m_auto_params.gain.max = m_dev_gain->get_max();
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

    m_auto_params.iris.min = m_dev_iris->get_min();
    m_auto_params.iris.max = m_dev_iris->get_max();

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
    m_auto_params.focus_onepush_params.run_cmd_params.focus_range_min = m_dev_focus->get_min();
    m_auto_params.focus_onepush_params.run_cmd_params.focus_range_max = m_dev_focus->get_max();

    enable_property(sp::FocusAuto);
    enable_property_int(sp::Focus, m_dev_focus);
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

    // SPDLOG_INFO("{} now has flags: {:x}", prop_base->get_name(), flags);

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


void tcam::property::SoftwareProperties::enable_property(sp prop_id)
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
                std::make_shared<emulated::SoftwarePropertyIntegerImpl>(desc, m_backend));
            break;
        }
        case TCAM_PROPERTY_TYPE_DOUBLE:
        {
            m_properties.push_back(
                std::make_shared<emulated::SoftwarePropertyDoubleImpl>(desc, m_backend));
            break;
        }
        case TCAM_PROPERTY_TYPE_ENUMERATION:
        {

            m_properties.push_back(
                std::make_shared<emulated::SoftwarePropertyEnumImpl>(desc, m_backend));
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
            m_properties.push_back(std::make_shared<emulated::SoftwarePropertyDoubleImpl>(desc, prop, m_backend));
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
            m_properties.push_back(std::make_shared<emulated::SoftwarePropertyIntegerImpl>(desc, prop, m_backend));
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
