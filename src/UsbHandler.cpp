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

#include <iostream>
#include <stdexcept>

#include "UsbHandler.h"
#include "LibusbDevice.h"

#include "logging.h"

#include <cstring>
#include <cstdio>

namespace tcam
{

UsbHandler& UsbHandler::get_instance ()
{
    static UsbHandler instance;

    return instance;
}


UsbHandler::UsbHandler ():
    session(new UsbSession())
{}


UsbHandler::~UsbHandler()
{}


struct libusb_device_handle* UsbHandler::open_device (const std::string& serial)
{
    struct libusb_device_handle* ret = nullptr;

    libusb_device** devs;

    int cnt = libusb_get_device_list(this->session->get_session(), &devs);

    if (cnt < 0)
    {
        throw std::runtime_error("Unable to retrieve device list. " + std::to_string(cnt));
    }

    for (ssize_t i = 0; i < cnt; i++)
    {
        libusb_device_descriptor desc;
        int r = libusb_get_device_descriptor(devs[i], &desc);
        if (r < 0)
        {
            throw std::runtime_error("Unable to retrieve device descriptor. " + std::to_string(cnt));
        }

        // ignore all devices that are not from TIS or otherwise needed
        if (desc.idVendor != 0x199e)
            continue;

        if (desc.idProduct != 0x8209 && desc.idProduct != 0x0804)
            continue;

        r = libusb_open(devs[i], &ret);

        if (r < 0)
        {
            tcam_log(TCAM_LOG_ERROR, "Unable to open device.");
            continue;
        }

        char tmp_str[sizeof(tcam_device_info::serial_number)];

        libusb_get_string_descriptor_ascii(ret, desc.iSerialNumber,
                                           (unsigned char*)tmp_str,
                                           sizeof(tcam_device_info::serial_number));
        if (serial.compare(tmp_str) == 0)
        {

            break;
        }

        libusb_close(ret);

    }

    libusb_free_device_list(devs, 1);


    return ret;
}


std::vector<DeviceInfo> UsbHandler::get_device_list ()
{
    libusb_device** devs;

    int cnt = libusb_get_device_list(this->session->get_session(), &devs);

    if (cnt < 0)
    {
        throw std::runtime_error("Unable to retrieve device list. " + std::to_string(cnt));
    }

    std::vector<DeviceInfo> ret;
    ret.reserve(5);

    for (ssize_t i = 0; i < cnt; i++)
    {
        libusb_device_descriptor desc;
        int r = libusb_get_device_descriptor(devs[i], &desc);
        if (r < 0)
        {
            throw std::runtime_error("Unable to retrieve device descriptor. " + std::to_string(cnt));
        }

        // ignore all devices that are not from TIS or otherwise needed
        if (desc.idVendor != 0x199e)
            continue;

        if (desc.idProduct != 0x8209 && desc.idProduct != 0x0804)
            continue;

        tcam_device_info d = { };

        d.type = TCAM_DEVICE_TYPE_LIBUSB;

        libusb_device_handle* dh;
        r = libusb_open(devs[i], &dh);

        if (r < 0)
        {
            tcam_log(TCAM_LOG_ERROR, "Unable to open device.");
            continue;
        }

        snprintf((char*)d.additional_identifier, sizeof(d.additional_identifier),
                 "%x", desc.idProduct);

        libusb_get_string_descriptor_ascii(dh, desc.iProduct, (unsigned char*)d.name, sizeof(d.name));
        libusb_get_string_descriptor_ascii(dh, desc.iSerialNumber, (unsigned char*)d.serial_number, sizeof(d.serial_number));

        libusb_close(dh);
        ret.push_back(DeviceInfo(d));
    }

    libusb_free_device_list(devs, 1);

    return ret;
}


std::shared_ptr<UsbSession> UsbHandler::get_session ()
{
    return this->session;
}

}; /* namespace tis */
