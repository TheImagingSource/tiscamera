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

#include "DeviceInfo.h"

#include <cstring>
#include <string>

using namespace tcam;

DeviceInfo::DeviceInfo (const struct tcam_device_info& device_desc)
    : device(device_desc)
{}


DeviceInfo::DeviceInfo ()
{
    device.type = TCAM_DEVICE_TYPE_UNKNOWN;

    memcpy(device.identifier, "\0", sizeof(device.identifier));
    memcpy(device.name, "\0", sizeof(device.name));
    memcpy(device.serial_number, "\0", sizeof(device.serial_number));
}


DeviceInfo& DeviceInfo::operator= (const DeviceInfo& other)
{
    this->device = other.device;
    return *this;
}


struct tcam_device_info DeviceInfo::get_info () const
{
    return device;
}


std::string DeviceInfo::get_name () const
{
    return device.name;
}


std::string DeviceInfo::get_serial () const
{
    return device.serial_number;
}


std::string DeviceInfo::get_identifier () const
{
    return device.identifier;
}


enum TCAM_DEVICE_TYPE DeviceInfo::get_device_type () const
{
    return device.type;
}


std::string DeviceInfo::get_device_type_as_string () const
{
    switch (device.type)
    {
        case TCAM_DEVICE_TYPE_V4L2:
            return "V4L2";
        case TCAM_DEVICE_TYPE_ARAVIS:
            return "Aravis";
        case TCAM_DEVICE_TYPE_LIBUSB:
            return "LibUsb";
        default:
            return "Unknown";
    }
}
