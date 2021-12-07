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

#include "property_dependencies.h"

#include <algorithm>
#include <iterator>

namespace
{

static const tcam::property::dependency_entry dependency_list[] =
{

        {
            "ExposureAuto", {"ExposureTime"},
        },
        {
            "ExposureAutoUpperLimitAuto", {"ExposureAutoUpperLimit"},
        },
        {
            "GainAuto",
            {
                "Gain"
            }
        },
        {
            "BalanceWhiteAuto",
            {
                "BalanceWhiteRed","BalanceWhiteGreen", "BalanceWhiteBlue",
            }
        },
        {
            "OffsetAuto",
            {
                "OffsetX", "OffsetY"
            }
        },
        {
            "TriggerMode", {"TriggerSoftware"},
        }
};

} // namespace

bool tcam::property::enum_to_bool (const std::string_view& value)
{
    if (value == "On"
        || value == "Continuous")
    {
        return true;
    }
    return false;

}

const tcam::property::dependency_entry* tcam::property::find_dependency(const std::string_view& name)
{
    auto iter = std::find_if(std::begin(dependency_list), std::end(dependency_list), [name](const dependency_entry& dep)
    {
        return dep.name == name;
    });

    if (iter != std::end(dependency_list))
    {
        return iter;
    }
    return nullptr;
}
