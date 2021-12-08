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
