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

        if (desc.idProduct != 0x8209)
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

        if (desc.idProduct != 0x8209)
            continue;

        tcam_device_info d = { };

        d.type = TCAM_DEVICE_TYPE_LIBUSB;

        // d.idVendor = desc.idVendor;
        // d.idProduct = desc.idProduct;

        // strcpy(d.name, "AFU050");
        // strcpy(d.serial_number, "47614135");
        // strcpy(d.additional_identifier, "9209");

        libusb_device_handle* dh;
        r = libusb_open(devs[i], &dh);

        if (r < 0)
        {
            tcam_log(TCAM_LOG_ERROR, "Unable to open device.");
            continue;
        }

        libusb_get_string_descriptor_ascii(dh, desc.iProduct, (unsigned char*)d.name, sizeof(d.name));

        // libusb_get_string_descriptor_ascii(dh, desc.iManufacturer, (unsigned char*)d.manufacturer, sizeof(d.manufacturer));
        // int lib_ret = libusb_get_port_numbers(dh, (uint8_t*)d.identifier, sizeof(d.identifier))

        libusb_get_string_descriptor_ascii(dh, desc.idProduct, (unsigned char*)d.additional_identifier, sizeof(d.additional_identifier));
        libusb_get_string_descriptor_ascii(dh, desc.iSerialNumber, (unsigned char*)d.serial_number, sizeof(d.serial_number));

        libusb_close(dh);
        ret.push_back(DeviceInfo(d));
    }

    libusb_free_device_list(devs, 1);

    return ret;
}


// std::shared_ptr<UsbCamera> UsbHandler::open_camera (std::string serial_number)
// {
//     auto list = get_device_list();
//     device_info d;

//     for (auto& dev : list)
//     {
//         // std::cout << "Comparing |" << dev.serial << "|" << (unsigned char*)serial_number.c_str() << "|" << std::endl;
//         if (serial_number.compare(dev.serial) == 0)
//         {
//              // std::cout << "dev.serial " << dev.serial << ";serial " << serial_number << std::endl;
//             d = dev;
//             break;
//         }
//     }

//     camera_type t = find_camera_type(d.idVendor, d.idProduct);


//     switch(t.camera_type)
//     {
//         case USB33:
//             return std::make_shared<Usb33Camera>(this->session, d);
//         case USB3:
//             return std::make_shared<Usb3Camera>(this->session, d);
//         case USB2:
//             return std::make_shared<Usb2Camera>(this->session, d);
//         case UNKNOWN:
//         default:
//             return nullptr;
//     }
// }


std::shared_ptr<UsbSession> UsbHandler::get_session ()
{
    return this->session;
}

}; /* namespace tis */
