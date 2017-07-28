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

#ifndef TCAM_CAMERA_INDEX_H
#define TCAM_CAMERA_INDEX_H

#include "base_types.h"
#include "DeviceInfo.h"

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

/**
 * @addtogroup API
 * @{
 */

namespace tcam
{

typedef void (*dev_callback) (const DeviceInfo&, void* user_data);

class DeviceIndex
{

public:


    std::vector<DeviceInfo> get_device_list () const;


    /**
     * @name register_device_lost
     * @param callback - function pointer to use
     * @brief
     */
    void register_device_lost (dev_callback callback,
                               void* user_data);

    /**
     * @name register_device_lost
     * @param callback - function pointer to use
     * @param serial - serialnumber of the device that has to be \
     *                 lost for the callback to be called
     * @brief
     */
    void register_device_lost (dev_callback callback,
                               void* user_data,
                               const std::string& serial);


    /**
     * @name remove_device_lost
     * @param callback - function pointer to use
     * @brief
     */
    void remove_device_lost (dev_callback callback);

    void remove_device_lost (dev_callback callback, const std::string& serial);

    /**
     * @param[in/out] DeviceInfo that shall be filled. \
     *                Must contain identifier or serial
     * @return true if device found and argument could be filled
     */
    bool fill_device_info (DeviceInfo&) const;

    static DeviceIndex& get_instance ();

private:

    DeviceIndex ();

    ~DeviceIndex ();

    bool continue_thread;
    mutable std::mutex mtx;
    unsigned int wait_period;
    std::thread work_thread;

    bool have_list;
    mutable std::condition_variable wait_for_list;

    std::vector<DeviceInfo> device_list;

    struct callback_data
    {
        dev_callback callback;
        void* data;
        std::string serial;
    };

    std::vector<callback_data> callbacks;

    void sort_device_list ();

    void update_device_list ();

    void run ();

    void fire_device_lost (const DeviceInfo& d);

};


std::vector<DeviceInfo> get_device_list ();

} /* namespace tcam */

/** @} */

#endif /* TCAM_CAMERA_INDEX_H */
