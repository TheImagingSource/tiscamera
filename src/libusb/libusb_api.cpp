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

#include "libusb_api.h"

#include "../logging.h"
#include "AFU050Device.h"
#include "AFU420Device.h"
#include "UsbHandler.h"
#include "libusb_utils.h"


std::shared_ptr<tcam::DeviceInterface> tcam::LibUsbBackend::open_device(const tcam::DeviceInfo& device)
{
    if (strcmp(device.get_info().additional_identifier, "804") == 0)
    {
        return std::shared_ptr<DeviceInterface>(new AFU420Device(device));
    }
    else if (strcmp(device.get_info().additional_identifier, "8209") == 0)
    {
        return std::shared_ptr<DeviceInterface>(new AFU050Device(device));
    }
    else
    {
        SPDLOG_ERROR("Unable to identify requested LibUsb Backend %x",
                     device.get_info().additional_identifier);
        return nullptr;
    }
}


std::vector<tcam::DeviceInfo> tcam::LibUsbBackend::get_device_list()
{
    return tcam::libusb::get_libusb_device_list();
}
