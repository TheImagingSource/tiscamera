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

#include <linux/videodev2.h>
#include <string>
#include <vector>
#include <optional>

namespace tcam::v4l2
{

using v4l2_queryctrl_list = std::vector<v4l2_queryctrl>;

bool is_id_present(const v4l2_queryctrl_list& qctrl_list, uint32_t id_to_look_for) noexcept;

struct converter_scale
{
    using func_type_to_device = int64_t (*)(double);
    using func_type_from = double (*)(int64_t);

    func_type_to_device to_device_ = nullptr;
    func_type_from from_device_ = nullptr;

    std::optional<double> step_ = std::nullopt;
    std::optional<double> default_ = std::nullopt;
    int64_t to_device(double val) const
    {
        return to_device_ ? to_device_(val) : val;
    }
    double from_device(int64_t val) const
    {
        return from_device_ ? from_device_(val) : val;
    }
};

struct menu_entry
{
    int value = 0;
    std::string entry_name;
};

using menu_entry_list = std::vector<menu_entry>;
using fetch_menu_entries_func = std::vector<menu_entry> (*)();


constexpr uint32_t V4L2_CID_TIS_WHITEBALANCE_ONE_PUSH = 0x0199e206;

} // namespace tcam::v4l2
