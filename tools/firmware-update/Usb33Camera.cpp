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

#include "Usb33Camera.h"

#include "33u/Firmware.h"
#include "33u/ReportProgress.h"

namespace tis
{

StdOutput::StdOutput (const std::function<void(int)>& callback)
    : progress(callback)
{}

void StdOutput::report_percentage (int pct)
{
    progress(pct);
}


void StdOutput::report_group (const std::string & msg)
{}


void StdOutput::report_step (const std::string & msg)
{}


void StdOutput::report_speed (float speed, const std::string & unit)
{}


Usb33Camera::Usb33Camera (std::shared_ptr<UsbSession> session,
                          device_info dev,
                          unsigned int _interface):
    UsbCamera(session, dev, _interface),
    device(std::make_shared<lib33u::driver_interface::libusb::ShowDevice>(session, dev)),
    cam(device)
{
    device->open();
}


Usb33Camera::~Usb33Camera ()
{}


int Usb33Camera::get_firmware_version ()
{
    return -1;
}


std::string Usb33Camera::get_firmware_version_string ()
{
    return cam.fpga_version();
}


int Usb33Camera::delete_firmware (std::function<void(int)> progress)
{
    return -1;
}


int Usb33Camera::download_firmware (std::vector<unsigned char>& firmware,
                                    std::function<void(int)> progress)
{
    return -1;
}


bool Usb33Camera::upload_firmware (const std::string& firmware_package,
                                   const std::string& firmware,
                                   std::function<void(int)> progress)
{

    auto fw = lib33u::Firmware::load_package(firmware_package);

    StdOutput out(progress);

    if (!firmware.empty())
    {
        auto types = fw.device_types();
        lib33u::firmware_update::DeviceTypeDesc type_desc;
        for (auto& t : types)
        {
            if (t.description.compare(firmware))
            {
                type_desc = t;
                break;
            }
        }
        fw.upload(cam, out, type_desc);
    }
    else
    {
        fw.upload(cam, out);
    }

    return true;
}


UVC_COMPLIANCE Usb33Camera::get_mode ()
{
    return UVC_COMPLIANCE::CAMERA_INTERFACE_MODE_UVC;
}


unsigned int Usb33Camera::get_eeprom_size ()
{
    return 0;
}


int Usb33Camera::set_mode (UVC_COMPLIANCE /* mode */)
{
    return -1;
}


int Usb33Camera::write_eeprom (unsigned int addr,
                               unsigned char* data,
                               unsigned int size)
{
    return -1;
}


int Usb33Camera::read_eeprom (unsigned int addr,
                              unsigned char* data,
                              unsigned int size)
{
    return -1;
}


int Usb33Camera::erase_sector (unsigned int addr)
{
    return -1;
}


int Usb33Camera::erase_eeprom (std::function<void(int)> progress)
{
    return -1;
}


bool Usb33Camera::initialize_eeprom (std::vector<uint8_t>& firmware)
{
    return false;
}


int Usb33Camera::upload_firmware_file (std::vector<uint8_t> firmware,
                                       std::function<void(int)> progress)
{
    return -1;
}

} /* namespace tis */
