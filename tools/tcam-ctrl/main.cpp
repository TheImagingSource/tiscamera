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

#include "../../src/public_utils.h"
#include "../../src/version.h"
#include "formats.h"
#include "general.h"
#include "properties.h"
#include "system.h"

#include <CLI11.hpp>
#include <gst/gst.h>
#include <iomanip>
#include <iostream>
#include <sys/stat.h>
#include <tcam-property-1.0.h>
#include <unistd.h>

using namespace tcam::tools::ctrl;

namespace
{

static void print_version(size_t /*t*/)
{
    std::cout << "Versions: " << std::endl
              << "\tTcam:\t" << get_version() << std::endl
              << "\tAravis:\t" << get_aravis_version() << std::endl
              << "\tModules:\t" << get_enabled_modules() << std::endl;
}


static void print_devices(size_t /*t*/)
{
    auto monitor = gst_device_monitor_new();

    gst_device_monitor_add_filter(monitor, "Video/Source/tcam", NULL);

    GList* devices = gst_device_monitor_get_devices(monitor);

    for (GList* dev = devices; dev; dev = dev->next)
    {
        GstStructure* struc = gst_device_get_properties(GST_DEVICE(dev->data));

        printf("Model: %s Serial: %s Type: %s\n",
               gst_structure_get_string(struc, "model"),
               gst_structure_get_string(struc, "serial"),
               gst_structure_get_string(struc, "type"));
    }

    g_list_free_full(devices, gst_object_unref);
    gst_object_unref(monitor);
}


static void print_serials_long(size_t)
{
    auto monitor = gst_device_monitor_new();

    gst_device_monitor_add_filter(monitor, "Video/Source/tcam", NULL);

    GList* devices = gst_device_monitor_get_devices(monitor);
    std::string out;
    for (GList* dev = devices; dev; dev = dev->next)
    {
        GstStructure* struc = gst_device_get_properties(GST_DEVICE(dev->data));

        out += gst_structure_get_string(struc, "serial");
        out += "-";
        out += gst_structure_get_string(struc, "type");
        out += " ";
    }
    printf("%s\n", out.c_str());

    g_list_free_full(devices, gst_object_unref);
    gst_object_unref(monitor);
}
} // namespace

int main(int argc, char* argv[])
{

    gst_init(&argc, &argv);

    CLI::App app { "Commandline camera manipulation utility." };

    auto show_version =
        app.add_flag_function("--version", print_version, "list used library versions");
    auto list_devices = app.add_flag_function("-l,--list", print_devices, "list capture devices");
    auto list_serial_long =
        app.add_flag_function("--list-serial-long", print_serials_long, "list capture devices");
    auto packages = app.add_flag_callback(
        "--packages", tcam::tools::print_packages, "list installed TIS packages");
    auto system_info = app.add_flag_callback(
        "--system-info", tcam::tools::print_system_info_general, "list general system information");
    auto gige_info = app.add_flag_callback(
        "--gige-info", tcam::tools::print_system_info_gige, "list network system information");
    auto usb_info = app.add_flag_callback(
        "--usb-info", tcam::tools::print_system_info_usb, "list usb system information");
    auto all_info = app.add_flag_callback(
        "--all-info", tcam::tools::print_system_info, "list all system information");

    std::string serial;

    auto show_caps = app.add_option("-c,--caps", serial, "list available gstreamer-1.0 caps");
    auto show_properties =
        app.add_option("-p,--properties", serial, "list available device properties");

    auto save_properties = app.add_option("--save",
                                          serial,
                                          "Print a GstStructure string containing all properties and their current values");
    auto save_properties_no_console = app.add_flag("--no-console",
                                                   "Output string is not intended for commandline usage.")->needs(save_properties);

    auto load_properties = app.add_option("--load",
                                          serial,
                                          "Read a GstStrcture string containing properties and their "
                                          "values and set them in the device");

    auto save_json = app.add_option(
        "--save-json", serial, "Print a JSON string containing all properties and their current values");

    auto load_json = app.add_option("--load-json",
                                     serial,
                                     "Read a JSON string/file containing properties and their "
                                     "values and set them in the device");

    auto list_transform = app.add_subcommand("--transform", "list format transformations of a GstElement");

    std::string transform_element = "tcamconvert";
    list_transform->add_option("-e,--element", transform_element, "Which transform element to use.", true);


    auto transform_group =list_transform->add_option_group("caps");
    auto list_transform_in = transform_group->add_option("--in", "Caps that go into the transform element");
    auto list_transform_out = transform_group->add_option("--out", "Caps to come out of the transform element");

    list_transform_in->excludes(list_transform_out);

    list_devices->excludes(show_caps);
    list_devices->excludes(show_properties);
    show_properties->excludes(show_caps);

    // CLI11 uses "TEXT" as a filler for the option string arguments
    // replace it with "SERIAL" to make the help text more intuitive.
    app.get_formatter()->label("TEXT", "SERIAL");

    // the help Formatter instance is inherited/shared from the parent (app)
    // We want a separate formatter to have different place holder texts
    // create new Formatter instance as we really only need the text and nothing else
    list_transform->formatter(std::make_shared<CLI::Formatter>());
    list_transform->get_formatter()->label("TEXT", "GstElement");

    app.allow_extras();

    CLI11_PARSE(app, argc, argv);

    if (argc == 1)
    {
        std::cout << app.help() << std::endl;
        return 1;
    }

    // check cb options
    if (*list_devices
        || *show_version
        || *list_devices
        || *list_serial_long
        || *packages
        || *system_info
        || *gige_info
        || *usb_info
        || *all_info)
    {
        return 0;
    }

    if (*show_caps)
    {
        list_gstreamer_1_0_formats(serial);
    }
    else if (*show_properties)
    {
        print_properties(serial);
    }
    else if (*save_properties)
    {
        OutputType print_type = OutputType::ConsoleFirendly;

        if (*save_properties_no_console)
        {
            print_type = OutputType::AsIs;
        }
        print_tcam_properties(serial, print_type);
    }
    else if (*load_properties)
    {
        if (app.remaining_size() != 1)
        {
            std::cerr << "Too many arguments" << std::endl;
        }

        std::vector<std::string> vec = app.remaining();
        std::cout << "Loading: " << vec.at(0) << std::endl;

        // TODO: replace with std::filesystem once c++17 is
        // available on reference system

        std::string prop_str;

        struct stat sb;

        if (stat(vec.at(0).c_str(), &sb) == 0
            && S_ISREG(sb.st_mode)) // can be open && is regular file
        {
            std::ifstream ifs(vec.at(0));
            prop_str = std::string((std::istreambuf_iterator<char>(ifs)),
                                   (std::istreambuf_iterator<char>()));
        }
        else // string itself is json
        {
            prop_str = vec.at(0);
        }

        load_tcam_properties(serial, prop_str);
    }
    else if (*save_json)
    {
        print_state_json(serial);
    }
    else if (*load_json)
    {
        if (app.remaining_size() != 1)
        {
            std::cerr << "Too many arguments" << std::endl;
        }

        std::vector<std::string> vec = app.remaining();
        std::cout << "Loading:\n\n " << std::endl
                  << std::endl
                  << vec.at(0) << "\n\n\n"
                  << std::endl;

        // TODO: replace with std::filesystem once c++17 is
        // available on reference system

        std::string json_str;

        struct stat sb;

        if (stat(vec.at(0).c_str(), &sb) == 0
            && S_ISREG(sb.st_mode)) // can be open && is regular file
        {
            std::ifstream ifs(vec.at(0));
            json_str = std::string((std::istreambuf_iterator<char>(ifs)),
                                   (std::istreambuf_iterator<char>()));
        }
        else // string itself is json
        {
            json_str = vec.at(0);
        }

        load_state_json_string(serial, json_str);
    }
    else if (*list_transform)
    {
        std::string caps_str = "";
        ElementPadDirection direction = ElementPadDirection::Both;

        if (*list_transform_in)
        {
            list_transform_in->results(caps_str);
            direction = ElementPadDirection::In;
        }
        else if (*list_transform_out)
        {
            list_transform_out->results(caps_str);
            direction = ElementPadDirection::Out;
        }

        return convert(transform_element, direction, caps_str);
    }
    else
    {

        std::cerr << "Unknown command" << std::endl << std::endl
         << app.help() << std::endl;
        return 2;
    }
    return 0;
}
