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

#include "Usb3Camera.h"

#include <string>
#include <vector>

#include "FileHandling.h"

#include <iostream>
#include <exception>
#include <stdexcept>

#define EEPROM_BUFFER_SIZE              4096
#define FIRMWARE_SIZE                   0x50000
#define SECTOR_SIZE                     0x10000
#define TIS_FX3_REQ_FIRMWARE_VERSION    0x01    // 4 bytes
#define TIS_FX3_REQ_EEPROM              0x10    // len = wLength bytes (max EEPROM_BUFFER_SIZE)
#define TIMEOUT                         10000
#define TIS_FX3_REQ_DEVICE_RESET        0x11    // value = index = 0xB007, len = 0

#define min(a,b)            ((a<b)?(a):(b))

namespace tis
{

Usb3Camera::Usb3Camera (std::shared_ptr<UsbSession> session, device_info _dev, unsigned int _interface):
UsbCamera(session, _dev, _interface)
{}


Usb3Camera::~Usb3Camera ()
{}


int Usb3Camera::get_firmware_version ()
{
    int size = 4;
    int data;

    int ret = libusb_control_transfer(dev_handle,
                                      LIBUSB_ENDPOINT_IN | LIBUSB_RECIPIENT_DEVICE | LIBUSB_REQUEST_TYPE_VENDOR,
                                      TIS_FX3_REQ_FIRMWARE_VERSION,
                                      0,
                                      0,
                                      (unsigned char*)&data,
                                      size,
                                      TIMEOUT);

    if (ret < 0)
    {
        throw std::runtime_error("Unable to read firmware version. Libusb returned " + std::to_string(ret) + ".");
    }

    return data;
}


int Usb3Camera::write_eeprom (unsigned int addr, unsigned char* data, unsigned int size)
{
    return libusb_control_transfer(this->dev_handle,
                                   LIBUSB_ENDPOINT_OUT | LIBUSB_RECIPIENT_DEVICE | LIBUSB_REQUEST_TYPE_VENDOR,
                                   TIS_FX3_REQ_EEPROM,
                                   addr >> 16,
                                   addr & 0xFFFF,
                                   data,
                                   size,
                                   TIMEOUT);
}


int Usb3Camera::read_eeprom (unsigned int addr, unsigned char* data, unsigned int size)
{
    return libusb_control_transfer(this->dev_handle,
                                   LIBUSB_ENDPOINT_IN | LIBUSB_RECIPIENT_DEVICE | LIBUSB_REQUEST_TYPE_VENDOR,
                                   TIS_FX3_REQ_EEPROM,
                                   addr >> 16,
                                   addr & 0xFFFF,
                                   data,
                                   size,
                                   TIMEOUT);
}

int Usb3Camera::download_firmware (std::vector<unsigned char>& firmware, std::function<void(int)> progress)
{
    int ret = -1;
    unsigned int size = 4096;

    unsigned char* data = new unsigned char[size];
    unsigned int max = firmware.size();
    for (unsigned int addr = 0; addr < max; addr += size)
    {
        size_t block_size = min( firmware.size() - addr, size );

        ret = read_eeprom(addr, firmware.data() + addr, block_size);

        if (ret < 0)
        {
            delete[] data;
            throw std::runtime_error("Unable to read data. Libusb returned " + std::to_string(ret) + ".");
        }
        else
        {
            progress( addr * 100 / max );
        }
    }

    delete[] data;
    return ret;
}


UVC_COMPLIANCE Usb3Camera::get_mode ()
{
    return CAMERA_INTERFACE_MODE_UVC;
}

int Usb3Camera::set_mode (UVC_COMPLIANCE mode)
{
    return -1;
}

int Usb3Camera::erase_sector (unsigned int addr)
{
    return write_eeprom(addr, 0, 0);
}


int Usb3Camera::erase_eeprom (std::function<void(int)> progress)
{
    int r = -1;
    progress(0);

    for (unsigned int i = 0; i < 5; ++i)
    {
        r = erase_sector(i * SECTOR_SIZE);

        if (r == 0)
            progress(20 * (i+1));
    }
    return r;
}


int Usb3Camera::upload_firmware_file (std::vector<uint8_t> firmware ,std::function<void(int)> progress)
{
    const size_t BLOCK_SIZE = 4096;
    int ret = 0;

    for (unsigned int addr = 0; addr < firmware.size(); addr += BLOCK_SIZE)
    {
        size_t block_size = min( firmware.size() - addr, BLOCK_SIZE );

        ret = write_eeprom( addr, firmware.data() + addr, block_size );

        if (ret < 0)
        {
            throw std::runtime_error("Unable to write eeprom. Libusb returned " + std::to_string(ret) + ".");
        }
        else
        {
            progress( addr * 100 / firmware.size() );
        }
    }
    return ret;
}


bool Usb3Camera::upload_firmware (const std::string& firmware_package,
                                  const std::string& firmware,
                                  std::function<void(int)> progress)
{
    int r = -1;

    std::vector<unsigned char> fw;
    if (is_package_file(firmware_package))
    {
        fw = extract_file_from_package(firmware_package, firmware);
    }
    else
        fw = load_file(firmware_package);

    if (fw.empty())
    {
        throw std::runtime_error("Firmware File could not be loaded correctly");
    }

    auto map_progress = [] ( std::function<void(int)> progress, int begin, int end )
        {
            return [=]( int x )
            {
                progress( begin + x * (end-begin) / 100 );
            };
        };

    int retry = 0;
    while (retry++ < 5)
    {
        progress( 0 );

        r = erase_eeprom( map_progress( progress, 0, 20 ) );

        if ( r < 0 )
        {
            continue;
        }
        try
        {
            r = upload_firmware_file( fw, map_progress( progress, 20, 70 ) );
        }
        catch (std::runtime_error& err)
        {

        }

        if ( r < 0 )
        {
            continue;
        }
        std::vector<unsigned char> buffer( fw.size() );
        try
        {
            r = download_firmware( buffer, map_progress( progress, 70, 100 ) );
        }
        catch (std::runtime_error& err)
        {
            throw;
        }

        if ( r < 0 )
            continue;

        if ( fw == buffer )
        {
            progress( 100 );
            return true;
        }
    }

    progress( 0 );
    return false;
}

}; /* namespace tis */
