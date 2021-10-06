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

#include "../../libs/tcamprop/src/tcam-property-1.0.h"
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
#include <unistd.h>

static void print_version(size_t /*t*/)
{
    std::cout << "Versions: " << std::endl
              << "\tTcam:\t" << get_version() << std::endl
              << "\tAravis:\t" << get_aravis_version() << std::endl
              << "\tModules:\t" << get_enabled_modules() << std::endl;
}


gboolean bus_function(GstBus* /*bus*/, GstMessage* message, gpointer /*user_data*/)
{
    GstDevice* device;
    //gchar* name;
    switch (GST_MESSAGE_TYPE(message))
    {
        case GST_MESSAGE_DEVICE_ADDED:
        {
            gst_message_parse_device_added(message, &device);

            GstStructure* struc = gst_device_get_properties(device);

            printf("Model: %s Serial: %s Type: %s\n",
                   gst_structure_get_string(struc, "model"),
                   gst_structure_get_string(struc, "serial"),
                   gst_structure_get_string(struc, "type"));

            gst_object_unref(device);
            break;
        }
        case GST_MESSAGE_DEVICE_REMOVED:
        {
            gst_message_parse_device_removed(message, &device);
            //name = gst_device_get_display_name(device);
            //g_print("Device removed: %s\n", name);
            //g_free(name);
            //     Device dev = to_device(device);

            //     self->_mutex.lock();

            //     self->_device_list.erase(std::remove_if(self->_device_list.begin(),
            //                                             self->_device_list.end(),
            //                                             [&dev](const Device& d) {
            //                                                 if (dev == d)
            //                                                 {
            //                                                     return true;
            //                                                 }
            //                                                 return false;
            //                                             }),
            //                              self->_device_list.end());

            //     //        self->_device_list.remove(dev);
            //     self->_mutex.unlock();
            //     emit(self, &Indexer::device_lost, dev);
            //     gst_object_unref(device);
            break;
        }
        default:
        {
            break;
        }
    }

    return true;;
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

    g_list_free(devices);
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

    g_list_free(devices);
    gst_object_unref(monitor);
}


int main(int argc, char* argv[])
{

    gst_init(&argc, &argv);

    CLI::App app { "Commandline camera manipulation utility." };

    auto show_version =
        app.add_flag_function("--version", print_version, "list used library versions");
    auto list_devices = app.add_flag_function("-l,--list", print_devices, "list capture devices");
    app.add_flag_function("--list-serial-long", print_serials_long, "list capture devices");
    app.add_flag_callback("--packages", tcam::tools::print_packages, "list installed TIS packages");
    app.add_flag_callback(
        "--system-info", tcam::tools::print_system_info_general, "list general system information");
    app.add_flag_callback(
        "--gige-info", tcam::tools::print_system_info_gige, "list network system information");
    app.add_flag_callback(
        "--usb-info", tcam::tools::print_system_info_usb, "list usb system information");
    app.add_flag_callback(
        "--all-info", tcam::tools::print_system_info, "list all system information");

    std::string serial;

    auto show_caps = app.add_option("-c,--caps", serial, "list available gstreamer-1.0 caps");
    auto show_properties =
        app.add_option("-p,--properties", serial, "list available device properties");

    auto save_state = app.add_option(
        "--save", serial, "Print a JSON string containing all properties and their current values");
    auto load_state = app.add_option("--load",
                                     serial,
                                     "Read a JSON string/file containing properties and their "
                                     "values and set them in the device");

    std::string device_type;
    auto existing_device_types = tcam::get_device_type_list_strings();
    std::set<std::string> s(existing_device_types.begin(), existing_device_types.end());

    app.add_set("-t,--type", device_type, s, "camera type", "unknown");

    list_devices->excludes(show_caps);
    list_devices->excludes(show_properties);
    show_properties->excludes(show_caps);

    // CLI11 uses "TEXT" as a filler for the option string arguments
    // replace it with "SERIAL" to make the help text more intuitive.
    app.get_formatter()->label("TEXT", "SERIAL");
    app.require_option();
    app.allow_extras();

    CLI11_PARSE(app, argc, argv);

    if (*list_devices || *show_version)
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
    else if (*save_state)
    {
        print_state_json(serial);
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

    return 0;
}
