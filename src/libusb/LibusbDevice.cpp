/*
 * Copyright 2017 The Imaging Source Europe GmbH
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

#include "LibusbDevice.h"

#include "../logging.h"
#include "UsbHandler.h"

#include <algorithm>
#include <stdexcept>

tcam::LibusbDevice::LibusbDevice(const std::shared_ptr<tcam::UsbSession>& s,
                                 const std::string& serial)
    : session_(s)
{
    device_handle_ = UsbHandler::get_instance().open_device(serial);
    if (!device_handle_)
    {
        SPDLOG_ERROR("Failed to open device.");
    }
}


tcam::LibusbDevice::LibusbDevice(const std::shared_ptr<tcam::UsbSession>& s, libusb_device* dev)
    : session_(s), device_(dev)
{
    if (device_)
    {
        libusb_ref_device(device_);
        int ret = libusb_open(device_, &device_handle_);

        if (ret < 0)
        {
            SPDLOG_ERROR("Unable to open device.");
            throw std::runtime_error("Unable to open device. LibUsb returned "
                                     + std::to_string(ret));
        }
    }
    else
    {
        throw std::runtime_error("No libusb_device.");
    }
}


tcam::LibusbDevice::~LibusbDevice()
{
    auto open_interfaces_copy = open_interfaces_;

    for (int interface : open_interfaces_copy) { close_interface(interface); }

    if (device_handle_)
    {
        libusb_close(device_handle_);
    }

    if (device_)
    {
        libusb_unref_device(device_);
    }
}


struct libusb_device_handle* tcam::LibusbDevice::get_handle()
{
    return device_handle_;
}


bool tcam::LibusbDevice::open_interface(int interface)
{
    if (std::find(open_interfaces_.begin(), open_interfaces_.end(), interface)
        != open_interfaces_.end())
    {
        SPDLOG_WARN("Interface {} is already open.", interface);
        return false;
    }

    int ret = libusb_claim_interface(device_handle_, interface);

    if (ret < 0)
    {
        SPDLOG_ERROR("Could not claim interface {}", interface);
        return false;
    }

    open_interfaces_.push_back(interface);

    return true;
}


bool tcam::LibusbDevice::close_interface(int interface)
{
    int ret = libusb_release_interface(device_handle_, interface);

    if (ret < 0)
    {
        SPDLOG_ERROR("Could not release interface {}", interface);
        return false;
    }

    auto entry = std::find(open_interfaces_.begin(), open_interfaces_.end(), interface);

    if (entry != open_interfaces_.end())
    {
        open_interfaces_.erase(entry);
    }

    return true;
}


bool tcam::LibusbDevice::is_superspeed()
{
    if (!device_)
    {
        return false;
    }

    if (libusb_get_device_speed(device_) >= LIBUSB_SPEED_SUPER)
    {
        return true;
    }
    return false;
}


int tcam::LibusbDevice::get_max_packet_size(int endpoint)
{
    if (!device_)
    {
        return -1;
    }

    return libusb_get_max_packet_size(device_, endpoint);
}


int tcam::LibusbDevice::internal_control_transfer(uint8_t RequestType,
                                                  uint8_t Request,
                                                  uint16_t Value,
                                                  uint16_t Index,
                                                  unsigned char* data,
                                                  unsigned int size,
                                                  unsigned int timeout)
{
    return libusb_control_transfer(
        device_handle_, RequestType, Request, Value, Index, (unsigned char*)&data, size, timeout);
}


void tcam::LibusbDevice::halt_endpoint(int endpoint)
{
    if (libusb_clear_halt(device_handle_, endpoint) != 0)
    {
        SPDLOG_ERROR("Could not halt endpoint");
    }
}
