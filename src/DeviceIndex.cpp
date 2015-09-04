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

#include "Error.h"

#if HAVE_ARAVIS
#include "aravis_utils.h"
#endif

#if HAVE_USB
#include "v4l2_utils.h"
#endif

using namespace tcam;


DeviceIndex::DeviceIndex ()
    : continue_thread(false),
      wait_period(2),
      init_count(0),
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
    mtx.lock();
    callbacks.push_back({c, user_data, ""});
    mtx.unlock();
}


void DeviceIndex::register_device_lost (dev_callback c,
                                        void* user_data,
                                        const std::string& serial)
{
    tcam_log(TCAM_LOG_DEBUG, "Registered device lost callback for %s", serial.c_str());
    mtx.lock();
    callbacks.push_back({c, user_data, serial});
    mtx.unlock();
}


void DeviceIndex::remove_device_lost (dev_callback callback)
{
    mtx.lock();

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

    mtx.unlock();
}


void DeviceIndex::remove_device_lost (dev_callback callback, const std::string& serial)
{
    mtx.lock();

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

    mtx.unlock();
}


void DeviceIndex::update_device_list ()
{
    std::vector<DeviceInfo> tmp_dev_list = std::vector<DeviceInfo>();
    tmp_dev_list.reserve(10);

#if HAVE_ARAVIS
    auto aravis_dev_list = get_aravis_device_list();
    if (!aravis_dev_list.empty())
        tmp_dev_list.insert(tmp_dev_list.end(), aravis_dev_list.begin(), aravis_dev_list.end());

    tcam_log(TCAM_LOG_DEBUG, "Number of found aravis devices: %d", aravis_dev_list.size());

#endif

#if HAVE_USB
    auto v4l2_dev_list = get_v4l2_device_list();
    if (!v4l2_dev_list.empty())
        tmp_dev_list.insert(tmp_dev_list.end(), v4l2_dev_list.begin(), v4l2_dev_list.end());

    tcam_log(TCAM_LOG_DEBUG, "Number of found v4l2 devices: %d", v4l2_dev_list.size());
#endif

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
            tcam_log(TCAM_LOG_INFO, "Lost device %s. Conntacting callbacks", d.get_name().c_str());
            fire_device_lost(d);
        }
    }

    mtx.lock();

    device_list.clear();

    device_list.insert(device_list.end(), tmp_dev_list.begin(), tmp_dev_list.end());

    if (init_count < INIT_MAX)
    {
        init_count++;
    }
    mtx.unlock();
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
    mtx.lock();
    for (auto& c : callbacks)
    {
        if (c.serial.empty() || c.serial.compare(d.get_serial()) == 0)
        {
            c.callback(d, c.data);
        }
    }
    mtx.unlock();
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

    while (init_count < INIT_MAX)
    {
        usleep(500);
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
