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

#include "v4l2_genicam_conversion.h"

#include <algorithm>

bool tcam::v4l2::is_id_present(const v4l2_queryctrl_list& qctrl_list,
                               uint32_t id_to_look_for) noexcept
{
    return std::any_of(qctrl_list.begin(),
                       qctrl_list.end(),
                       [id_to_look_for](const v4l2_queryctrl& ctrl)
                       { return ctrl.id == id_to_look_for; });
}

auto tcam::v4l2::converter_scale_init_integer::to_range(
    prop_range_integer_default device_range) const noexcept -> prop_range_integer_default
{
    auto rval = prop_range_integer_default {
        {
            static_cast<int64_t>(from_device(device_range.range.min)),
            static_cast<int64_t>(from_device(device_range.range.max)),
            static_cast<int64_t>(from_device(device_range.range.stp)),
        },
        static_cast<int64_t>(from_device(device_range.def)),
    };

    if (overwrite_min_.has_value())
        rval.range.min = static_cast<int64_t>(overwrite_min_.value());

    if (overwrite_max_.has_value())
        rval.range.max = static_cast<int64_t>(overwrite_max_.value());

    if (overwrite_stp_.has_value())
        rval.range.stp = static_cast<int64_t>(overwrite_stp_.value());

    if (overwrite_def_.has_value())
        rval.def = static_cast<int64_t>(overwrite_def_.value());

    if (transform_integer_range_)
        return transform_integer_range_(rval);

    return rval;
}

auto tcam::v4l2::converter_scale_init_float::to_range(
    prop_range_integer_default device_range) const noexcept -> prop_range_float_default
{
    auto rval = prop_range_float_default {
        {
            from_device(device_range.range.min),
            from_device(device_range.range.max),
            from_device(device_range.range.stp),
        },
        from_device(device_range.def),
    };

    if (overwrite_min_.has_value())
        rval.range.min = overwrite_min_.value();

    if (overwrite_max_.has_value())
        rval.range.max = overwrite_max_.value();

    if (overwrite_stp_.has_value())
        rval.range.stp = overwrite_stp_.value();

    if (overwrite_def_.has_value())
        rval.def = overwrite_def_.value();

    if (transform_float_range_)
        return transform_float_range_(rval);

    return rval;
}
