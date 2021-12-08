/*
 * Copyright 2021 The Imaging Source Europe GmbH
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

#pragma once

#include <memory>
#include <string_view>
#include <vector>

namespace tcam::property
{
class PropertyLock
{
public:
    virtual ~PropertyLock() = default;

    /**
     * Called when a property like 'ExposureAuto' wants to 'lock' 'ExposureTime'.
     * 'ExposureAuto' calls this, when its state changes, on 'ExposureTime' to lock it.
     */
    virtual void set_locked(bool new_locked_state) = 0;
    
    /**
     * Function called to specify the list of dependent properties
     * This then returns the list of dependent names, and if available set_dependent_names is called with the according property list.
     */
    virtual void set_dependent_properties(std::vector<std::weak_ptr<PropertyLock>>&&) = 0;
    /**
     * Function called after all properties were created.
     * This then returns the list of dependent names, and if available set_dependent_names is called with the according property list.
     */
    virtual std::vector<std::string_view> get_dependent_names() const {
        return {};
    }
};

struct dependency_entry
{
    const std::string_view name;
    const std::vector<std::string_view> dependent_property_names;
    const std::string_view prop_enum_state_for_locked;
    //const bool prop_boolean_state_for_locked = false; // currently unused
};

const dependency_entry* find_dependency_entry(std::string_view name);

} // namespace tcam::property
