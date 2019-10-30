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

#include <tcam.h>

#include <iostream>
#include <iomanip>
#include <unistd.h>

#include <CLI11/CLI11.hpp>

using namespace tcam;



void print_version (size_t /*t*/)
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


void print_devices (size_t /*t*/)
{
    DeviceIndex index;
    std::vector<DeviceInfo> device_list = index.get_device_list();

    print_capture_devices(device_list);
}


int main (int argc, char *argv[])
{

    CLI::App app {"Commandline camera manipulation utility."};

    auto show_version = app.add_flag_function("--version", print_version, "list used library versions");
    auto list_devices = app.add_flag_function("-l,--list", print_devices, "list capture devices");

    std::string serial;

    auto show_caps = app.add_option("-c,--caps", serial,
                                    "list available gstreamer-1.0 caps");
    auto show_formats = app.add_option("-f,--formats", serial,
                                       "list available formats");
    auto show_properties = app.add_option("-p,--properties", serial,
                                          "list available device properties");

    auto save_state = app.add_option("--save", serial,
                                     "Print a JSON string containing all properties and their current values");
    std::string state;
    auto load_state = app.add_option("--load", serial,
                                     "Read a JSON string containing properties and their values and set them in the device");

    std::string device_type;
    auto existing_device_types = tcam::get_device_type_list_strings();
    std::set<std::string> s(existing_device_types.begin(),existing_device_types.end());

    app.add_set("-t,--type", device_type, s,
                "camera type", "unknown");

    list_devices->excludes(show_caps);
    list_devices->excludes(show_formats);
    list_devices->excludes(show_properties);
    show_properties->excludes(show_caps);
    show_properties->excludes(show_formats);

    // CLI11 uses "TEXT" as a filler for the option string arguments
    // replace it with "SERIAL" to make the help text more intuitive.
    app.get_formatter()->label("TEXT", "SERIAL");

    app.allow_extras();

    CLI11_PARSE(app, argc, argv);

    if (*list_devices || *show_version)
    {
        return 0;
    }


    auto t = tcam::tcam_device_from_string(device_type);

    auto dev = open_device(serial, t);

    if (!dev)
    {
        std::cerr << "Unable to open device with serial \"" << serial << "\"." << std::endl;
        return 1;
    }

    if (*show_caps)
    {
        list_gstreamer_1_0_formats(dev->get_available_video_formats());
    }
    else if (*show_formats)
    {
        list_formats(dev->get_available_video_formats());
    }
    else if (*show_properties)
    {
        print_properties(dev->get_available_properties());
    }
    else if (*save_state)
    {
        print_state_json(dev);
    }
    else if (*load_state)
    {
        if (app.remaining_size() != 1)
        {
            std::cerr << "Too many arguments" << std::endl;
        }

        std::vector<std::string> vec = app.remaining();

        load_json_state(dev, vec.at(0));
        // bool state = load_device_settings(TCAM_PROP(self),
        //                                   self->device_serial,
        //                                   g_value_get_string(value));

    }

    return 0;
}
