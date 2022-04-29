/*
 * Copyright 2016 The Imaging Source Europe GmbH
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

#include "DeviceInfo.h"

#include <vector>
#include <memory>

namespace tcam
{

// forward declaration
class DeviceInterface;

/*
 * This class is meant as the definition of backend accessibility
 */
class BackendInterface
{
public:
    virtual ~BackendInterface(){};
    virtual TCAM_DEVICE_TYPE get_type() const = 0;
    virtual std::shared_ptr<DeviceInterface> open_device(const tcam::DeviceInfo&) = 0;
    virtual std::vector<DeviceInfo> get_device_list()= 0;
};

} // namespace tcam
