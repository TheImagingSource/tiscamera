
#include "PropertyGeneration.h"

#include "base_types.h"
#include "utils.h"
#include "logging.h"

#include "standard_properties.h"

#include <linux/videodev2.h>
#include <cstring>
#include <algorithm>

using namespace tcam;

std::vector<std::shared_ptr<Property>> tcam::generate_simulated_properties (std::vector<std::shared_ptr<Property>> props,
                                                                                   std::shared_ptr<PropertyImpl> impl)
{
    std::vector<std::shared_ptr<Property>> new_properties;

    // requirements for auto center
    if (find_property(props, PROPERTY_OFFSET_AUTO) == nullptr &&
        find_property(props, PROPERTY_OFFSET_X) != nullptr &&
        find_property(props, PROPERTY_OFFSET_Y) != nullptr)
    {
        camera_property cp = {};
        cp.id = PROPERTY_OFFSET_AUTO;
        strcpy(cp.name, "Offset Auto Center");
        cp.type = PROPERTY_TYPE_BOOLEAN;
        cp.value.b.default_value = false;
        cp.value.b.value = cp.value.b.default_value;
        cp.flags = set_bit(cp.flags, PROPERTY_FLAG_EXTERNAL);

        auto property_auto_offset = std::make_shared<PropertyBoolean>(impl, cp, Property::BOOLEAN);

        // property_description pd = { EMULATED_PROPERTY, property_auto_offset};

        tcam_log(TCAM_LOG_DEBUG, "Adding 'Offset Auto Center' to property list");

        new_properties.push_back(property_auto_offset);
    }

    return new_properties;
}


bool tcam::handle_auto_center (const Property& new_property,
                                      std::vector<std::shared_ptr<Property>>& props,
                                      const IMG_SIZE& sensor,
                                      const IMG_SIZE& current_format)
{
    if (new_property.getType() != PROPERTY_TYPE_BOOLEAN)
    {
        return false;
    }

    auto p = static_cast<const PropertyBoolean&>(new_property);

    if (p.getValue())
    {
        IMG_SIZE values = calculate_auto_center(sensor, current_format);

        auto prop_off_x = find_property(props, "Offset X");
        auto prop_off_y = find_property(props, "Offset Y");

        std::static_pointer_cast<PropertyInteger>(prop_off_x)->setValue(values.width);
        std::static_pointer_cast<PropertyInteger>(prop_off_y)->setValue(values.height);

        // TODO: set properties read only
    }
    else
    {
        // TODO: remove read only

        auto prop_off_x = find_property(props, "Offset X");
        auto prop_off_y = find_property(props, "Offset Y");

        std::static_pointer_cast<PropertyInteger>(prop_off_x)->setValue(0);
        std::static_pointer_cast<PropertyInteger>(prop_off_y)->setValue(0);
    }
}
