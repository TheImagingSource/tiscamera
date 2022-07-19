/*
 * Copyright 2022 The Imaging Source Europe GmbH
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

#include "virtcam_api.h"

#include "virtcam_device.h"

#include "../utils.h"

using namespace tcam;

static std::vector<tcam::DeviceInfo> get_virtcam_device_list()
{
    std::vector<tcam::DeviceInfo> rval;

    std::vector<std::string> split_names;
    auto env_devices = tcam::get_environment_variable( "TCAM_VIRTCAM_DEVICES", "" );
    if (env_devices.empty())
    {
        // no devices wanted
        return rval;
    }
    else
    {
        split_names = split_string( env_devices, ":" );
    }

    int index = 0;
    for (auto name : split_names)
    {
        std::string serial = "7150" + std::to_string( index++ );

        tcam_device_info tmp = {};
        tmp.type = TCAM_DEVICE_TYPE::TCAM_DEVICE_TYPE_VIRTCAM;
        strncpy(tmp.name, name.c_str(), sizeof(tmp.name));
        strncpy(tmp.identifier, name.c_str(), sizeof(tmp.identifier));
        strncpy(tmp.serial_number, serial.c_str(), sizeof(tmp.serial_number));

        rval.push_back( DeviceInfo(tmp) );
    }
    return rval;
}

std::shared_ptr<tcam::DeviceInterface> tcam::virtcam::VirtBackend::open_device(const tcam::DeviceInfo& device)
{

    return std::shared_ptr<DeviceInterface>(new VirtcamDevice(device));
}

std::vector<tcam::DeviceInfo> tcam::virtcam::VirtBackend::get_device_list()
{
    return get_virtcam_device_list();
}
