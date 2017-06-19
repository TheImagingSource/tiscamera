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

#ifndef _USB2CAMERA_H_
#define _USB2CAMERA_H_

#include "UsbSession.h"
#include "UsbCamera.h"

#include <string>
#include <memory>
#include <vector>
#include <functional>

#include "libusb-1.0/libusb.h"

namespace tis
{

struct device_info;

class Usb2Camera : public UsbCamera
{

public:

    Usb2Camera (std::shared_ptr<UsbSession> session, device_info dev, unsigned int _interface = 0);

    ~Usb2Camera ();

    Usb2Camera (const Usb2Camera& _camera) = delete;
    Usb2Camera& operator=(const Usb2Camera&) = delete;

    /// @name get_firmware_version
    /// @return integer containing the currently used firmware version
    int get_firmware_version ();

    std::string  get_firmware_version_string ();

    int delete_firmware (std::function<void(int)> progress);

    /// @name download_firmware
    /// @param firmware - vector containing the firmware file
    /// @param progress - callback function
    /// @return libusb return value; >= 0 on success
    int download_firmware (std::vector<unsigned char>& firmware, std::function<void(int)> progress);

    /// @name upload_firmware
    /// @param firmware_package - path to firmware/firmwarepack that shall be used
    /// @param firmware - optional firmware file name that shall be extracted from firmwarepack
    /// @param progress - callback function for updates
    /// @return true if success
    /// @brief uploads firmware to camera
    bool upload_firmware (const std::string& firmware_package,
                          const std::string& firmware,
                          std::function<void(int)> progress);

    /// @name set_mode
    /// @param mode -  UVC_COMPLIANCE the camera shall use
    /// @return libusb return value; >= 0 on success
    int set_mode (UVC_COMPLIANCE mode);

    /// @name get_mode
    /// @return UVC_COMPLIANCE the camera currently has
    UVC_COMPLIANCE get_mode ();

    /// @name get_eeprom_size
    /// @return size of EEPROM present on the camera
    unsigned int get_eeprom_size ();


private:

    /// @name write_eeprom
    /// @param addr - address to which to write
    /// @param data - pointer to data which shall be written
    /// @param size -
    /// @return integer response from libusb
    int write_eeprom (unsigned int addr, unsigned char* data, unsigned int size);

    /// @name read_eeprom
    /// @param addr - address from which to read
    /// @param data - buffer in which read data shall be written
    /// @param size - size of data
    /// @return integer response from libusb; >= 0 on success
    int read_eeprom (unsigned int addr, unsigned char* data, unsigned int size);

    int usbbuffer_to_string (unsigned char* usbbuffer,
                             int buffer_size,
                             char* string,
                             int string_size);

    int string_to_usbbuffer (unsigned char* usbbuffer, int buffer_size, const char* string);

    /// @name upload_firmware_file
    /// @param buffer - pointer to
    /// @param size -
    /// @param progress - callback function
    /// @return integer response from libusb; >= 0 on success
    int upload_firmware_file (unsigned char* buffer, unsigned int size, std::function<void(int)> progress);

    char* read_string (int index);

    /// @name write_strings
    /// @param strvendor
    /// @param strproduct
    /// @param strserial
    /// @return integer response from libusb
    int write_strings (const char* strvendor,
                       const char* strproduct,
                       const char* strserial);

    void get_strings (char* strvendor,
                      char* strproduct,
                      char* strserial,
                      int bufsize);

    char* read_string_from_image ( unsigned char* image, int index );

    bool apply_strings ( const char** strings, unsigned char* eeprom_image );

    /// @name patch_strings
    /// @param model - string containing the model description the camera shall use
    /// @param vendor - string containing the vendor description the camera shall use
    /// @param serial - string containing the serial number the camera shall use
    /// @param eeprom_image - pointer to the firmware buffer that shall be patched
    /// @return true on success
    /// @brief searches positions of strings in vanilla firmware image and writes given strings
    bool patch_strings (const char* model,
                        const char* vendor,
                        const char* serial,
                        unsigned char* eeprom_image);

    int get_productid (unsigned short& pid);

    int set_productid (unsigned short pid);

}; /* class Usb2Camera */

} /* namespace tis */

#endif /* _USB2CAMERA_H_ */
