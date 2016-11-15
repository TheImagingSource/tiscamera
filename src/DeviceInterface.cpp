/*
 * Copyright 2014 The Imaging Source Europe GmbH
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

#include "DeviceInterface.h"
#include "logging.h"

#include "BackendLoader.h"

#include <algorithm>
#include <memory>

using namespace tcam;


std::shared_ptr<DeviceInterface> tcam::openDeviceInterface (const DeviceInfo& device)
{

    try
    {
        return BackendLoader::getInstance().open_device(device);
    }
    catch (...)
    {
        tcam_log(TCAM_LOG_ERROR, "Encountered Error while creating device interface.");
        return nullptr;
    }
    return nullptr;
}
