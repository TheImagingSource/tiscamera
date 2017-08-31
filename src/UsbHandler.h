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

#ifndef TCAM_USBHANDLER_H
#define TCAM_USBHANDLER_H

#include <vector>
#include <string>
#include <memory>

#include <libusb-1.0/libusb.h>

#include "UsbSession.h"

#include "DeviceInfo.h"

namespace tcam
{

class UsbCamera;

class UsbHandler
{
private:
    std::shared_ptr<UsbSession> session;
    UsbHandler ();
    ~UsbHandler ();

public:

    static UsbHandler& get_instance();

    UsbHandler (const UsbHandler& _handler) = delete;
    UsbHandler& operator=(const UsbHandler&) = delete;


    struct libusb_device_handle* open_device (const std::string& serial);

    /// @name get_device_list
    /// @return vector of device_info of found cameras
    std::vector<DeviceInfo> get_device_list ();

    /// @name open_camera
    /// @param serial - string containing the serial number of the camera that shall be opened
    /// @return shared pointer to the opened usb camera; Reutrns nullptr on failure
    // std::shared_ptr<UsbCamera> open_camera (std::string serial);

    std::shared_ptr<UsbSession> get_session ();



}; /* class UsbHandler */

} /* namespace tcam */

#endif /* TCAM_USBHANDLER_H */
