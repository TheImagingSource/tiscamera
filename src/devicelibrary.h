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

#ifndef TCAM_DEVICELIBRARY_H
#define TCAM_DEVICELIBRARY_H

#include "DeviceInterface.h"

using namespace tcam;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    struct libinfo_v1
    {
        // std::shared_ptr<DeviceInterface> (*create_device) (const DeviceInfo& device);
        // std::vector<DeviceInfo> (*get_device_list) ();

        DeviceInterface* (*open_device) (const struct tcam_device_info*);


        size_t (*get_device_list_size) ();

        /**
         * @return number of copied device_infos
         */
        size_t (*get_device_list) (struct tcam_device_info* array, size_t array_size);

    };



    struct libinfo_v1* get_library_functions_v1 ();


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TCAM_DEVICELIBRARY_H */
