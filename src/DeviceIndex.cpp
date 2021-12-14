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

#include "Indexer.h"
#include "logging.h"

#include <algorithm>

using namespace tcam;

DeviceIndex::DeviceIndex()
    : indexer_(Indexer::get_instance())
{
}

DeviceIndex::~DeviceIndex()
{
    for (auto& cb : callbacks) { indexer_->remove_device_lost(cb); }
}

void DeviceIndex::register_device_lost(dev_callback c, void* user_data)
{
    callbacks.push_back(c);

    indexer_->register_device_lost(c, user_data);
}

void DeviceIndex::register_device_lost(dev_callback c, void* user_data, const std::string& serial)
{
    callbacks.push_back(c);

    indexer_->register_device_lost(c, user_data, serial);
}

void DeviceIndex::remove_device_lost(dev_callback callback)
{
    indexer_->remove_device_lost(callback);

    auto it = std::begin(callbacks); //std::begin is a free function in C++11
    for (const auto& value : callbacks)
    {
        if (value == callback)
        {
            callbacks.erase(it);
            break;
        }
        it++; //at the end OR make sure you do this in each iteration
    }
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
