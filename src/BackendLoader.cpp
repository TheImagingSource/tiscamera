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

std::weak_ptr<BackendLoader> BackendLoader::instance;


std::shared_ptr<BackendLoader> BackendLoader::get_instance ()
{
    auto inst = instance.lock();

    if (!inst)
    {
        struct wrapper : BackendLoader {};

        inst = std::make_shared<wrapper>();
        instance = inst;
    }

    return inst;
}


tcam::BackendLoader::BackendLoader ()
{
    load_backends();
}


tcam::BackendLoader::~BackendLoader ()
{
    // Do not actually unload the backends for now.
    // If the application does not stop the video stream and/or unrefs all stream objects when it terminates,
    // a backend may still have some threads running when reaching this point.
    // Unloading the backend now would then result in a segmentation fault.
    // Since the unloader only gets called when the application terminates, not unloading the backends should not impose a resource leak.
    //
    // TODO: Add a mechanism for reference counting for device backends such that the unloader could at least throw an exception if a
    //       backend is still referenced at this point.

    unload_backends();
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

        b.handle = LibraryHandle::open(b.name);

        if (b.handle == nullptr)
        {
            tcam_log(TCAM_LOG_INFO, "Could not load backend %s", b.name.c_str());
            continue;
        }

        auto i = b.handle->load<struct libinfo_v1*()>("get_library_functions_v1");

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

    auto ptr = std::shared_ptr<DeviceInterface>(b.open_device(&dev));

    ptr->set_backend_loader(get_instance());

    return ptr;
}


std::vector<DeviceInfo> BackendLoader::get_device_list_from_backend (BackendLoader::backend& b)
{
    std::vector<DeviceInfo> ret;

    if (b.handle == nullptr)
    {
        return ret;
    }
    tcam_trace("retrieving list for %s", b.name.c_str());
    size_t t = b.get_device_list_size();

    tcam_trace("Amount of devices: %d", t);

    struct tcam_device_info *temp = new struct tcam_device_info[t];

    size_t copied_elements = b.get_device_list(temp, t);
    ret.reserve(copied_elements);
    for (size_t i = 0; i < copied_elements; i++)
    {
        ret.push_back(DeviceInfo(temp[i]));
    }

    delete [] temp;

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
