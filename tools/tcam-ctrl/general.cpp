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


bool is_valid_device_serial(const std::string& serial)
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

    g_list_free(devices);

    return ret;
}
