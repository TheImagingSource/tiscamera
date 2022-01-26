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

#ifndef TCAM_LIBUSBDEVICE_H
#define TCAM_LIBUSBDEVICE_H

#include "UsbSession.h"

#include <memory>
#include <string>
#include <vector>

namespace tcam
{

class LibusbDevice
{
public:
    LibusbDevice(const std::shared_ptr<tcam::UsbSession>&, const std::string& serial);
    LibusbDevice(const std::shared_ptr<tcam::UsbSession>&, libusb_device* dev);

    ~LibusbDevice();

    struct libusb_device_handle* get_handle();

    /**
     * Opened interfaces will automatically be closed on object destruction
     */
    bool open_interface(int interface);

    /**
     * Explicitly close an interface
     */
    bool close_interface(int interface);

    /**
     * @return True if device has USB3.0
     */
    bool is_superspeed();

    int get_max_packet_size(int endpoint);

    template<typename T>
    int control_transfer(uint8_t RequestType,
                         uint8_t Request,
                         uint16_t Value,
                         uint16_t Index,
                         T& data,
                         unsigned int size = sizeof(T),
                         unsigned int timeout = 500)
    {

        return internal_control_transfer(
            RequestType, Request, Value, Index, (unsigned char*)&data, size, timeout);
    }

    int control_transfer(uint8_t RequestType,
                         uint8_t Request,
                         uint16_t Value,
                         uint16_t Index,
                         unsigned char* data,
                         unsigned int size,
                         unsigned int timeout = 500)
    {

        return internal_control_transfer(RequestType, Request, Value, Index, data, size, timeout);
    }

    void halt_endpoint(int endpoint);

private:
    std::shared_ptr<tcam::UsbSession> session_;
    libusb_device* device_ = nullptr;
    libusb_device_handle* device_handle_ = nullptr;

    std::vector<int> open_interfaces_;

    int internal_control_transfer(uint8_t RequestType,
                                  uint8_t Request,
                                  uint16_t Value,
                                  uint16_t Index,
                                  unsigned char* data,
                                  unsigned int size,
                                  unsigned int timeout);
}; // class LibusbDevice

} // namespace tcam

#endif /* TCAM_LIBUSBDEVICE_H */
