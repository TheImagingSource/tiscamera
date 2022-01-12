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
#include "ep_defines_rx.h"

namespace tcam
{
class AFU420Device;
}

namespace tcam::property
{
class AFU420DeviceBackend
{
public:
    explicit AFU420DeviceBackend(AFU420Device*);

    outcome::result<int64_t> get_int(tcam::afu420::AFU420Property id);
    outcome::result<void> set_int(tcam::afu420::AFU420Property id, int64_t new_value);

    outcome::result<double> get_float(tcam::afu420::AFU420Property id);
    outcome::result<void> set_float(tcam::afu420::AFU420Property id, double new_value);

private:
    AFU420Device* p_device;
};

} // namespace tcam::property
