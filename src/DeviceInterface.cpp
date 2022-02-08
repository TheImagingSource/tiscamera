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

#include "BackendLoader.h"
#include "logging.h"

#include <algorithm>
#include <memory>

using namespace tcam;


std::shared_ptr<DeviceInterface> tcam::openDeviceInterface(const DeviceInfo& device)
{

    try
    {
        auto loader = BackendLoader::get_instance();
        return loader->open_device(device);
    }
    catch (const std::runtime_error& err)
    {
        SPDLOG_ERROR("Encountered Error while creating device interface. {}", err.what());
        return nullptr;
    }
    catch (...)
    {
        SPDLOG_ERROR("Caught unhandled exception while opening device.");
    }
    return nullptr;
}

outcome::result<tcam::framerate_info> DeviceInterface::get_framerate_info(const VideoFormat& fmt)
{
    for (auto&& desc : get_available_video_formats())
    {
        if (desc.get_fourcc() == fmt.get_fourcc())
        {
            return tcam::framerate_info { desc.get_framerates(fmt) };
        }
    }
    return tcam::status::FormatInvalid;
}
