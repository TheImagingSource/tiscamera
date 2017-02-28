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

#include "LibUsbShowDevice.h"

#include <stdexcept>
#include <cstdint>
#include <vector>

static const int TIMEOUT = 10000;

namespace lib33u
{
namespace driver_interface
{
namespace libusb
{

ShowDevice::ShowDevice (std::shared_ptr<UsbSession> ksps, device_info device)
    : usb_session(ksps), device(device), interface(0)
{}


ShowDevice::~ShowDevice ()
{
    release_interface();
    libusb_close(this->dev_handle);
}


uint16_t ShowDevice::product_id ()
{
    return device.idProduct;
}

void ShowDevice::open ()
{
    try
    {
        this->dev_handle = libusb_open_device_with_vid_pid(usb_session->get_session(),
                                                           device.idVendor,
                                                           device.idProduct);

        if (this->dev_handle == NULL)
        {
            throw std::runtime_error("Unable to attain device handle.");
        }

        claim_interface();
    }
    catch (std::runtime_error& err)
    {
        throw;
    }
}


void ShowDevice::read_vendor_request (uint8_t req,
                                      uint16_t value,
                                      uint16_t index,
                                      uint8_t* buffer,
                                      uint16_t length)
{
    int ret = libusb_control_transfer(this->dev_handle,
                                      LIBUSB_ENDPOINT_IN | LIBUSB_RECIPIENT_DEVICE | LIBUSB_REQUEST_TYPE_VENDOR,
                                      req,
                                      value,
                                      index,
                                      buffer,
                                      length,
                                      TIMEOUT);

    if (ret < 0)
    {
        throw std::runtime_error( "IKsPropertySet::Get failed" );
    }
}


void ShowDevice::write_vendor_request (uint8_t req,
                                       uint16_t value,
                                       uint16_t index,
                                       uint8_t* data,
                                       uint16_t length)
{
     int ret = libusb_control_transfer(this->dev_handle,
                                      LIBUSB_ENDPOINT_OUT | LIBUSB_RECIPIENT_DEVICE | LIBUSB_REQUEST_TYPE_VENDOR,
                                      req,
                                      value,
                                      index,
                                      (unsigned char*)data,
                                      length,
                                      TIMEOUT);

    if (ret < 0)
    {
        throw std::runtime_error(std::to_string(ret));
    }
}


void ShowDevice::claim_interface ()
{
    if (libusb_kernel_driver_active(this->dev_handle, this->interface) == 1)
    {
        if (libusb_detach_kernel_driver(dev_handle, interface) != 0)
        {
            throw std::runtime_error("Unable to detach kernel driver.");
        }
    }

    int r = libusb_claim_interface(this->dev_handle, this->interface);
    if (r < 0)
    {
        throw std::runtime_error("Unable to claim interface.");
    }
}


void ShowDevice::release_interface ()
{
    int r = libusb_release_interface(this->dev_handle, this->interface);
    if (r != 0)
    {
        throw std::runtime_error("Unable to release interface.");
    }
    libusb_attach_kernel_driver(this->dev_handle, this->interface);
}

} /* namespace libusb */
} /* namespace driver_interface */
} /* namespace lib33u */
