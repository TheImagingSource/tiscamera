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

#include <tcamprop.h>

#include <iostream>
#include <iomanip>
#include <unistd.h>
 #include <sys/stat.h>

#include <gst/gst.h>

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

    gst_init(&argc, &argv);

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

    TCAM_DEVICE_TYPE t;

    // helper function to handle 12345678-backend situations
    auto separate_serial_and_type = [&serial, &t] (std::string input)
    {
        auto pos = input.find("-");

        if (pos != std::string::npos)
        {
            // assign to tmp variables
            // input could be self->device_serial
            // overwriting it would ivalidate input for
            // device_type retrieval
            std::string tmp1 = input.substr(0, pos);
            std::string tmp2 = input.substr(pos+1);

            serial = tmp1;
            t = tcam::tcam_device_from_string(tmp2);
        }
        else
        {
            serial = input;
            t = TCAM_DEVICE_TYPE_UNKNOWN;
        }
    };

    separate_serial_and_type(serial);

    if (t == TCAM_DEVICE_TYPE_UNKNOWN)
    {
        t = tcam::tcam_device_from_string(device_type);
    }

    auto string_is_valid = [](const std::string &str)
                           {
                               // user serial are generally only numbers
                               // developer and special devices may contain letters
                               auto is_illegal_char = [](const char c)
                                                      {
                                                          if ((c <= 'z' && c >= 'a')
                                                              || (c <= 'Z' && c >= 'A')
                                                              || isdigit(c))
                                                              return false;
                                                          return true;
                                                      };
                               return find_if(str.begin(), str.end(), is_illegal_char) == str.end();
                           };

    if (!string_is_valid(serial))
    {
        std::cerr << "'" << serial << "' is not a valid serial number" << std::endl;
        return 1;
    }

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
        print_properties(serial);
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
        std::cout << "Loading: " << vec.at(0) << std::endl;

        // TODO: replace with std::filesystem once c++17 is
        // available on reference system

        std::string json_str;

        struct stat sb;

        if (stat(vec.at(0).c_str(), &sb) == 0 && S_ISREG(sb.st_mode)) // can be open && is regular file
        {
            std::ifstream ifs(vec.at(0));
            json_str = std::string((std::istreambuf_iterator<char>(ifs)),
                                   (std::istreambuf_iterator<char>()   ));

        }
        else // string itself is json
        {
            json_str = vec.at(0);
        }
        std::pair<bool, std::vector<std::string>> ret = load_json_state(dev, json_str);

        for (const auto& msg : ret.second)
        {
            std::cout << msg << std::endl;
        }
        if (!ret.first)
        {
            return 1;
        }
    }

    return 0;
}
