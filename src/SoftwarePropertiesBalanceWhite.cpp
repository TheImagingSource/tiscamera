#include "SoftwareProperties.h"
#include "SoftwarePropertiesImpl.h"
#include "logging.h"
#include <tcamprop1.0_base/tcamprop_property_info_list.h>

using sp = tcam::property::emulated::software_prop;

namespace
{
namespace prop_lst = tcamprop1::prop_list;

static constexpr auto balance_white_channel_range =
    tcam::property::emulated::prop_range_float_def { 0.0, 4.0, 0.01, 1.0 };

} // namespace

namespace tcam::property
{

void SoftwareProperties::generate_balance_white_channels()
{
    auto selector =
        tcam::property::find_property<IPropertyEnum>(m_device_properties, "BalanceRatioSelector");
    auto ratio = tcam::property::find_property<IPropertyFloat>(m_device_properties, "BalanceRatio");

    if (!ratio || !selector)
    {
        SPDLOG_ERROR("Unable to correctly identify balance white properties. Balance White "
                     "will not work correctly.");
        return;
    }

    SPDLOG_INFO("Adding BalanceWhiteRGB channels.");

    m_wb.m_dev_wb_selector = selector;
    m_wb.m_dev_wb_ratio = ratio;

    add_prop_entry(sp::BalanceWhiteRed, &prop_lst::BalanceWhiteRed, balance_white_channel_range);
    add_prop_entry(
        sp::BalanceWhiteGreen, &prop_lst::BalanceWhiteGreen, balance_white_channel_range);
    add_prop_entry(sp::BalanceWhiteBlue, &prop_lst::BalanceWhiteBlue, balance_white_channel_range);
}

void SoftwareProperties::generate_balance_white_auto()
{
    auto get_safe = [this](sp prop) -> double
    {
        auto res = get_double(prop);
        if (res)
        {
            return res.value();
        }
        return 1.0;
    };

    auto base_r = tcam::property::find_property<IPropertyFloat>(m_device_properties, "BalanceWhiteRed");
    auto base_g = tcam::property::find_property<IPropertyFloat>(m_device_properties, "BalanceWhiteGreen");
    auto base_b = tcam::property::find_property<IPropertyFloat>(m_device_properties, "BalanceWhiteBlue");
    if (m_wb.m_dev_wb_selector) // we already generated RGB channels for BalanceWhiteRed/BalanceWhiteGreen/BalanceWhiteBlue via selector
    {
        m_auto_params.wb.is_software_whitebalance = false;
        m_wb_is_claimed = true;

        m_auto_params.wb.channels.r = get_safe(sp::BalanceWhiteRed);
        m_auto_params.wb.channels.g = get_safe(sp::BalanceWhiteGreen);
        m_auto_params.wb.channels.b = get_safe(sp::BalanceWhiteBlue);

        SPDLOG_INFO("Adding BalanceWhiteAuto device selector based.");
    }
    else if (base_r && base_g && base_b)
    {
        m_wb.m_dev_wb_r = base_r;
        m_wb.m_dev_wb_g = base_g;
        m_wb.m_dev_wb_b = base_b;

        add_prop_entry(
            sp::BalanceWhiteRed, &prop_lst::BalanceWhiteRed, emulated::to_range(*m_wb.m_dev_wb_r));
        add_prop_entry(sp::BalanceWhiteGreen,
                       &prop_lst::BalanceWhiteGreen,
                       emulated::to_range(*m_wb.m_dev_wb_g));
        add_prop_entry(sp::BalanceWhiteBlue,
                       &prop_lst::BalanceWhiteBlue,
                       emulated::to_range(*m_wb.m_dev_wb_b));

        m_auto_params.wb.is_software_whitebalance = false;
        m_wb_is_claimed = true;

        m_auto_params.wb.channels.r = get_safe(sp::BalanceWhiteRed);
        m_auto_params.wb.channels.g = get_safe(sp::BalanceWhiteGreen);
        m_auto_params.wb.channels.b = get_safe(sp::BalanceWhiteBlue);

        SPDLOG_INFO("Adding BalanceWhiteAuto device based.");
    }
    else
    {
        m_wb.m_emulated_wb = true;

        add_prop_entry(sp::ClaimBalanceWhiteSoftware, &prop_lst::ClaimBalanceWhiteSoftware, false);

        add_prop_entry(
            sp::BalanceWhiteRed, &prop_lst::BalanceWhiteRed, balance_white_channel_range);
        add_prop_entry(
            sp::BalanceWhiteGreen, &prop_lst::BalanceWhiteGreen, balance_white_channel_range);
        add_prop_entry(
            sp::BalanceWhiteBlue, &prop_lst::BalanceWhiteBlue, balance_white_channel_range);

        m_auto_params.wb.is_software_whitebalance = true;
        m_wb_is_claimed = false;

        SPDLOG_INFO("Adding BalanceWhiteAuto software based.");
    }

    add_prop_entry(sp::BalanceWhiteAuto,
                   &prop_lst::BalanceWhiteAuto,
                   emulated::to_range(prop_lst::enum_entries_off_auto_once),
                   1);
    m_auto_params.wb.auto_enabled = true;
    m_is_software_auto_wb = true;
}

static std::string_view to_channel_name(emulated::software_prop prop)
{
    if (prop == emulated::software_prop::BalanceWhiteRed)
        return "Red";
    if (prop == emulated::software_prop::BalanceWhiteGreen)
        return "Green";
    if (prop == emulated::software_prop::BalanceWhiteBlue)
        return "Blue";
    return {};
}

outcome::result<double> SoftwareProperties::get_device_wb(emulated::software_prop prop_id)
{
    if (prop_id != emulated::software_prop::BalanceWhiteRed
        && prop_id != emulated::software_prop::BalanceWhiteGreen
        && prop_id != emulated::software_prop::BalanceWhiteBlue)
    {
        SPDLOG_ERROR("Not a whitebalance property");
        return tcam::status::NotSupported;
    }

    if (m_wb.get_type() == wb_type::DevChannel)
    {
        outcome::result<double> dev_val = [&]() -> outcome::result<double>
        {
            if (prop_id == emulated::software_prop::BalanceWhiteRed)
            {
                return m_wb.m_dev_wb_r->get_value();
            }
            else if (prop_id == emulated::software_prop::BalanceWhiteGreen)
            {
                return m_wb.m_dev_wb_g->get_value();
            }
            else if (prop_id == emulated::software_prop::BalanceWhiteBlue)
            {
                return m_wb.m_dev_wb_b->get_value();
            }
            return tcam::status::NotSupported;
        }();

        return dev_val;
    }
    else if (m_wb.get_type() == wb_type::DevSelector)
    {
        auto channel = to_channel_name(prop_id);

        OUTCOME_TRY(std::string_view current_selector_setting, m_wb.m_dev_wb_selector->get_value());

        // selector already set?
        if (current_selector_setting == channel)
        {
            return m_wb.m_dev_wb_ratio->get_value();
        }

        if (auto ret = m_wb.m_dev_wb_selector->set_value_str(channel); !ret)
        {
            SPDLOG_WARN("Setting BalanceWhiteSelector to {} failed with: {}",
                        channel,
                        ret.error().message());
            return ret.error();
        }

        auto actual_rval = m_wb.m_dev_wb_ratio->get_value();

        if (auto ret = m_wb.m_dev_wb_selector->set_value_str(current_selector_setting); !ret)
        {
            SPDLOG_WARN("Resetting BalanceWhiteSelector failed with: {}", ret.error().message());
        }
        return actual_rval;
    }
    else if (m_wb.get_type() == wb_type::Emulation)
    {
        if (prop_id == emulated::software_prop::BalanceWhiteRed)
        {
            return m_auto_params.wb.channels.r;
        }
        else if (prop_id == emulated::software_prop::BalanceWhiteGreen)
        {
            return m_auto_params.wb.channels.g;
        }
        else
        {
            return m_auto_params.wb.channels.b;
        }
    }

    return tcam::status::NotSupported;
}
outcome::result<void> SoftwareProperties::set_device_wb(emulated::software_prop prop_id,
                                                        double new_value_tmp)
{
    if (m_wb.get_type() == wb_type::DevChannel)
    {
        if (prop_id == emulated::software_prop::BalanceWhiteRed)
        {
            return m_wb.m_dev_wb_r->set_value(new_value_tmp);
        }
        else if (prop_id == emulated::software_prop::BalanceWhiteGreen)
        {
            return m_wb.m_dev_wb_g->set_value(new_value_tmp);
        }
        else if (prop_id == emulated::software_prop::BalanceWhiteBlue)
        {
            return m_wb.m_dev_wb_b->set_value(new_value_tmp);
        }
    }
    else if (m_wb.get_type() == wb_type::DevSelector)
    {
        auto channel = to_channel_name(prop_id);
        auto value = new_value_tmp;

        OUTCOME_TRY(std::string_view current_selector_setting, m_wb.m_dev_wb_selector->get_value());

        // selector already set?
        if (current_selector_setting == channel)
        {
            return m_wb.m_dev_wb_ratio->set_value(value);
        }

        if (auto ret = m_wb.m_dev_wb_selector->set_value_str(channel); !ret)
        {
            SPDLOG_WARN("Setting BalanceWhiteSelector to {} failed with: {}",
                        channel,
                        ret.error().message());
            return ret;
        }

        auto actual_rval = m_wb.m_dev_wb_ratio->set_value(value);

        if (auto ret = m_wb.m_dev_wb_selector->set_value_str(current_selector_setting); !ret)
        {
            SPDLOG_WARN("Resetting BalanceWhiteSelector failed with: {}", ret.error().message());
        }
        return actual_rval;
    }

    SPDLOG_ERROR(
        "Device has no properties for whitebalance. Only software whitebalance are available!");

    return tcam::status::NotSupported;
}


} // namespace tcam::property