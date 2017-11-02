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

#include "devicelibrary.h"
// #include "v4l2library.h"

#include "AFU050Device.h"
#include "UsbHandler.h"
#include "libusb_utils.h"


#include <cstring>

VISIBILITY_INTERNAL

DeviceInterface* open_device (const struct tcam_device_info* device)
{
    return new AFU050Device(DeviceInfo(*device), UsbHandler::get_instance().get_session());
}


size_t get_device_list_size ()
{
    auto vec = get_libusb_device_list();
    return vec.size();
}


/**
 * @return number of copied device_infos
 */
size_t get_device_list (struct tcam_device_info* array, size_t array_size)
{
    auto vec = get_libusb_device_list();

    if (vec.size() > array_size)
    {
        return 0;
    }

    for (const auto v : vec)
    {
        auto i = v.get_info();
        memcpy(array, &i, sizeof(struct tcam_device_info));
        array++;
    }

    return vec.size();
}

VISIBILITY_POP

struct libinfo_v1* get_library_functions_v1 ()
{
    struct libinfo_v1* info = new libinfo_v1();

    info->open_device = &open_device;
    info->get_device_list_size = &get_device_list_size;
    info->get_device_list = &get_device_list;

    return info;
}
