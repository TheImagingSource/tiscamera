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
#include <string.h>
#include <unistd.h>
#include <stdexcept>

#include "Usb2Camera.h"
#include "FileHandling.h"

static const short TIMEOUT = 10000;

#define EEPROM_SIZE 16384
#define DATA_SIZE   512
#define FIRMWARE_END (EEPROM_SIZE - DATA_SIZE)
#define FLAGS_LOCATION (FIRMWARE_END-1)
#define PID_LOCATION_LO (0x5a)
#define PID_LOCATION_HI (0x5b)

#define min(a,b)            ((a<b)?(a):(b))

namespace tis
{

Usb2Camera::Usb2Camera (std::shared_ptr<UsbSession> session, device_info dev, unsigned int _interface):
    UsbCamera(session, dev, _interface)
{
    open();
}


Usb2Camera::~Usb2Camera ()
{}


int Usb2Camera::get_firmware_version ()
{
    int size = 1;
    int i = -1;
    unsigned char data[size];

    int ret = libusb_control_transfer(this->dev_handle,
                                      LIBUSB_ENDPOINT_IN | LIBUSB_RECIPIENT_DEVICE | LIBUSB_REQUEST_TYPE_CLASS,
                                      0x81,
                                      0x2d << 8,
                                      0x1 << 8,
                                      data,
                                      size,
                                      TIMEOUT);

    if (ret < 0)
    {
        throw std::runtime_error("Unable to read firmware version. Libusb returned " + std::to_string(ret) + ".");
    }
    else
    {
        i = data[0];
    }
    return i;
}


std::string Usb2Camera::get_firmware_version_string ()
{
    return std::to_string(get_firmware_version());
}


int Usb2Camera::delete_firmware (std::function<void(int)> progress)
{
    return -1;
}


int Usb2Camera::usbbuffer_to_string (unsigned char* usbbuffer,
                                     int buffer_size,
                                     char* string,
                                     int string_size )
{
    int s = usbbuffer[0];

    if ( s > buffer_size )
    {
        return -1;
    }

    if ( usbbuffer[1] != 0x03 )
    {
        return -1;
    }

    if ( ( (s-2)/2 ) > string_size )
    {
        return -1;
    }

    for (int i = 2; i < s; i+=2 )
    {
        string[ i/2 - 1 ] = usbbuffer[i];
    }

    return 0;
}


int Usb2Camera::string_to_usbbuffer (unsigned char* usbbuffer, int buffer_size, const char* string)
{
    size_t length = strlen((char*) string );
    unsigned int len = length * 2 + 4;

    if ( buffer_size < len )
    {
        return 0;
    }

    usbbuffer[0] = len;
    usbbuffer[1] = 0x03;
    unsigned int i;
    for (i = 0; i < length; i++)
    {
        usbbuffer[ 2 + ( 2 * i ) ] = string[i];
        usbbuffer[ 3 + ( 2 * i ) ] = 0x0;
    }

    usbbuffer[ 2 + ( 2 * i ) ] = 0x0;
    usbbuffer[ 3 + ( 2 * i ) ] = 0x0;


    return len;
}


int Usb2Camera::download_firmware (std::vector<unsigned char>& firmware, std::function<void(int)> progress)
{
    int ret;
    const int len = 64;
    unsigned short addr = 0;
    size_t max = firmware.size();

    ret = libusb_control_transfer( this->dev_handle,
                                   LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
                                   0x2,
                                   0,
                                   0xa1,
                                   (unsigned char*)&addr,
                                   2,
                                   TIMEOUT);
    if (ret < 0)
        throw std::runtime_error("Unable to request data. Libusb returned " + std::to_string(ret) + ".");

    usleep( 2000 );
    for (unsigned int i = 0; i < max; i += len )
    {
        ret = libusb_control_transfer( this->dev_handle,
                                       LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
                                       0x3, // No-Stop Serial Bus Request
                                       0,
                                       0xa0,
                                       firmware.data()+i,
                                       len,
                                       TIMEOUT);
        if (ret < 0)
        {
            throw std::runtime_error("Unable to request data. Libusb returned " + std::to_string(ret) + ".");
        }
        else
        {
            progress( i * 100 / max);
        }
    }
    return 0;
}


int Usb2Camera::upload_firmware_file (unsigned char* data, unsigned int size, std::function<void(int)> progress)
{
    int ret;

    const int len = 32;
    unsigned char buffer[256] = { 0 };

    for (unsigned int i = 0; i < size; i+=len )
    {
        buffer[0] = ( i >> 8 ) & 0xff;
        buffer[1] = i & 0xff;

        memcpy (buffer + 2, data + i, len);

        ret = libusb_control_transfer( this->dev_handle,
                                       LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
                                       0x2,
                                       0,
                                       0xa1,
                                       buffer,
                                       len + 2,
                                       TIMEOUT);
        if ( ret < 0 )
        {
            throw std::runtime_error("Unable to request data. Libusb returned " + std::to_string(ret) + ".");
        }
        if ( ( i % 1024 ) == 0 )
        {
            progress(i/size);
        }


        ret = libusb_control_transfer( this->dev_handle,
                                       LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
                                       0x2, // No-Stop Serial Bus Request
                                       0,
                                       0xa0,
                                       buffer,
                                       len,
                                       TIMEOUT);
        if ( ret < 0 )
        {
            throw std::runtime_error("Unable to request data. Libusb returned " + std::to_string(ret) + ".");
        }

        usleep( 5000 );
    }

    return ret;
}


char* Usb2Camera::read_string (int index)
{
    unsigned char usb_buffer[255];
    static char string[255];
    int strsize = 255;
    int addr;
    int len;

    read_eeprom(0x60 + ( index * 2 ), usb_buffer, 2 );

    addr = usb_buffer[0] + 0x54;

    len = usb_buffer[1];

    read_eeprom(addr, usb_buffer, len );
    usbbuffer_to_string( usb_buffer, len, string, strsize );

    return string;
}


void Usb2Camera::get_strings (char* strvendor,
                              char* strproduct,
                              char* strserial,
                              int bufsize )
{
    libusb_device_descriptor usbdev;
    int ret = libusb_get_device_descriptor(libusb_get_device(this->dev_handle), &usbdev);

    if (ret < 0)
    {
        throw std::runtime_error("Unable to retrieve descrptor. Libusb returned " + std::to_string(ret) + ".");
    }

    libusb_get_string_descriptor_ascii( this->dev_handle, usbdev.iManufacturer,(unsigned char*) strvendor, bufsize );
    libusb_get_string_descriptor_ascii( this->dev_handle, usbdev.iProduct, (unsigned char*)strproduct, bufsize );
    libusb_get_string_descriptor_ascii( this->dev_handle, usbdev.iSerialNumber, (unsigned char*)strserial, bufsize );
}


char* Usb2Camera::read_string_from_image ( unsigned char* image, int index )
{
    static char static_buffer[255];

    int addr = image[0x60 + index * 2] + 0x54;
    int len = image[0x60 + index * 2 + 1];

    usbbuffer_to_string( image + addr, len, static_buffer, sizeof(static_buffer) );

    return static_buffer;
}

bool Usb2Camera::apply_strings ( const char** strings, unsigned char* eeprom_image )
{
    int pos0 = 0xbe;
    int len0 = string_to_usbbuffer( eeprom_image + pos0, 512 - pos0, strings[0] );
    int pos1 = pos0 + len0;
    int len1 = string_to_usbbuffer( eeprom_image + pos1, 512 - pos1, strings[1] );
    int pos2 = pos1 + len1;
    int len2 = string_to_usbbuffer( eeprom_image + pos2, 512 - pos2, strings[2] );

    if( !len0 || !len1 || !len2 )
    {
        return false;
    }

    eeprom_image[0x60] = pos0 - 0x54;
    eeprom_image[0x61] = len0;
    eeprom_image[0x62] = pos1 - 0x54;
    eeprom_image[0x63] = len1;
    eeprom_image[0x64] = pos2 - 0x54;
    eeprom_image[0x65] = len2;

    return true;
}

bool Usb2Camera::patch_strings (const char* model,
                                const char* vendor,
                                const char* serial,
                                unsigned char* eeprom_image)
{
    const char* strings[3] = { 0 };

    for (unsigned int i = 0; i < 3; ++i )
    {
        char* str = read_string_from_image( eeprom_image, i );
        if( !strcmp( str, "vv" ) )
        {
            strings[i] = vendor;
        }
        else if( !strcmp( str, "pp") )
        {
            strings[i] = model;
        }
        else if( !strcmp( str, "123") )
        {
            strings[i] = serial;
        }
    }

    for (unsigned int i = 0; i < 3; ++i )
    {
        if( !strings[i] ) return false;
    }

    return apply_strings( strings, eeprom_image );

}


bool Usb2Camera::upload_firmware (const std::string& firmware_package,
                                  const std::string& firmware,
                                  std::function<void(int)> progress)
{
    // This function loads a single firmware description
    // patches the file with the corresponding camera specific values
    // uploads the image
    // and then downloads it to compare it with the existing one
    // to assure it was correctly transferred

    if (!is_valid_firmware_file(firmware_package))
    {
        throw std::runtime_error("Not a valid firmware file!");
    }

    auto fw = load_file(firmware_package);

    if (fw.empty())
    {
        throw std::runtime_error("Retrieved empty firmware file.");
    }

    unsigned int eeprom_size = this->get_eeprom_size();

    std::cout << "Firmware Size: " << fw.size() << " EEPROM Size: " << eeprom_size << std::endl;

    if ((fw.size() + 512) > eeprom_size)
    {
        std::cerr << "Firmware does not fit in EEPROM. Aborting!" << std::endl;
        return false;
    }

    char vendor[128];
    char model[128];
    char serial[128];
    get_strings(vendor, model, serial, 128);

    auto map_progress = [] ( std::function<void(int)> progress, int begin, int end )
    {
        return [=]( int x )
        {
            progress( begin + x * (end-begin) / 100 );
        };
    };


    patch_strings(model, vendor, serial, &fw[0]);

    upload_firmware_file(&fw[0], fw.size(), map_progress(progress, 0, 49));

    std::vector<unsigned char> buffer (fw.size());

    try
    {
        download_firmware(buffer, map_progress(progress, 50, 90));
    }
    catch (std::runtime_error& err)
    {
        throw err;
    }

    if (fw == buffer)
    {
        progress(100);
        return true;
    }

    progress(0);
    return false;
}


int Usb2Camera::set_mode (UVC_COMPLIANCE mode)
{
    unsigned short addr = FLAGS_LOCATION;
    unsigned char buffer[3];
    buffer[0] = ( addr >> 8 ) & 0xff;
    buffer[1] = addr & 0xff;

    unsigned short pid;
    int ret = get_productid(pid);

    // we change the product id to ensure that a user visible change has happened so
    // it can easily be determined if UVC mode is active or not
    // this also causes possible TIS windows drivers to not use the camera

    if (mode == CAMERA_INTERFACE_MODE_UVC)
    {
        pid = (0x83 << 8) | (pid & 0xff);
        ret = set_productid(pid);
        buffer[2] = 1 & 0xff;
    }
    else if (mode == CAMERA_INTERFACE_MODE_PROPRIETARY)
    {
        pid = (0x82 << 8) | (pid & 0xff);
        ret = set_productid(pid);
        buffer[2] = 0 & 0xff;
    }
    usleep (10000);
    ret = libusb_control_transfer(this->dev_handle,
                                  LIBUSB_ENDPOINT_OUT | LIBUSB_RECIPIENT_DEVICE | LIBUSB_REQUEST_TYPE_VENDOR,
                                  0x2,
                                  0,
                                  0xa1,
                                  (unsigned char*)&addr,
                                  2,
                                  TIMEOUT);

    if (ret < 0)
    {
        throw std::runtime_error("Error while setting mode.");
    }

    ret = libusb_control_transfer(this->dev_handle,
                                  LIBUSB_ENDPOINT_OUT | LIBUSB_RECIPIENT_DEVICE | LIBUSB_REQUEST_TYPE_VENDOR,
                                  0x2,
                                  0,
                                  0xa1,
                                  buffer,
                                  3,
                                  TIMEOUT);

    return ret;
}


UVC_COMPLIANCE Usb2Camera::get_mode ()
{
    unsigned char mode;

    try
    {
        read_eeprom(FLAGS_LOCATION, &mode, 1);
    }
    catch (std::runtime_error& err)
    {
        throw;
    }

    if ( mode & 0x1)
        return CAMERA_INTERFACE_MODE_UVC;
    else
        return CAMERA_INTERFACE_MODE_PROPRIETARY;

}

unsigned int Usb2Camera::get_eeprom_size ()
{
    unsigned char bufa[64];
    unsigned char bufb[64];

    if (!read_eeprom (0, bufa, sizeof(bufa)))
    {
        return 0;
    }
    if (!read_eeprom (16384, bufb, sizeof(bufb)))
    {
        return 0;
    }
    if (!memcmp(bufa, bufb, sizeof(bufa)))
    {
        return 16384;
    }

    return 32768;
}

int Usb2Camera::write_eeprom (unsigned int addr, unsigned char* data, unsigned int size)
{
    int ret = 0;
    unsigned char buffer[256] = { 0 };
    int len = ( size < 32 ) ? size : 32;
    unsigned int i;
    for (i = 0; (i + len) < size; i += len )
    {
        buffer[0] = ( (addr+i) >> 8 ) & 0xff;
        buffer[1] = (addr+i) & 0xff;

        memcpy( buffer + 2, data + i, len );

        ret = libusb_control_transfer(this->dev_handle,
                                      LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
                                      0x2,
                                      0,
                                      0xa1,
                                      buffer,
                                      len + 2,
                                      TIMEOUT);
        if( ret < 0 )
        {
            throw std::runtime_error("Error while requesting write permission. Libusb returned "
                                     + std::to_string(ret) + ".");
        }

        ret = libusb_control_transfer(this->dev_handle,
                                      LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
                                      0x2, // No-Stop Serial Bus Request
                                      0,
                                      0xa0,
                                      buffer,
                                      len,
                                      TIMEOUT);
        if ( ret < 0 )
        {
            throw std::runtime_error("Unable to request data. Libusb returned " + std::to_string(ret) + ".");
        }

        usleep( 5000 );
    }

    if ( (ret>=0) && ( i < (size ) ) )
    {
        len = (size-1) - i;

        buffer[0] = ( (addr+i) >> 8 ) & 0xff;
        buffer[1] = (addr+i) & 0xff;

        memcpy( buffer + 2, data + i, len );

        ret = libusb_control_transfer(this->dev_handle,
                                      LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
                                      0x2,
                                      0,
                                      0xa1,
                                      buffer,
                                      len + 2,
                                      TIMEOUT);
        if ( ret < 0 )
        {
            throw std::runtime_error("Unable to request data. Libusb returned " + std::to_string(ret) + ".");
        }

        ret = libusb_control_transfer(this->dev_handle,
                                      LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
                                      0x2, // No-Stop Serial Bus Request
                                      0,
                                      0xa0,
                                      buffer,
                                      len,
                                      TIMEOUT);
        if (ret < 0)
        {
            throw std::runtime_error("Unable to request data. Libusb returned " + std::to_string(ret) + ".");
        }
    }

    return ret;
}


int Usb2Camera::read_eeprom (unsigned int addr, unsigned char* data, unsigned int size)
{
    unsigned char tmp[2];
    const int len = (64 < size) ? 64: size;
    int ret;

    memset( data, 0x0, size );

    tmp[0] = ( addr >> 8 ) & 0xff;
    tmp[1] = addr & 0xff;

    ret = libusb_control_transfer(this->dev_handle,
                                  LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
                                  0x3,
                                  0,
                                  0xa1,
                                  tmp,
                                  2,
                                  TIMEOUT);
    if (ret < 0)
    {
        throw std::runtime_error("Unable to request data. Libusb returned " + std::to_string(ret) + ".");
    }

    usleep( 2000 );

    for (unsigned int i = 0; i < size; i += len )
    {
        ret = libusb_control_transfer(this->dev_handle,
                                      LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
                                      0x2,
                                      0,
                                      0xa0,
                                      (unsigned char*)data+i,
                                      len,
                                      TIMEOUT);
        if ( ret < 0 )
        {
            throw std::runtime_error("Error while reading data. Libusb returned " + std::to_string(ret) + ".");
        }
    }

    return ret;
}


int Usb2Camera::get_productid (unsigned short& pid)
{
    unsigned short addr = PID_LOCATION_LO;
    int ret;
    unsigned char buffer[2];

    ret = read_eeprom (addr, buffer, 2);
    pid = buffer[0] | buffer[1] << 8;

    return ret;
}


int Usb2Camera::set_productid (unsigned short pid)
{
    unsigned short addr = PID_LOCATION_LO;
    int ret;
    unsigned char buffer[4];
    buffer[0] = ( addr >> 8 ) & 0xff;
    buffer[1] = addr & 0xff;
    buffer[2] = pid & 0xff;
    buffer[3] = (pid >> 8) & 0xff;

    ret = libusb_control_transfer(this->dev_handle,
                                  LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
                                  0x2,
                                  0,
                                  0xa1,
                                  buffer,
                                  sizeof(buffer),
                                  TIMEOUT);

    return ret;
}

}; /* namespace tis */
