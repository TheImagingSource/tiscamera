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

#include "BackendLoader.h"

#include <dlfcn.h>

#include <string>

#include "internal.h"
#include "devicelibrary.h"

tcam::BackendLoader& tcam::BackendLoader::getInstance ()
{
    static BackendLoader loader;

    return loader;
}


tcam::BackendLoader::BackendLoader ()
{
    load_backends();
}


tcam::BackendLoader::~BackendLoader ()
{
    unload_backends();
}


template<class T>
std::function<T> load(void* handle, std::string const& functionName)
{
    dlerror();
    void* const result = dlsym(handle, functionName.c_str());
    if (!result)
    {
        char* const error = dlerror();
        if (error)
        {
            throw std::logic_error("can't find symbol named \"" + functionName + "\": " + error);
        }
    }

    return reinterpret_cast<T*>(result);
}


void tcam::BackendLoader::load_backends ()
{

    backends =
        {
            {TCAM_DEVICE_TYPE_V4L2,    "libtcam-v4l2.so",   nullptr, nullptr, nullptr, nullptr},
            {TCAM_DEVICE_TYPE_ARAVIS,  "libtcam-aravis.so", nullptr, nullptr, nullptr, nullptr},
            {TCAM_DEVICE_TYPE_LIBUSB,  "libtcam-libusb.so", nullptr, nullptr, nullptr, nullptr},
            {TCAM_DEVICE_TYPE_UNKNOWN, "none",              nullptr, nullptr, nullptr, nullptr}
        };

    for (auto& b : backends)
    {
        if (b.type == TCAM_DEVICE_TYPE_UNKNOWN)
        {
            continue;
        }
        void* handle = dlopen(b.name.c_str(), RTLD_LAZY);

        if (handle == nullptr)
        {
            tcam_log(TCAM_LOG_INFO, "Could not load backend %s", b.name.c_str());
            tcam_log(TCAM_LOG_INFO, "    Reason: %s", dlerror());
            continue;
        }
        b.handle = handle;

        auto i = load<struct libinfo_v1*()>(b.handle, "get_library_functions_v1");

        auto info = (i)();


        auto f = std::function<tcam::DeviceInterface*(const struct tcam_device_info*)>(info->open_device);
        b.open_device = f;


        auto fls = std::function<size_t()>(info->get_device_list_size);
        b.get_device_list_size = fls;

        auto fl = std::function<size_t(struct tcam_device_info*, size_t)>(info->get_device_list);
        b.get_device_list = fl;

        delete info;
    }
}


void tcam::BackendLoader::unload_backends ()
{
    for (auto& b : backends)
    {
        if (b.handle != nullptr)
        {
            dlclose(b.handle);
            b.handle = nullptr;
            b.open_device = nullptr;
            b.get_device_list = nullptr;
        }
    }
}


std::shared_ptr<DeviceInterface> tcam::BackendLoader::open_device (const tcam::DeviceInfo& device)
{

    auto search = [this] (TCAM_DEVICE_TYPE type)
        {
            for (auto& b : this->backends)
            {
                if (b.type == type)
                    return b;
            }
            return this->backends.back();
        };

    auto b = search(device.get_device_type());

    if (b.type == TCAM_DEVICE_TYPE_UNKNOWN)
    {
        throw std::runtime_error("Unsupported device type");
    }

    auto dev = device.get_info();

    return std::shared_ptr<DeviceInterface>(b.open_device(&dev));
}


std::vector<DeviceInfo> BackendLoader::get_device_list_from_backend (BackendLoader::backend& b)
{
    std::vector<DeviceInfo> ret;

    if (b.handle == nullptr)
    {
        return ret;
    }
    tcam_log(TCAM_LOG_DEBUG, "retrieving list for %s", b.name.c_str());
    size_t t = b.get_device_list_size();

    tcam_log(TCAM_LOG_DEBUG, "Amount of devices: %d", t);

    struct tcam_device_info temp[t];

    auto copied_elements = b.get_device_list(temp, t);
    ret.reserve(copied_elements);

    for (const auto& info : temp)
    {
        ret.push_back(DeviceInfo(info));
    }

    return ret;
}


std::vector<DeviceInfo> BackendLoader::get_device_list (enum TCAM_DEVICE_TYPE type)
{
    for (auto& b : backends)
    {
        if (b.type == type && b.get_device_list != nullptr)
        {
            return get_device_list_from_backend(b);
        }
    }

    return {};
}


std::vector<DeviceInfo> BackendLoader::get_device_list_all_backends ()
{
    std::vector<DeviceInfo> ret;
    for (auto& b : backends)
    {
        if (b.type == TCAM_DEVICE_TYPE_UNKNOWN || b.handle == nullptr)
        {
            continue;
        }
        auto tmp = get_device_list_from_backend(b);
        ret.insert(ret.begin(), tmp.begin(), tmp.end());
    }
    return ret;
}
