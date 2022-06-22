/*
 * Copyright 2022 The Imaging Source Europe GmbH
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

#include "virtcam_device.h"

#include "virtcam_properties_impl.h"

#include <memory>

void tcam::virtcam::VirtcamDevice::generate_properties()
{
    property_backend_ = std::make_shared<VirtcamPropertyBackend>(this);


    auto trigger_software = std::make_shared<prop_impl_trigger_software>(property_backend_);
    properties_.push_back(trigger_software);

    // trigger mode
    {
        std::vector<std::weak_ptr<tcam::property::PropertyLock>> dependencies_to_include;

        dependencies_to_include.push_back(
            std::dynamic_pointer_cast<tcam::property::PropertyLock>(trigger_software));

        auto trigger_mode = std::make_shared<prop_impl_trigger_mode>(property_backend_);

        auto ptr = std::dynamic_pointer_cast<tcam::property::PropertyLock>(trigger_mode);
        ptr->set_dependent_properties(std::move(dependencies_to_include));

        properties_.push_back(trigger_mode);
    }
}
