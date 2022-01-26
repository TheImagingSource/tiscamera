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

#include "UsbHandler.h"

#include "../logging.h"
#include "../utils.h"

#include <cstdio>
#include <cstring>
#include <stdexcept>

namespace tcam
{

UsbHandler& UsbHandler::get_instance()
{
    static UsbHandler instance;

    return instance;
}

UsbHandler::UsbHandler() : session(std::make_shared<UsbSession>()), run_event_thread(true)
{
    event_thread = std::thread(&UsbHandler::handle_events, this);
}


UsbHandler::~UsbHandler()
{
    run_event_thread = false;
    if (event_thread.joinable())
    {
        event_thread.join();
    }
}


std::unique_ptr<LibusbDevice> UsbHandler::open_device_(const std::string& serial)
{
    std::unique_ptr<LibusbDevice> ret = nullptr;

    libusb_device** devs;
    libusb_device_handle* dev;

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
            throw std::runtime_error("Unable to retrieve device descriptor. "
                                     + std::to_string(cnt));
        }

        // ignore all devices that are not from TIS or otherwise needed
        if (desc.idVendor != 0x199e)
            continue;

        if (desc.idProduct != 0x8209 && desc.idProduct != 0x0804)
            continue;

        r = libusb_open(devs[i], &dev);

        if (r < 0)
        {
            SPDLOG_ERROR("Unable to open device.");
            continue;
        }

        char tmp_str[sizeof(tcam_device_info::serial_number)] = {};

        libusb_get_string_descriptor_ascii(dev,
                                           desc.iSerialNumber,
                                           (unsigned char*)tmp_str,
                                           sizeof(tcam_device_info::serial_number));
        if (serial.compare(tmp_str) == 0)
        {
            libusb_close(dev);
            ret = std::make_unique<LibusbDevice>(session, devs[i]);
            break;
        }

        libusb_close(dev);
    }

    libusb_free_device_list(devs, 1);

    return ret;
}

//
//void printdev(libusb_device* dev)
//{
//    libusb_device_descriptor desc;
//    int r = libusb_get_device_descriptor(dev, &desc);
//    if (r < 0)
//    {
//        std::cout << "failed to get device descriptor" << std::endl;
//        return;
//    }
//    std::cout << "Number of possible configurations: " << (int)desc.bNumConfigurations << " ";
//    std::cout << "Device Class: " << (int)desc.bDeviceClass << " ";
//    std::cout << "VendorID: " << desc.idVendor << " ";
//    std::cout << "ProductID: " << desc.idProduct << std::endl;
//    libusb_config_descriptor* config;
//    libusb_get_config_descriptor(dev, 0, &config);
//    std::cout << "Interfaces: " << (int)config->bNumInterfaces << " ||| ";
//    const libusb_interface* inter;
//    const libusb_interface_descriptor* interdesc;
//    const libusb_endpoint_descriptor* epdesc;
//    for (int i = 0; i < (int)config->bNumInterfaces; i++)
//    {
//        inter = &config->interface[i];
//        std::cout << "Number of alternate settings: " << inter->num_altsetting << " | ";
//        for (int j = 0; j < inter->num_altsetting; j++)
//        {
//            interdesc = &inter->altsetting[j];
//            std::cout << "Interface Number: " << (int)interdesc->bInterfaceNumber << " | ";
//            std::cout << "Number of endpoints: " << (int)interdesc->bNumEndpoints << " | ";
//            for (int k = 0; k < (int)interdesc->bNumEndpoints; k++)
//            {
//                epdesc = &interdesc->endpoint[k];
//                std::cout << "Descriptor Type: " << (int)epdesc->bDescriptorType << " | ";
//                std::cout << "EP Address: " << (int)epdesc->bEndpointAddress << " | ";
//            }
//        }
//    }
//    std::cout << std::endl << std::endl << std::endl;
//    libusb_free_config_descriptor(config);
//}


struct libusb_device_handle* UsbHandler::open_device(const std::string& serial)
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
            throw std::runtime_error("Unable to retrieve device descriptor. "
                                     + std::to_string(cnt));
        }

        // ignore all devices that are not from TIS or otherwise needed
        if (desc.idVendor != 0x199e)
            continue;

        if (desc.idProduct != 0x8209 && desc.idProduct != 0x0804)
            continue;

        r = libusb_open(devs[i], &ret);

        if (r < 0)
        {
            SPDLOG_ERROR("Unable to open device.");
            continue;
        }

        char tmp_str[sizeof(tcam_device_info::serial_number)];

        libusb_get_string_descriptor_ascii(ret,
                                           desc.iSerialNumber,
                                           (unsigned char*)tmp_str,
                                           sizeof(tcam_device_info::serial_number));
        if (serial.compare(tmp_str) == 0)
        {
            //     SPDLOG_INFO("Max packet size for endpoint 0: {}", libusb_get_max_packet_size(devs[i], 0));

            // cout<<"Number of endpoints: "<<(int)interdesc->bNumEndpoints<<endl;
            // for(int k=0; k<(int)interdesc->bNumEndpoints; k++) {
            //     epdesc = &interdesc->endpoint[k];
            //     cout<<"Descriptor Type: "<<(int)epdesc->bDescriptorType<<endl;
            //     cout<<"EP Address: "<<(int)epdesc->bEndpointAddress<<endl;
            // }
            //printdev(devs[i]);
            break;
        }

        libusb_close(ret);
    }

    libusb_free_device_list(devs, 1);


    return ret;
}


std::vector<DeviceInfo> UsbHandler::get_device_list()
{
    libusb_device** devs = nullptr;

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
            throw std::runtime_error("Unable to retrieve device descriptor. "
                                     + std::to_string(cnt));
        }

        // ignore all devices that are not from TIS or otherwise needed
        if (desc.idVendor != 0x199e)
            continue;

        if (desc.idProduct != 0x8209 && desc.idProduct != 0x0804)
            continue;

        tcam_device_info d = {};

        d.type = TCAM_DEVICE_TYPE_LIBUSB;

        libusb_device_handle* dh;
        r = libusb_open(devs[i], &dh);

        if (r < 0)
        {
            SPDLOG_ERROR("Unable to open device.");
            continue;
        }

        snprintf(
            (char*)d.additional_identifier, sizeof(d.additional_identifier), "%x", desc.idProduct);

        libusb_get_string_descriptor_ascii(
            dh, desc.iProduct, (unsigned char*)d.name, sizeof(d.name));
        libusb_get_string_descriptor_ascii(
            dh, desc.iSerialNumber, (unsigned char*)d.serial_number, sizeof(d.serial_number));

        libusb_close(dh);
        ret.push_back(DeviceInfo(d));
    }

    libusb_free_device_list(devs, 1);

    return ret;
}

void UsbHandler::handle_events()
{
    tcam::set_thread_name("tcam_usbhand");
    struct timeval tv = {};
    tv.tv_usec = 200; // #TODO this seems to be an excessively short wake timeout
    while (run_event_thread)
    {
        libusb_handle_events_timeout_completed(this->session->get_session(), &tv, nullptr);
    }
}

} // namespace tcam
