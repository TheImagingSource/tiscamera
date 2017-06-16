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

#ifndef TCAM_BACKEND_LOADER_H
#define TCAM_BACKEND_LOADER_H


#include <vector>
#include <string>
#include <memory>
#include <functional>

#include "base_types.h"

#include "DeviceInterface.h"

VISIBILITY_INTERNAL

namespace tcam
{

class BackendLoader
{
private:

    BackendLoader ();
    ~BackendLoader ();

    BackendLoader& operator= (const BackendLoader&) = delete;

    struct backend
    {
        enum TCAM_DEVICE_TYPE type;
        std::string name;
        void* handle;

        // std::function<std::vector<DeviceInfo>()> get_device_list;
        // std::function<std::shared_ptr<DeviceInterface>(const DeviceInfo&)> open_device;
        std::function<size_t(struct tcam_device_info*, size_t)> get_device_list;
        std::function<size_t()> get_device_list_size;
        std::function<tcam::DeviceInterface*(const struct tcam_device_info*)> open_device;
    };

    std::vector<backend> backends;

    void load_backends ();


    void unload_backends ();

    std::vector<DeviceInfo> get_device_list_from_backend (BackendLoader::backend& b);

public:

    static BackendLoader& getInstance ();

    std::shared_ptr<DeviceInterface> open_device (const DeviceInfo&);

    std::vector<DeviceInfo> get_device_list_all_backends ();

    std::vector<DeviceInfo> get_device_list (enum TCAM_DEVICE_TYPE);

}; /* class BackendLoader*/


} /* namespace tcam */

VISIBILITY_POP

#endif /* TCAM_BACKEND_LOADER_H */
