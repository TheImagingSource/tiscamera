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

    virtual void set_locked(bool new_locked_state) = 0;
    virtual bool lock_others() const = 0;
    virtual void set_dependencies(std::vector<std::weak_ptr<PropertyLock>>&) = 0;
};

struct dependency_entry
{
    const std::string_view name;
    const std::vector<std::string_view> dependencies;
};

bool enum_to_bool(const std::string_view& value);

const dependency_entry* find_dependency(const std::string_view& name);

} // namespace tcam::property
