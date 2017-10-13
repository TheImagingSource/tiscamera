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

#include "general.h"
#include "properties.h"
#include "formats.h"
#include "multimedia.h"

#include <tcam.h>

#include <iostream>
#include <iomanip>
#include <unistd.h>

using namespace tcam;


void print_help (const std::string& prog_name)
{
    std::cout << "Commandline camera manipulation utility." << std::endl
    << std::endl
    << "Options:\n"
    << "\t-l - list cameras\n"
    << "\t-p - list properties\n"
    << "\t-s - set property\n"
    << "\t-f - list video formats\n"
    << "\t-c - list gstreamer-1.0 caps\n"
    << "\t--version - list used library versions\n"
    << "\n"
    << "Examples:\n"
    << "\n"
    << "Set video format:\n"
    << "\t" << prog_name << " -f -s \"format=GRBG,width=1920,height=1080,framerate=15.0\" <SERIAL>\n"
    << "\n"
    << "Set property\n"
    << "\t" << prog_name << " -p -s \"Auto Exposure=false\" <SERIAL>\n"
    << "\n"
    << std::endl;
}


void print_version ()
{
    std::cout << "Versions: "<< std::endl
              << "\tTcam:\t" << get_version() << std::endl
              << "\tAravis:\t" << get_aravis_version() << std::endl;
}


void print_capture_devices (const std::vector<DeviceInfo>& devices)
{
    if (devices.size() == 0)
    {
        std::cout << "No devices found." << std::endl;
    }
    else
    {
        std::cout << "Available devices:" << std::endl;
        std::cout << "Model\t\tType\tSerial" << std::endl << std::endl;
        for (const auto& d : devices)
        {
            std::cout << d.get_name() << "\t" << d.get_device_type_as_string() << "\t" << d.get_serial() << std::endl;
        }
        std::cout << std::endl;
    }
}


enum modes
{
    LIST_PROPERTIES = 0,
    SET_PROPERTY,
    LIST_FORMATS,
    LIST_GST_1_0_FORMATS,
    SET_FORMAT,
    LIST_DEVICES,
    SAVE_STREAM,
    SAVE_IMAGE,
    SAVE_DEVICE_LIST,
    SAVE_DEVICE_SETTINGS,
    LOAD_DEVICE_SETTINGS,
    PRINT_VERSION,
};

enum INTERACTION
{
    GET = 0,
    SET,
};

int main (int argc, char *argv[])
{

    std::string executable = extract_filename(argv[0]);

    if (argc == 1)
    {
        print_help(executable);
        return 0;
    }

    INTERACTION way = GET;
    std::string serial;
    std::string param;
    std::string filename;
    modes do_this;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help")
        {
            print_help(executable);
            return 0;
        }
        else if (arg == "--version")
        {
            print_version();

            return 0;
        }
        else if (arg == "-l" || arg == "--list")
        {
            std::vector<DeviceInfo> device_list = get_device_list();

            print_capture_devices(device_list);
            return 0;
        }
        else if (arg == "-p" || arg == "--list-properties")
        {
            do_this = LIST_PROPERTIES;
        }
        else if (arg == "-s" || arg == "--set")
        {
            way = SET;
            //do_this = SET_PROPERTY;
            int tmp_i = i;
            tmp_i++;
            if (tmp_i >= argc)
            {
                std::cout << "--set requires additional arguments to work properly!" << std::endl;
                return 1;
            }
            param = argv[tmp_i];
            i++;

        }
        else if (arg == "-f" || arg == "--format")
        {
            do_this = LIST_FORMATS;
        }
        else if (arg == "-c" || arg == "--caps")
        {
            do_this = LIST_GST_1_0_FORMATS;
        }
        else
        {
            serial = arg;
        }
    }

    if (serial.empty())
    {
        std::cout << "No serial given!" << std::endl;
        return 1;
    }

    auto dev = open_device(serial);

    if (!dev)
    {
        std::cerr << "Unable to open device with serial \"" << serial << "\"." << std::endl;
        return 1;
    }

    switch (do_this)
    {
        case LIST_PROPERTIES:
        {
            if (way == GET)
            {
                print_properties(dev->get_available_properties());
            }
            else
            {
                set_property(dev, param);
            }
            break;
        }
        case LIST_FORMATS:
        {
            if (way == GET)
            {
                list_formats(dev->get_available_video_formats());
            }
            else
            {
                set_active_format(dev, param);
            }
            break;
        }
        case LIST_GST_1_0_FORMATS:
        {
            list_gstreamer_1_0_formats(dev->get_available_video_formats());
            break;
        }
        case SET_PROPERTY:
        {
            set_property(dev, param);
            break;
        }
        case SET_FORMAT:
        {
            set_active_format(dev, param);
            break;
        }
        case SAVE_STREAM:
        {
            //save_stream(g, filename);
        }
        case SAVE_IMAGE:
        {
            //save_image(g, filename);
            break;
        }
        case SAVE_DEVICE_LIST:
        case SAVE_DEVICE_SETTINGS:
        case LOAD_DEVICE_SETTINGS:
        default:
        {
            std::cout << "Unknown command." << std::endl;
            break;
        }
    }

    return 0;
}
