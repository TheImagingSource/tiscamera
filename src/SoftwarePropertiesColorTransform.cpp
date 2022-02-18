#include "SoftwareProperties.h"
#include "SoftwarePropertiesBase.h"
#include "SoftwarePropertiesImpl.h"
#include "logging.h"

#include <tcamprop1.0_base/tcamprop_property_info_list.h>

using namespace tcam;

using sp = tcam::property::emulated::software_prop;


static std::string_view to_transform_name(tcam::property::emulated::software_prop prop)
{
    if (prop == sp::ColorTransformRedToRed)
    {
        return "Gain00";
    }
    if (prop == sp::ColorTransformGreenToRed)
    {
        return "Gain01";
    }
    if (prop == sp::ColorTransformBlueToRed)
    {
        return "Gain02";
    }
    if (prop == sp::ColorTransformRedToGreen)
    {
        return "Gain10";
    }
    if (prop == sp::ColorTransformGreenToGreen)
    {
        return "Gain11";
    }
    if (prop == sp::ColorTransformBlueToGreen)
    {
        return "Gain12";
    }
    if (prop == sp::ColorTransformRedToBlue)
    {
        return "Gain20";
    }
    if (prop == sp::ColorTransformGreenToBlue)
    {
        return "Gain21";
    }
    if (prop == sp::ColorTransformBlueToBlue)
    {
        return "Gain22";
    }
    return {};
}


outcome::result<double> tcam::property::SoftwareProperties::get_device_color_transform(
    emulated::software_prop prop_id)
{
    auto channel = to_transform_name(prop_id);

    auto res = m_dev_color_transform_value_selector->set_value(channel);

    if (!res)
    {
        return res.as_failure();
    }

    return m_dev_color_transform_value->get_value();
}


outcome::result<void> tcam::property::SoftwareProperties::set_device_color_transform(
    emulated::software_prop prop_id,
    double new_value_tmp)
{
    auto channel = to_transform_name(prop_id);

    auto res = m_dev_color_transform_value_selector->set_value(channel);

    if (!res)
    {
        return res.as_failure();
    }

    return m_dev_color_transform_value->set_value(new_value_tmp);
}


void tcam::property::SoftwareProperties::generate_color_transformation()
{
    auto enable =
        tcam::property::find_property<IPropertyBool>(m_properties, "ColorTransformationEnable");
    auto value =
        tcam::property::find_property<IPropertyFloat>(m_properties, "ColorTransformationValue");
    auto value_selector = tcam::property::find_property<IPropertyEnum>(
        m_properties, "ColorTransformationValueSelector");

    if (!enable || !value || !value_selector)
    {
        return;
    }

#if 0 // currently not necessary
    auto color_transform_type =
        tcam::property::find_property<IPropertyEnum>(m_device_properties, "ColorTransformationSelector");
    if (color_transform_type) {
        if (auto res = color_transform_type->get_value(); res.has_value()) {
            if (res.value() != "RGBtoRGB") {
                return;
            }
        }
    }
#endif

    m_dev_color_transform_enable = enable;
    m_dev_color_transform_value = value;
    m_dev_color_transform_value_selector = value_selector;

    auto range = emulated::to_range(*m_dev_color_transform_value);

    prop_ptr_vec new_list;

    auto new_enable_item = make_prop_entry(
        sp::ColorTransformEnable, &tcamprop1::prop_list::ColorTransformationEnable, false);

    add_prop_entry(new_list,
                   sp::ColorTransformRedToRed,
                   &tcamprop1::prop_list::ColorTransformation_Value_Gain00,
                   range);

    add_prop_entry(new_list,
                   sp::ColorTransformBlueToRed,
                   &tcamprop1::prop_list::ColorTransformation_Value_Gain01,
                   range);

    add_prop_entry(new_list,
                   sp::ColorTransformGreenToRed,
                   &tcamprop1::prop_list::ColorTransformation_Value_Gain02,
                   range);

    add_prop_entry(new_list,
                   sp::ColorTransformRedToGreen,
                   &tcamprop1::prop_list::ColorTransformation_Value_Gain10,
                   range);

    add_prop_entry(new_list,
                   sp::ColorTransformGreenToGreen,
                   &tcamprop1::prop_list::ColorTransformation_Value_Gain11,
                   range);

    add_prop_entry(new_list,
                   sp::ColorTransformBlueToGreen,
                   &tcamprop1::prop_list::ColorTransformation_Value_Gain12,
                   range);

    add_prop_entry(new_list,
                   sp::ColorTransformRedToBlue,
                   &tcamprop1::prop_list::ColorTransformation_Value_Gain20,
                   range);

    add_prop_entry(new_list,
                   sp::ColorTransformGreenToBlue,
                   &tcamprop1::prop_list::ColorTransformation_Value_Gain21,
                   range);

    add_prop_entry(new_list,
                   sp::ColorTransformBlueToBlue,
                   &tcamprop1::prop_list::ColorTransformation_Value_Gain22,
                   range);

    remove_entry(m_properties, "ColorTransformationValue");
    remove_entry(m_properties, "ColorTransformationValueSelector");
    replace_entry(m_properties, new_enable_item);
    add_prop_entry(m_properties, "ColorTransformationEnable", new_list);
}
