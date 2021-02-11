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

#include "DeviceIndex.h"

#include "BackendLoader.h"
#include "Indexer.h"
#include "logging.h"
#include "utils.h"

#include <algorithm>
#include <cstring>
#include <memory>
#include <string>
#include <unistd.h>
#include <vector>

using namespace tcam;


DeviceIndex::DeviceIndex()
    : indexer_(Indexer::get_instance()), device_list(std::vector<DeviceInfo>()),
      callbacks(std::vector<callback_data>())
{
}


DeviceIndex::~DeviceIndex()
{
    for (auto& cb : callbacks) { indexer_->remove_device_lost(cb.callback); }
}


void DeviceIndex::register_device_lost(dev_callback c, void* user_data)
{
    callbacks.push_back({ c, user_data, "" });

    indexer_->register_device_lost(c, user_data);
}


void DeviceIndex::register_device_lost(dev_callback c, void* user_data, const std::string& serial)
{
    callbacks.push_back({ c, user_data, serial });

    indexer_->register_device_lost(c, user_data, serial);
}


void DeviceIndex::remove_device_lost(dev_callback callback)
{
    indexer_->remove_device_lost(callback);

    auto it = std::begin(callbacks); //std::begin is a free function in C++11
    for (auto& value : callbacks)
    {

        if (value.callback == callback)
        {
            callbacks.erase(it);
            break;
        }
        it++; //at the end OR make sure you do this in each iteration
    }
}


void DeviceIndex::remove_device_lost(dev_callback callback, const std::string& serial)
{
    indexer_->remove_device_lost(callback, serial);

    auto it = std::begin(callbacks); //std::begin is a free function in C++11
    for (auto& value : callbacks)
    {

        if (value.callback == callback)
        {
            callbacks.erase(it);
            break;
        }
        it++; //at the end OR make sure you do this in each iteration
    }
}


bool DeviceIndex::fill_device_info(DeviceInfo& info) const
{
    if (!info.get_serial().empty())
    {
        for (const auto& d : device_list)
        {
            if (info.get_serial() == d.get_serial())
            {
                info = d;
                return true;
            }
        }
        return false;
    }

    if (!info.get_identifier().empty())
    {
        for (const auto& d : device_list)
        {
            if (info.get_identifier() == d.get_identifier())
            {
                info = d;
                return true;
            }
        }
    }
    return false;
}


std::vector<DeviceInfo> DeviceIndex::get_device_list() const
{
    if (!indexer_)
    {
        SPDLOG_ERROR("No Indexer present. Unable to retrieve device list");
        return std::vector<DeviceInfo>();
    }

    return indexer_->get_device_list();
}
