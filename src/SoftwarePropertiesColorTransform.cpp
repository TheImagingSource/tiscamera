#include "SoftwareProperties.h"
#include "SoftwarePropertiesBase.h"
#include "SoftwarePropertiesImpl.h"
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
static std::string_view to_transform_name(emulated::software_prop prop)
{
    if (prop == emulated::software_prop::ColorTransformRedToRed)
    {
        return "Gain00";
    }
    if (prop == emulated::software_prop::ColorTransformGreenToRed)
    {
        return "Gain01";
    }
    if (prop == emulated::software_prop::ColorTransformBlueToRed)
    {
        return "Gain02";
    }
    if (prop == emulated::software_prop::ColorTransformRedToGreen)
    {
        return "Gain10";
    }
    if (prop == emulated::software_prop::ColorTransformGreenToGreen)
    {
        return "Gain11";
    }
    if (prop == emulated::software_prop::ColorTransformBlueToGreen)
    {
        return "Gain12";
    }
    if (prop == emulated::software_prop::ColorTransformRedToBlue)
    {
        return "Gain20";
    }
    if (prop == emulated::software_prop::ColorTransformGreenToBlue)
    {
        return "Gain21";
    }
    if (prop == emulated::software_prop::ColorTransformBlueToBlue)
    {
        return "Gain22";
    }
    return {};
}


outcome::result<double> SoftwareProperties::get_device_color_transform (emulated::software_prop prop_id)
{
    auto channel = to_transform_name(prop_id);

    auto res = m_dev_color_transform_value_selector->set_value_str(channel);

    if (!res)
    {
        return res.as_failure();
    }

    return m_dev_color_transform_value->get_value();
}


outcome::result<void> SoftwareProperties::set_device_color_transform (emulated::software_prop prop_id,
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


void SoftwareProperties::generate_color_transformation()
{
    m_dev_color_transform_enable = tcam::property::find_property<IPropertyBool>(m_device_properties, "ColorTransformationEnable");
    m_dev_color_transform_value = tcam::property::find_property<IPropertyFloat>(m_device_properties, "ColorTransformationValue");
    m_dev_color_transform_value_selector = tcam::property::find_property<IPropertyEnum>(m_device_properties, "ColorTransformationValueSelector");

    add_prop_entry(sp::ColorTransformEnable,
                   &prop_lst::ColorTransformationEnable,
                   false);

    add_prop_entry(sp::ColorTransformRedToRed,
                   &prop_lst::ColorTransformation_Value_Gain00,
                   emulated::to_range(*m_dev_color_transform_value));

    add_prop_entry(sp::ColorTransformBlueToRed,
                   &prop_lst::ColorTransformation_Value_Gain01,
                   emulated::to_range(*m_dev_color_transform_value));

    add_prop_entry(sp::ColorTransformGreenToRed,
                   &prop_lst::ColorTransformation_Value_Gain02,
                   emulated::to_range(*m_dev_color_transform_value));

    add_prop_entry(sp::ColorTransformRedToGreen,
                   &prop_lst::ColorTransformation_Value_Gain10,
                   emulated::to_range(*m_dev_color_transform_value));

    add_prop_entry(sp::ColorTransformGreenToGreen,
                   &prop_lst::ColorTransformation_Value_Gain11,
                   emulated::to_range(*m_dev_color_transform_value));

    add_prop_entry(sp::ColorTransformBlueToGreen,
                   &prop_lst::ColorTransformation_Value_Gain12,
                   emulated::to_range(*m_dev_color_transform_value));

    add_prop_entry(sp::ColorTransformRedToBlue,
                   &prop_lst::ColorTransformation_Value_Gain20,
                   emulated::to_range(*m_dev_color_transform_value));

    add_prop_entry(sp::ColorTransformGreenToBlue,
                   &prop_lst::ColorTransformation_Value_Gain21,
                   emulated::to_range(*m_dev_color_transform_value));

    add_prop_entry(sp::ColorTransformBlueToBlue,
                   &prop_lst::ColorTransformation_Value_Gain22,
                   emulated::to_range(*m_dev_color_transform_value));
}

} // namepace tcam::property