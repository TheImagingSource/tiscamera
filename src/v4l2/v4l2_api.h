/*
 * Copyright 2019 The Imaging Source Europe GmbH
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

#include "../devicelibrary.h"

namespace tcam
{

class V4L2Backend : public BackendInterface
{
public:
    TCAM_DEVICE_TYPE get_type() const final
    {
        return TCAM_DEVICE_TYPE_V4L2;
    };
    std::shared_ptr<DeviceInterface> open_device(const tcam::DeviceInfo&) final;
    std::vector<DeviceInfo> get_device_list() final;

    static V4L2Backend* get_instance()
    {
        static V4L2Backend b;
        return &b;
    };
};

} // namespace tcam
