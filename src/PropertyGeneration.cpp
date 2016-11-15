/*
 * Copyright 2014 The Imaging Source Europe GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
    if (find_property(props, TCAM_PROPERTY_OFFSET_AUTO) == nullptr &&
        find_property(props, TCAM_PROPERTY_OFFSET_X) != nullptr &&
        find_property(props, TCAM_PROPERTY_OFFSET_Y) != nullptr)
    {
        tcam_device_property cp = create_empty_property(TCAM_PROPERTY_OFFSET_AUTO);
        cp.value.b.default_value = false;
        cp.value.b.value = cp.value.b.default_value;
        cp.flags = set_bit(cp.flags, TCAM_PROPERTY_FLAG_EXTERNAL);

        auto property_auto_offset = std::make_shared<PropertyBoolean>(impl, cp, Property::BOOLEAN);
        tcam_log(TCAM_LOG_DEBUG, "Adding 'Offset Auto Center' to property list");

        new_properties.push_back(property_auto_offset);
    }

    return new_properties;
}


bool tcam::handle_auto_center (const Property& new_property,
                               std::vector<std::shared_ptr<Property>>& props,
                               const tcam_image_size& sensor,
                               const tcam_image_size& current_format)
{
    if (new_property.get_type() != TCAM_PROPERTY_TYPE_BOOLEAN)
    {
        return false;
    }

    auto p = static_cast<const PropertyBoolean&>(new_property);

    if (p.get_value())
    {
        tcam_image_size values = calculate_auto_center(sensor, current_format);

        auto prop_off_x = find_property(props, TCAM_PROPERTY_OFFSET_X);
        auto prop_off_y = find_property(props, TCAM_PROPERTY_OFFSET_Y);

        std::static_pointer_cast<PropertyInteger>(prop_off_x)->set_value(values.width);
        std::static_pointer_cast<PropertyInteger>(prop_off_y)->set_value(values.height);
    }
    else
    {
        auto prop_off_x = find_property(props, TCAM_PROPERTY_OFFSET_X);
        auto prop_off_y = find_property(props, TCAM_PROPERTY_OFFSET_Y);

        std::static_pointer_cast<PropertyInteger>(prop_off_x)->set_value(0);
        std::static_pointer_cast<PropertyInteger>(prop_off_y)->set_value(0);
    }

    return true;
}
