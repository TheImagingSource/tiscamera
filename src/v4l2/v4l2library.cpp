/*
 * Copyright 2016 The Imaging Source Europe GmbH
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
#include "v4l2_api.h"

#include <cstring>

VISIBILITY_INTERNAL

DeviceInterface* open_device (const struct tcam_device_info* device)
{
    return open_v4l2_device(device);
}


size_t get_device_list_size ()
{
    return get_v4l2_device_list_size();
}


/**
 * @return number of copied device_infos
 */
size_t get_device_list (struct tcam_device_info* array, size_t array_size)
{
    return get_v4l2_device_list(array, array_size);
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
