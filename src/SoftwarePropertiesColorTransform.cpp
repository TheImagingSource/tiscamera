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


outcome::result<double> tcam::property::SoftwareProperties::get_device_color_transform (emulated::software_prop prop_id)
{
    auto channel = to_transform_name(prop_id);

    auto res = m_dev_color_transform_value_selector->set_value_str(channel);

    if (!res)
    {
        return res.as_failure();
    }

    return m_dev_color_transform_value->get_value();
}


outcome::result<void> tcam::property::SoftwareProperties::set_device_color_transform (emulated::software_prop prop_id,
                                                                                      double new_value_tmp)
{
    auto channel = to_transform_name(prop_id);

    auto res = m_dev_color_transform_value_selector->set_value_str(channel);

    if (!res)
    {
        return res.as_failure();
    }

    return m_dev_color_transform_value->set_value(new_value_tmp);
}


void tcam::property::SoftwareProperties::generate_color_transformation()
{
    m_dev_color_transform_enable = tcam::property::find_property<IPropertyBool>(m_device_properties, "ColorTransformationEnable");
    m_dev_color_transform_value = tcam::property::find_property<IPropertyFloat>(m_device_properties, "ColorTransformationValue");
    m_dev_color_transform_value_selector = tcam::property::find_property<IPropertyEnum>(m_device_properties, "ColorTransformationValueSelector");

    add_prop_entry(sp::ColorTransformEnable,
                   &tcamprop1::prop_list::ColorTransformationEnable,
                   false);

    add_prop_entry(sp::ColorTransformRedToRed,
                   &tcamprop1::prop_list::ColorTransformation_Value_Gain00,
                   emulated::to_range(*m_dev_color_transform_value));

    add_prop_entry(sp::ColorTransformBlueToRed,
                   &tcamprop1::prop_list::ColorTransformation_Value_Gain01,
                   emulated::to_range(*m_dev_color_transform_value));

    add_prop_entry(sp::ColorTransformGreenToRed,
                   &tcamprop1::prop_list::ColorTransformation_Value_Gain02,
                   emulated::to_range(*m_dev_color_transform_value));

    add_prop_entry(sp::ColorTransformRedToGreen,
                   &tcamprop1::prop_list::ColorTransformation_Value_Gain10,
                   emulated::to_range(*m_dev_color_transform_value));

    add_prop_entry(sp::ColorTransformGreenToGreen,
                   &tcamprop1::prop_list::ColorTransformation_Value_Gain11,
                   emulated::to_range(*m_dev_color_transform_value));

    add_prop_entry(sp::ColorTransformBlueToGreen,
                   &tcamprop1::prop_list::ColorTransformation_Value_Gain12,
                   emulated::to_range(*m_dev_color_transform_value));

    add_prop_entry(sp::ColorTransformRedToBlue,
                   &tcamprop1::prop_list::ColorTransformation_Value_Gain20,
                   emulated::to_range(*m_dev_color_transform_value));

    add_prop_entry(sp::ColorTransformGreenToBlue,
                   &tcamprop1::prop_list::ColorTransformation_Value_Gain21,
                   emulated::to_range(*m_dev_color_transform_value));

    add_prop_entry(sp::ColorTransformBlueToBlue,
                   &tcamprop1::prop_list::ColorTransformation_Value_Gain22,
                   emulated::to_range(*m_dev_color_transform_value));
}
