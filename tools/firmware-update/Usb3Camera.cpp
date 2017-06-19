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
#include "FileHandling.h"

#include <string>
#include <vector>
#include <iostream>
#include <exception>
#include <stdexcept>

#include <unistd.h>

#include <string.h>

#define EEPROM_BUFFER_SIZE 4096
#define FIRMWARE_SIZE 0x50000
#define SECTOR_SIZE 0x10000
#define TIS_FX3_REQ_FIRMWARE_VERSION 0x01 // 4 bytes
#define TIS_FX3_REQ_EEPROM 0x10 // len = wLength bytes (max EEPROM_BUFFER_SIZE)
#define TIMEOUT 10000
#define TIS_FX3_REQ_DEVICE_RESET 0x11 // value = index = 0xB007, len = 0

#define min(a,b) ((a<b)?(a):(b))

namespace tis
{

Usb3Camera::Usb3Camera (std::shared_ptr<UsbSession> session,
                        device_info _dev,
                        unsigned int _interface):
    UsbCamera(session, _dev, _interface)
{
    open();
}


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


std::string Usb3Camera::get_firmware_version_string ()
{
    return std::to_string(get_firmware_version());
}


int Usb3Camera::delete_firmware (std::function<void(int)> progress)
{
    auto map_progress = [] ( std::function<void(int)> progress, int begin, int end )
        {
            return [=]( int x )
            {
                progress( begin + x * (end-begin) / 100 );
            };
        };

    erase_eeprom(map_progress(progress, 0, 100));

    return 0;
}


int Usb3Camera::write_eeprom (unsigned int addr,
                              unsigned char* data,
                              unsigned int size)
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


int Usb3Camera::read_eeprom (unsigned int addr,
                             unsigned char* data,
                             unsigned int size)
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

int Usb3Camera::download_firmware (std::vector<unsigned char>& firmware,
                                   std::function<void(int)> progress)
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

unsigned int Usb3Camera::get_eeprom_size ()
{
    return 0;
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


int Usb3Camera::upload_firmware_file (std::vector<uint8_t> firmware,
                                      std::function<void(int)> progress)
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



#define MAX_FWIMG_SIZE 533504 //(512 * 1024) // Maximum size of the firmware binary.
#define MAX_WRITE_SIZE 2048 //(2 * 1024) // Max. size of data that can be written through one vendor command.

#define I2C_PAGE_SIZE (64) // Page size for I2C EEPROM.
#define I2C_SLAVE_SIZE 65536 //(64 * 1024) // Max. size of data that can fit on one EEPROM address.

#define SPI_PAGE_SIZE (256) // Page size for SPI flash memory.
#define SPI_SECTOR_SIZE 65536 //(64 * 1024) // Sector size for SPI flash memory.

#define VENDORCMD_TIMEOUT (5000) // Timeout (in milliseconds) for each vendor command.
#define GETHANDLE_TIMEOUT (5) // Timeout (in seconds) for getting a FX3 flash programmer handle.

#define GET_LSW(v)	((unsigned short)((v) & 0xFFFF))	// Get Least Significant Word part of an integer.
#define GET_MSW(v)	((unsigned short)((v) >> 16))		// Get Most Significant Word part of an integer.


static int fx3_ram_write (libusb_device_handle* h,
                          unsigned char* buf,
                          unsigned int ramAddress,
                          int len)
{

    int index = 0;

    while (len > 0)
    {
        int size = (len > MAX_WRITE_SIZE) ? MAX_WRITE_SIZE : len;
        int r = libusb_control_transfer (h, 0x40, 0xA0, GET_LSW(ramAddress), GET_MSW(ramAddress),
                                         &buf[index], size, VENDORCMD_TIMEOUT);
        if (r != size)
        {
            fprintf (stderr, "Error: Vendor write to FX3 RAM failed\n");
            return -1;
        }

        ramAddress += size;
        index      += size;
        len        -= size;
    }

    return 0;
}


bool Usb3Camera::initialize_eeprom (std::vector<uint8_t>& firmware)
{

    unsigned char *fwBuf = firmware.data();
    int filesize = firmware.size();

    if (this->dev_handle == NULL)
    {
        return false;
    }

    // Run through each section of code, and use vendor commands to download them to RAM.
    int index = 4;
    unsigned int checksum = 0;
    int r;
    while (index < filesize)
    {
        unsigned int* data_p  = (unsigned int *)(fwBuf + index);
        int length  = data_p[0];
        int address = data_p[1];
        if (length != 0)
        {
            for (int i = 0; i < length; i++)
            {
                checksum += data_p[2 + i];
            }
            r = fx3_ram_write (this->dev_handle, fwBuf + index + 8, address, length * 4);
            if (r != 0)
            {
                fprintf (stderr, "Error: Failed to download data to FX3 RAM\n");
                free (fwBuf);
                return false;
            }
        }
        else
        {
            if (checksum != data_p[2])
            {
                fprintf (stderr, "Error: Checksum error in firmware binary\n");
                free (fwBuf);
                return false;
            }

            r = libusb_control_transfer (this->dev_handle,
                                         0x40,
                                         0xA0,
                                         GET_LSW(address),
                                         GET_MSW(address),
                                         NULL,
                                         0,
                                         VENDORCMD_TIMEOUT);
            if (r != 0)
            {
                printf ("Info: Ignored error in control transfer: %d\n", r);
            }
            break;
        }

        index += (8 + length * 4);
    }

    return true;

}


bool Usb3Camera::upload_firmware (const std::string& firmware_package,
                                  const std::string& firmware,
                                  std::function<void(int)> progress)
{
    if (!is_valid_firmware_file(firmware_package))
    {
        throw std::runtime_error("Not a valid firmware file!");
    }

    std::vector<unsigned char> fw;
    if (is_package_file(firmware_package))
    {
        fw = extract_file_from_package(firmware_package, firmware);
    }
    else
        fw = load_file(firmware_package);

    if (fw.empty() || ( fw.size() * sizeof(unsigned char) ) > FIRMWARE_SIZE)
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

    // if this device starts as cypress/westwood
    // handle it as a vanilla device that has to be initialized and
    // not just update
    if (this->dev.idVendor == 0x04b4)
    {
        return initialize_eeprom (fw);
    }

    // normal update cycle
    int retry = 0;
    while (retry++ < 5)
    {
        progress( 0 );

        int r = erase_eeprom( map_progress( progress, 0, 20 ) );

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
            std::cerr << err.what() << std::endl;
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
