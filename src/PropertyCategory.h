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

#include "PropertyFlags.h"
#include "base_types.h"
#include "visibility.h"

#include <memory>
#include <string>
#include <vector>

namespace tcam::property
{

class Category
{
    explicit Category(const std::string& name, std::shared_ptr<Category> parent = nullptr);

    tcam::Visibility get_visibility() const
    {
        return m_visibility;
    };
    std::string get_name() const
    {
        return m_name;
    };
    std::string get_description() const
    {
        return m_description;
    };

    std::shared_ptr<Category> get_parent()
    {
        auto p = m_parent.lock();
        if (p)
        {
            return p;
        }
        return nullptr;
    };
    std::vector<std::shared_ptr<Category>> get_children()
    {
        return m_children;
    };

private:
    tcam::Visibility m_visibility = tcam::Visibility::Beginner;
    std::string m_name;
    std::string m_description;

    bool is_root = false;
    std::weak_ptr<Category> m_parent;
    std::vector<std::shared_ptr<Category>> m_children;
};

std::string_view get_display_category(std::string_view property);
std::string get_display_group(std::string_view property);

} // namespace tcam::property
