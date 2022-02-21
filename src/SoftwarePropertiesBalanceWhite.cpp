#include "SoftwareProperties.h"
#include "SoftwarePropertiesImpl.h"
#include "logging.h"

#include <tcamprop1.0_base/tcamprop_property_info_list.h>

using sp = tcam::property::emulated::software_prop;
using namespace tcam::property;

namespace
{

static constexpr auto balance_white_channel_range =
    tcam::property::emulated::prop_range_float_def { 0.0, 4.0, 0.01, 1.0 };

} // namespace

void SoftwareProperties::generate_balance_white_auto()
{
    bool has_wb_auto = find_property(m_properties, "BalanceWhiteAuto") != nullptr;
    if (has_wb_auto)
    {
        return;
    }

    auto wb_auto =
        make_prop_entry(sp::BalanceWhiteAuto,
                        &tcamprop1::prop_list::BalanceWhiteAuto,
                        emulated::to_range(tcamprop1::prop_list::enum_entries_off_auto_once),
                        1);
    m_auto_params.wb.auto_enabled = true;
    m_wb.m_is_software_auto_wb = true;

    auto base_r = tcam::property::find_property<IPropertyFloat>(m_properties, "BalanceWhiteRed");
    auto base_g = tcam::property::find_property<IPropertyFloat>(m_properties, "BalanceWhiteGreen");
    auto base_b = tcam::property::find_property<IPropertyFloat>(m_properties, "BalanceWhiteBlue");

    if (base_r && base_g && base_b)
    {
        SPDLOG_INFO("Adding BalanceWhiteAuto device based.");

        m_auto_params.wb.is_software_whitebalance = false;
        m_wb.m_wb_is_claimed = true;

        auto get_safe = [this](auto prop) -> double
        {
            auto res = prop->get_value();
            if (res)
                return res.value();
            return 1.0;
        };

        m_auto_params.wb.channels.r = get_safe(base_r);
        m_auto_params.wb.channels.g = get_safe(base_g);
        m_auto_params.wb.channels.b = get_safe(base_b);

        m_wb.m_dev_wb_r = base_r;
        m_wb.m_dev_wb_g = base_g;
        m_wb.m_dev_wb_b = base_b;

        auto wb_r = make_prop_entry(sp::BalanceWhiteRed,
                                    &tcamprop1::prop_list::BalanceWhiteRed,
                                    emulated::to_range(*m_wb.m_dev_wb_r));
        auto wb_g = make_prop_entry(sp::BalanceWhiteGreen,
                                    &tcamprop1::prop_list::BalanceWhiteGreen,
                                    emulated::to_range(*m_wb.m_dev_wb_g));
        auto wb_b = make_prop_entry(sp::BalanceWhiteBlue,
                                    &tcamprop1::prop_list::BalanceWhiteBlue,
                                    emulated::to_range(*m_wb.m_dev_wb_b));

        replace_entry(m_properties, wb_r);
        replace_entry(m_properties, wb_g);
        replace_entry(m_properties, wb_b);

        auto f = std::find(m_properties.begin(), m_properties.end(), wb_r);
        assert(f != m_properties.end());
        m_properties.insert(f, wb_auto); // insert auto in front of wb_r
    }
    else
    {
        SPDLOG_INFO("Adding BalanceWhiteAuto software based.");

        m_auto_params.wb.is_software_whitebalance = true;
        m_wb.m_wb_is_claimed = false;

        m_properties.push_back(wb_auto);

        add_prop_entry(m_properties,
                       sp::ClaimBalanceWhiteSoftware,
                       &tcamprop1::prop_list::ClaimBalanceWhiteSoftware,
                       false);
        add_prop_entry(m_properties,
                       sp::BalanceWhiteRed,
                       &tcamprop1::prop_list::BalanceWhiteRed,
                       balance_white_channel_range);
        add_prop_entry(m_properties,
                       sp::BalanceWhiteGreen,
                       &tcamprop1::prop_list::BalanceWhiteGreen,
                       balance_white_channel_range);
        add_prop_entry(m_properties,
                       sp::BalanceWhiteBlue,
                       &tcamprop1::prop_list::BalanceWhiteBlue,
                       balance_white_channel_range);
    }
}

outcome::result<double> SoftwareProperties::get_whitebalance_channel(
    emulated::software_prop prop_id)
{
    if (prop_id == emulated::software_prop::BalanceWhiteRed)
    {
        if (m_wb.m_dev_wb_r)
            return m_wb.m_dev_wb_r->get_value();
        else
            return m_auto_params.wb.channels.r;
    }
    else if (prop_id == emulated::software_prop::BalanceWhiteGreen)
    {
        if (m_wb.m_dev_wb_g)
            return m_wb.m_dev_wb_g->get_value();
        else
            return m_auto_params.wb.channels.g;
    }
    else if (prop_id == emulated::software_prop::BalanceWhiteBlue)
    {
        if (m_wb.m_dev_wb_b)
            return m_wb.m_dev_wb_b->get_value();
        else
            return m_auto_params.wb.channels.b;
    }

    SPDLOG_ERROR("Not a whitebalance property");
    return tcam::status::PropertyNotImplemented;
}

outcome::result<void> SoftwareProperties::set_whitebalance_channel(emulated::software_prop prop_id,
                                                                   double new_value_tmp)
{
    if (prop_id == emulated::software_prop::BalanceWhiteRed)
    {
        m_auto_params.wb.channels.r = new_value_tmp;
        if (m_wb.m_dev_wb_r)
            return m_wb.m_dev_wb_r->set_value(new_value_tmp);
        return outcome::success();
    }
    else if (prop_id == emulated::software_prop::BalanceWhiteGreen)
    {
        m_auto_params.wb.channels.g = new_value_tmp;
        if (m_wb.m_dev_wb_g)
            return m_wb.m_dev_wb_g->set_value(new_value_tmp);
        return outcome::success();
    }
    else if (prop_id == emulated::software_prop::BalanceWhiteBlue)
    {
        m_auto_params.wb.channels.b = new_value_tmp;
        if (m_wb.m_dev_wb_b)
            return m_wb.m_dev_wb_b->set_value(new_value_tmp);
        return outcome::success();
    }

    SPDLOG_ERROR("Not a whitebalance property");
    return tcam::status::PropertyNotImplemented;
}