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


#include "DeviceInterface.h"
#include "LibraryHandle.h"
#include "base_types.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

VISIBILITY_INTERNAL

namespace tcam
{

class BackendLoader
{
private:
    BackendLoader();
    ~BackendLoader();

    BackendLoader& operator=(const BackendLoader&) = delete;

    static std::weak_ptr<BackendLoader> instance;

    struct backend;

    std::vector<backend> backends;

    void load_backends();


    void unload_backends();

    std::vector<DeviceInfo> get_device_list_from_backend(BackendLoader::backend& b);

public:
    static std::shared_ptr<BackendLoader> get_instance();

    std::shared_ptr<DeviceInterface> open_device(const DeviceInfo&);

    std::vector<DeviceInfo> get_device_list_all_backends();

    std::vector<DeviceInfo> get_device_list(enum TCAM_DEVICE_TYPE);

}; /* class BackendLoader*/


} /* namespace tcam */

VISIBILITY_POP

#endif /* TCAM_BACKEND_LOADER_H */
