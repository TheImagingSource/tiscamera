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

#ifndef TCAM_PROPERTYGENERATION_H
#define TCAM_PROPERTYGENERATION_H

#include "Properties.h"

#include <memory>

#include "compiler_defines.h"

VISIBILITY_INTERNAL

namespace tcam
{


/**
 * @param props - vector of existing properties
 * @param impl - PropertyImpl that shall be used for new properties
 * @return vector containing the simulated properties; can be empty
 */
std::vector<std::shared_ptr<Property>> generate_simulated_properties (std::vector<std::shared_ptr<Property>> props,
                                                                      std::shared_ptr<PropertyImpl> impl);


/**
 * @param new_property
 * @param props - vector of device properties
 * @param sensor - maximum image size the sensor allows
 * @param current_format - current image size
 * @return true on success
 */
bool handle_auto_center (const Property& new_property,
                         std::vector<std::shared_ptr<Property>>& props,
                         const tcam_image_size& sensor,
                         const tcam_image_size& current_format);

} /* namespace tcam */

VISIBILITY_POP

#endif /* TCAM_PROPERTYGENERATION_H */
