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

#ifndef _USBCAMERA_H_
#define _USBCAMERA_H_

#include "definitions.h"
#include "UsbSession.h"

#include <functional>
#include <vector>
#include <memory>
#include <libusb-1.0/libusb.h>

namespace tis
{

struct device_info
{
    char manufacturer[256];
    char product[256];
    char serial[256];
    uint16_t idVendor;
    uint16_t idProduct;
};

const camera_type find_camera_type (const unsigned int& idVendor,
                                    const unsigned int& idProduct);


class UsbCamera
{

protected:
    std::shared_ptr<UsbSession> usb_session;
    libusb_device_handle* dev_handle;
    device_info dev;
    int interface;

public:

    UsbCamera (std::shared_ptr<UsbSession> session,
               device_info dev,
               unsigned int _interface = 0);

    virtual ~UsbCamera ();

    virtual bool open ();

    virtual bool close ();


    /// @name get_firmware_version
    /// @return number of firmware version
    virtual int get_firmware_version () = 0;

    virtual std::string get_firmware_version_string () = 0;

    virtual int delete_firmware (std::function<void(int)> progress) = 0;

    /// @name download_firmware
    /// @param firmware - vector containing the firmware file
    /// @param progress - callback function
    /// @return libusb return value; >= 0 on success
    virtual int download_firmware (std::vector<unsigned char>& firmware,
                                   std::function<void(int)> progress = 0) = 0;

    /// @name upload_firmware
    /// @param firmware_package - firmware or firmware package that shall be used
    /// @param firmware - firmware of package; else empty
    /// @param progress - callback function
    /// @return true on success
    virtual bool upload_firmware (const std::string& firmware_package,
                                  const std::string& firmware,
                                  std::function<void(int)> progress) = 0;

    /// @name get_device_info
    /// @return device_info of the camera
    virtual device_info get_device_info ();

    /// @name get_camera_type
    /// @return camera_type
    virtual camera_type get_camera_type ();


    virtual UVC_COMPLIANCE get_mode () = 0;

    virtual int set_mode (UVC_COMPLIANCE mode) = 0;

    virtual unsigned int get_eeprom_size () = 0;

private:

    void claim_interface ();

    void release_interface ();

}; /* class UsbCamera */

} /* namespace tis */

#endif /* _USBCAMERA_H_ */
