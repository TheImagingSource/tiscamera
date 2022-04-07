/*
 * Copyright 2020 The Imaging Source Europe GmbH
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

#include <gst/gst.h>
#include <iostream>


bool tcam::tools::ctrl::is_valid_device_serial (const std::string& serial)
{
    bool ret = false;

    auto monitor = gst_device_monitor_new();
    gst_device_monitor_add_filter(monitor, "Video/Source/tcam", NULL);

    GList* devices = gst_device_monitor_get_devices(monitor);

    gst_object_unref(monitor);

    bool is_long_serial = false;

    if (serial.find("-") != std::string::npos)
    {
        is_long_serial = true;
    }

    for (GList* elem = devices; elem; elem = elem->next)
    {
        auto dev = (GstDevice*) elem->data;
        GstStructure* struc = gst_device_get_properties(dev);

        std::string dev_serial;

        if (is_long_serial)
        {
            dev_serial = gst_structure_get_string(struc, "serial");
            dev_serial += "-";
            dev_serial += gst_structure_get_string(struc, "type");
        }
        else
        {
            dev_serial = gst_structure_get_string(struc, "serial");
        }
        gst_structure_free(struc);

        if (serial == dev_serial)
        {
            ret = true;
            break;
        }
    }

    g_list_free_full(devices, gst_object_unref);

    return ret;
}


gst_helper::gst_ptr<GstElement> tcam::tools::ctrl::open_element (const std::string& element_name)
{
    gst_helper::gst_ptr<GstElement> source = gst_helper::make_ptr(gst_element_factory_make(element_name.c_str(), "source"));

    if (!source)
    {
        std::cerr << "Unable to create gstreamer element." << std::endl;
        return nullptr;
    }
    return source;
}


bool tcam::tools::ctrl::set_serial (gst_helper::gst_ptr<GstElement>& element,
                                    const std::string& serial)
{
    if (!is_valid_device_serial(serial))
    {
        std::cerr << "Device with given serial does not exist." << std::endl;
        return false;
    }

    GValue val = {};
    g_value_init(&val, G_TYPE_STRING);
    g_value_set_static_string(&val, serial.c_str());

    g_object_set_property(G_OBJECT(element.get()), "serial", &val);
    g_value_unset(&val);

    return true;
}


tcam::tools::ctrl::ElementStateGuard::ElementStateGuard(GstElement& element) : p_element(element) {}

tcam::tools::ctrl::ElementStateGuard::~ElementStateGuard()
{
    gst_element_set_state(&p_element, GST_STATE_NULL);
}

bool tcam::tools::ctrl::ElementStateGuard::set_state(GstState state)
{
    auto change_res = gst_element_set_state(&p_element, state);

    if (change_res == GST_STATE_CHANGE_SUCCESS)
    {
        return true;
    }
    else if (change_res == GST_STATE_CHANGE_ASYNC)
    {
        GstClockTime check_timeout = 2000000; // 2 seconds
        auto check_res = gst_element_get_state(&p_element, nullptr, nullptr, check_timeout);

        // only check for success
        // if after 2 seconds that is not the case something is fishy
        // user has to take care of that
        if (check_res == GST_STATE_CHANGE_SUCCESS)
        {
            return true;
        }
    }
    return false;
}
