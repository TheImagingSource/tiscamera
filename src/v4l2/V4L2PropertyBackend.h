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

#include "../error.h"
#include "v4l2_genicam_conversion.h"

#include <vector>

namespace tcam::v4l2
{

class V4L2PropertyBackend
{
public:
    explicit V4L2PropertyBackend(int fd);

    outcome::result<int64_t> write_control(int v4l2_id, int new_value);

    outcome::result<int64_t> read_control(int v4l2_id);

    std::vector<tcam::v4l2::menu_entry> get_menu_entries(int v4l2_id, int max);
private:
    int p_fd = 0;
};

} // namespace tcam::property
