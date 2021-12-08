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

namespace
{

// clang-format off
static const tcam::property::dependency_entry dependency_list[] =
{
    {
        "ExposureAuto",
        { "ExposureTime" },
        "Continuous"
    },
    {
        "ExposureAutoUpperLimitAuto",
        {"ExposureAutoUpperLimit"},
        "Continuous"
    },
    {
        "GainAuto",
        { "Gain" },
        "Continuous"
    },
    {
        "BalanceWhiteAuto",
        { "BalanceWhiteRed","BalanceWhiteGreen", "BalanceWhiteBlue", },
        "Continuous"
    },
    {
        "OffsetAutoCenter",
        { "OffsetX", "OffsetY" },
        "On"
    },
    {
        "TriggerMode",
        {"TriggerSoftware"},
        "Off"
    }
};

// clang-format on

} // namespace

const tcam::property::dependency_entry* tcam::property::find_dependency_entry(std::string_view name)
{
    for (const auto& entry : dependency_list)
    {
        if (entry.name == name)
        {
            return &entry;
        }
    }
    return nullptr;
}
