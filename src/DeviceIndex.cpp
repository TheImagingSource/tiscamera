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

#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <cstring>
#include <unistd.h>

#include "utils.h"
#include "logging.h"
#include "DeviceIndex.h"

#include "BackendLoader.h"

using namespace tcam;


DeviceIndex::DeviceIndex ()
    : continue_thread(false),
      wait_period(30),
      have_list(false),
      device_list(std::vector<DeviceInfo>()),
      callbacks(std::vector<callback_data>())
{
    continue_thread = true;
    work_thread = std::thread (&DeviceIndex::run, this);

    work_thread.detach();
}


DeviceIndex::~DeviceIndex ()
{
    continue_thread = false;
    if (work_thread.joinable())
    {
        work_thread.join();
    }
}


void DeviceIndex::register_device_lost (dev_callback c, void* user_data)
{
    tcam_log(TCAM_LOG_DEBUG, "Registered device lost callback");

    std::lock_guard<std::mutex> lock(mtx);
    callbacks.push_back({c, user_data, ""});
}


void DeviceIndex::register_device_lost (dev_callback c,
                                        void* user_data,
                                        const std::string& serial)
{
    tcam_log(TCAM_LOG_DEBUG, "Registered device lost callback for %s", serial.c_str());
    std::lock_guard<std::mutex> lock(mtx);
    callbacks.push_back({c, user_data, serial});
}


void DeviceIndex::remove_device_lost (dev_callback callback)
{
    std::lock_guard<std::mutex> lock(mtx);

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


void DeviceIndex::remove_device_lost (dev_callback callback, const std::string& serial)
{
    std::lock_guard<std::mutex> lock(mtx);

    auto it = std::begin(callbacks); //std::begin is a free function in C++11
    for (auto& value : callbacks)
    {
        if (value.callback == callback && value.serial.compare(serial) == 0)
        {
            callbacks.erase(it);
            break;
        }
        it++; //at the end OR make sure you do this in each iteration
    }
}


void DeviceIndex::sort_device_list ()
{
    /*
      Sorting of devices shall create the following order:

      local before remote/network meaing v4l2, libusb, aravis
      sorted after user defined names
      sorted serials, if no name is given
     */

    auto compareDeviceInfo = [] (const DeviceInfo& info1, const DeviceInfo& info2)
        {
            if (info1.get_device_type() >= info2.get_device_type())
            {
                if (info1.get_serial() > info2.get_serial())
                {
                    return false;
                }

            }
            return true;
        };

    std::sort(device_list.begin(), device_list.end(), compareDeviceInfo);
}


void DeviceIndex::update_device_list ()
{
    std::vector<DeviceInfo> tmp_dev_list = std::vector<DeviceInfo>();
    tmp_dev_list.reserve(10);

    auto dev_list = BackendLoader::getInstance().get_device_list_all_backends();
    if (!dev_list.empty())
        tmp_dev_list.insert(tmp_dev_list.end(), dev_list.begin(), dev_list.end());

    tcam_log(TCAM_LOG_DEBUG, "Number of found devices: %d", dev_list.size());

    // check for lost devices
    for (const auto& d : device_list)
    {
        auto f = [&d] (const DeviceInfo& info)
            {
                if (d.get_serial().compare(info.get_serial()) == 0)
                    return true;
                return false;
            };

        auto found = std::find_if(tmp_dev_list.begin(), tmp_dev_list.end(), f);

        if (found == tmp_dev_list.end())
        {
            tcam_log(TCAM_LOG_INFO, "Lost device %s. Contacting callbacks", d.get_name().c_str());
            fire_device_lost(d);
        }
    }
    std::lock_guard<std::mutex> lock(mtx);

    device_list.clear();

    device_list.insert(device_list.end(), tmp_dev_list.begin(), tmp_dev_list.end());

    sort_device_list();

    have_list = true;
    wait_for_list.notify_all();
}


void DeviceIndex::run ()
{
    while (continue_thread)
    {
        update_device_list();
        std::this_thread::sleep_for(std::chrono::seconds(wait_period));
    }
}


void DeviceIndex::fire_device_lost (const DeviceInfo& d)
{
    std::lock_guard<std::mutex> lock(mtx);

    for (auto& c : callbacks)
    {
        if (c.serial.empty() || c.serial.compare(d.get_serial()) == 0)
        {
            c.callback(d, c.data);
        }
    }
}


bool DeviceIndex::fill_device_info (DeviceInfo& info) const
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


std::vector<DeviceInfo> DeviceIndex::get_device_list () const
{
    std::unique_lock<std::mutex> lock(mtx);

    // wait for work_thread to deliver first valid list
    // since get_aravis_device_list is a blocking function
    // our thread would retrieve an empty list without this wait loop
    if (!have_list)
    {
        wait_for_list.wait_for(lock, std::chrono::seconds(wait_period));
    }

    return device_list;
}


DeviceIndex& DeviceIndex::get_instance()
{
    static DeviceIndex static_instance;

    return static_instance;
}


std::vector<DeviceInfo> tcam::get_device_list ()
{
    return DeviceIndex::get_instance().get_device_list();
}
