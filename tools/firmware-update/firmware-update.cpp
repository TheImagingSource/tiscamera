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

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <getopt.h>
#include <exception>
#include <stdexcept>

using namespace tis;

void usage (const std::string& program_name)
{
    std::cout << std::endl
              << program_name << " - Firmware update tool for USB cameras" << std::endl
              << std::endl
              << "usage: " << program_name << " [-l | --list]" << std::endl
              << "usage: " << program_name << " [-i | --info] [-d serial]" << std::endl
              << "usage: " << program_name << " [-u | --upload] [-d serial] [-f | --file file]" << std::endl
              << "usage: " << program_name << " [-d serial] [-m | --mode {u|uvc|p|proprietary}]" << std::endl
              << std::endl
              << "Options:" << std::endl
              << "  --list\t-l\tlist cameras that can be accessed" << std::endl
              << "  --device\t-d\tcamera that shall be interacted with" << std::endl
              << "  --info\t-i\tprint information about the specified camera" << std::endl
              << "  --upload\t-u\tupload firmware" << std::endl
              << "  --mode\t-m\tswitch operating mode of camera (uvc/proprietary) (only USB2 cameras)" << std::endl
              << "  --file\t-f\tfirmware file to use" << std::endl
              << std::endl;
}


void list_devices ()
{
    UsbHandler usb;

    std::vector<device_info> devs;
    try
    {
         devs = usb.get_device_list();
    }
    catch (const std::runtime_error& err)
    {
        std::cout << err.what() << std::endl;

        exit(1);
    }
    unsigned short name_width = 14;
    unsigned short id_width = 9;
    unsigned short serial_width = 12;

    std::cout << std::endl << std::left
              << "Found " << devs.size() << " device(s)." << std::endl << std::endl
              << std::setw(name_width)   << "Name" << " - "
              << std::setw(id_width)     << "ID " << " - "
              << std::setw(serial_width) << "Serialnumber" << std::endl;

    for (auto& d : devs)
    {
        std::cout << std::setw(name_width) << d.product
                  << " - " << std::hex << d.idVendor << ":" << d.idProduct << std::dec
                  << " - " << d.serial<< std::endl;
    }
    std::cout << std::endl;

}


// query user for strings that shall be used in the camera
// the values from the parameters will be displayed as defaults
void query_device_strings (std::string& vendor, std::string& device, std::string& serial)
{
    std::string input;

    std::cout << "Please insert the following strings that shall be used as camera descriptions." << std::endl
              << "Vendor Description [" << vendor << "]: ";
    std::getline(std::cin, input);

    if (!input.empty())
    {
        vendor = input;
    }

    std::cout << "Device Description [" << device << "]: ";
    std::getline(std::cin, input);
    if (!input.empty())
    {
        device = input;
    }

    std::cout << "Device Description [" << serial << "]: ";
    std::getline(std::cin, input);
    if (!input.empty())
    {
        serial = input;
    }

    std::cout << std::endl
              << "Will use the following strings for the selected device:" << std::endl
              << std::endl
              << "Vendor: " << vendor << std::endl
              << "Device: " << device << std::endl
              << "Serial: " << serial << std::endl << std::endl;
    std::cout << "Do you really want to write these strings?[y/N] ";

    std::string confirm;
    while (1)
    {
        std::getline(std::cin, confirm);
        if (confirm.compare("y") == 0 || confirm.compare("Y") == 0)
        {
            std::cout << "Applying strings" <<std::endl;
            break;
        }
        else if (confirm.empty() || confirm.compare("n") || confirm.compare("N"))
        {
            std::cout << "Aborting..." << std::endl << std::endl;
            break;
        }
        else
        {
            std::cout << "Please answer yes or no." << std::endl;
        }
    }
}


void print_device_info (const std::string& serial_number)
{
    UsbHandler usb;

    auto cam = usb.open_camera(serial_number);

    if (cam == NULL)
    {
        std::cerr << "Unable to find device with serial \""
                  << serial_number << "\"" << std::endl;
        return;
    }

    auto d = cam->get_device_info();

    std::cout << std::endl
              << "Device manufacturer: " << d.manufacturer << std::endl
              << "Product name:        " << d.product << std::endl
              << "Serial number:       " << d.serial << std::endl
              << "VendorID:ProductID:  " << std::hex << d.idVendor << ":" << d.idProduct << std::dec << std::endl;

    if (cam->get_firmware_version() != -1)
    {
        std::cout << "Firmware version:    " << cam->get_firmware_version() << std::endl;
    }
    else
    {
        std::cout << "Firmware version:    " << cam->get_firmware_version_string() << std::endl;
    }

    auto type = cam->get_camera_type();

    if (type.camera_type == USB2)
    {
        UVC_COMPLIANCE mode = cam->get_mode();

        std::cout << "UVC mode is:         ";
        if (mode == CAMERA_INTERFACE_MODE_UVC)
            std::cout << "on" << std::endl;
        else
            std::cout << "off" << std::endl;

        std::cout << "Camera EEPROM size:  ";
        std::cout << cam->get_eeprom_size();
        std::cout << std::endl;
    }
    else if (type.camera_type == USB3 && cam->get_firmware_version() < 102)
    {
        std::cout << "\n!!! FIRMWARE UPGRADE REQUIRED !!!\n\n";
        std::cout << "To correctly interact with this camera under Linux\n";
        std::cout << "this device requires a firmware upgrade.\n\n";
        std::cout << "Please contact the manufacturer to receive the concerning firmware files." << std::endl;
    }

    std::cout << std::endl;
}


// toggle uvc mode in USB2 cameras
// USB3 cameras do net have this "feature" and have to be ignored
void set_device_mode (const std::string& serial_number, const std::string& mode)
{
    UsbHandler usb;

    auto cam = usb.open_camera(serial_number);

    if (cam == NULL)
    {
        std::cerr << "Unable to find device with serial \""
                  << serial_number << "\"" << std::endl;
        return;
    }
    auto type = cam->get_camera_type();

    if (type.camera_type != USB2)
    {
        std::cerr << "Mode settings only available for USB2 cameras." << std::endl;
        exit(69);
    }

    int ret = -1;
    if (mode.compare("uvc") == 0 || mode.compare("u") == 0)
    {
        std::cout << std::endl << "Setting camera to UVC mode ... ";
        ret = cam->set_mode(CAMERA_INTERFACE_MODE_UVC);
    }
    else if (mode.compare("proprietary") == 0 || mode.compare("p") == 0)
    {
        std::cout << std::endl << "Setting camera to PROPRIETARY mode ... ";
        ret = cam->set_mode(CAMERA_INTERFACE_MODE_PROPRIETARY);
    }
    else
    {
        std::cerr << std::endl
                  << "Unknown mode identifier \"" << mode << "\"." << std::endl
                  << "Please try again with a valid one." << std::endl
                  << "Allowed ones are: 'uvc' and 'proprietary'." << std::endl << std::endl;
        return;
    }

    if (ret >= 0)
    {
        std::cout << "SUCCESS" << std::endl << std::endl;
    }
    else
    {
        std::cout << "FAILED - " << ret << std::endl << std::endl;
    }
}


void upload_to_device (const std::string& serial_number,
                       const std::string& firmware,
                       std::string model = "",
                       bool assume_yes = false)
{
    UsbHandler usb;

    auto cam = usb.open_camera(serial_number);

    if (cam == NULL)
    {
        std::cerr << "Unable to find device with serial \""
                  << serial_number << "\"" << std::endl;
        return;
    }
    std::cout << std::endl;

    if (!assume_yes)
    {
        std::cout << "!!! Attention !!!" << std::endl
                  << "This action could break your camera." << std::endl << std::endl
                  << "Do you really want to proceed? [y/N] ";

        while (1)
        {
            std::string s;
            std::getline(std::cin, s);
            if (s.compare("y") == 0 || s.compare("Y") == 0 )
                break;
            else if (s.empty() || s.compare("n") == 0 || s.compare("N") == 0 )
            {
                std::cout << "Aborting..." << std::endl;
                return;
            }
            else
            {
                std::cout << "Please answer yes or no." << std::endl;
            }
        }
    }

    auto func = [] (int progress)
        {
            std::cout << "\r    " << progress << " %";

            std::cout.flush();
        };

    device_info dev = cam->get_device_info();

    // automatically determine which fw to write
    if (model.empty() && cam->get_camera_type().camera_type != USB33)
    {
        std::stringstream sstream;
        sstream << std::hex << dev.idProduct;

        model = sstream.str() + ".fw";
    }

    bool success = false;
    try
    {
        success = cam->upload_firmware(firmware, model, func);
    }
    catch (const std::runtime_error& err)
    {
        std::cout << std::endl << "There was a mistake. " << std::endl
                  << err.what() << std::endl;
    }

    if (success)
    {
        std::cout << std::endl << std::endl
                  << "Upload successful!" << std::endl;

        if (dev.idVendor != 0x04b4)
        {
            std::cout << "Please reconnect your camera."<< std::endl
                      << std::endl;
        }
        else
        {
            std::cout << std::endl;
        }
    }
    else
    {
        std::cout << std::endl
                  << "Firmware update not successful..." << std::endl
                  << "DO NOT DISCONNECT YOUR CAMERA!" << std::endl
                  << "Please try again." << std::endl
                  << std::endl;
    }
}


void delete_firmware (std::string& serial_number)
{
    UsbHandler usb;

    auto cam = usb.open_camera(serial_number);

    if (cam == NULL)
    {
        std::cerr << "Unable to find device with serial \""
                  << serial_number << "\"" << std::endl;
        return;
    }
    std::cout << std::endl;

    auto func = [] (int progress)
        {
            std::cout << "\r    " << progress << " %";

            std::cout.flush();
        };

    cam->delete_firmware(func);
    std::cout << std::endl;

}


int main (int argc, char *argv[])
{
    std::string program_name(argv[0]);
    std::string firmware_file = "";
    std::string serial_number = "";
    std::string mode_string = "";
    std::string firmware_model = "";
    bool assume_yes = false;

    enum mode
    {
        UNKNOWN =0,
        LIST,
        DETAILS,
        MODE,
        UPLOAD,
        DELETE,
    };

    mode active_mode = UNKNOWN;

    static struct option long_options[] =
        {
            {"help",   no_argument,       0, 'h'},
            {"device", required_argument, 0, 'd'},
            {"list",   no_argument,       0, 'l'},
            {"upload", required_argument, 0, 'u'},
            {"mode",   required_argument, 0, 'm'},
            {"file",   required_argument, 0, 'f'},
            {"model",  required_argument, 0, 'o'},
            {"set",    no_argument,       0, 's'},
            {"yes",    no_argument,       0, 'y'},
            {"delete", no_argument,       0, '9'},
            {0,        0,                 0, 0}
        };

    while (1)
    {
        int option_index = 0;

        int c = getopt_long (argc, argv, "uyd:lim:f:s:o:h",
                             long_options, &option_index);

        if (c == -1)
            break;

        switch (c)
        {
            case 0:
                break;
            case 'h':
                usage(program_name);
                return 0;
            case 'd':
                serial_number = optarg;
                break;
            case 'l':
                active_mode = LIST;
                list_devices();
                break;
            case 'u':
                active_mode = UPLOAD;
                break;
            case 'm':
                active_mode = MODE;
                mode_string = optarg;
                break;
            case 'f':
                firmware_file = optarg;
                break;
            case 'o':
                firmware_model = optarg;
                break;
            case 'i':
                active_mode = DETAILS;
                break;
            case 's':
                break;
            case 'y':
                assume_yes = true;
                break;
            case '9':
                active_mode = DELETE;
                break;
            default:
                break;
        }
    }

    if (active_mode == UNKNOWN)
    {
        usage(program_name);
        return 1;
    }

    if (active_mode == UPLOAD && firmware_file.empty())
    {
        std::cerr << "You need to specify the firmware to use!" << std::endl;
        return 64;
    }

    if ((active_mode == UPLOAD || active_mode == DETAILS || active_mode == MODE) && serial_number.empty())
    {
        std::cerr << "Missing camera identification. Please specify which camera shall be used." << std::endl;
        return 64;
    }

    if (active_mode == DETAILS)
    {
        print_device_info(serial_number);
    }
    else if (active_mode == MODE)
    {
        if (serial_number.empty() || mode_string.empty())
        {
            std::cerr << "Please supply all information needed." << std::endl;
            return 69;
        }

        set_device_mode(serial_number, mode_string);
    }
    else if (active_mode == UPLOAD)
    {
        upload_to_device(serial_number, firmware_file, firmware_model, assume_yes);
    }
    else if (active_mode == DELETE)
    {
        delete_firmware(serial_number);
    }

    return 0;
}
