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

/**
 * @addtogroup API
 * @{
 */

namespace tcam
{

typedef void (*dev_callback) (const DeviceInfo&);

class DeviceIndex
{

public:

    DeviceIndex ();

    ~DeviceIndex ();

    std::vector<DeviceInfo> get_device_list () const;

    void register_device_lost (dev_callback);

    /**
     * @param[in/out] DeviceInfo that shall be filled. \
     *                Must contain identifier or serial
     * @return true if device found and argument could be filled
     */
    bool fill_device_info (DeviceInfo&) const;

private:

    bool continue_thread;
    std::mutex mtx;
    unsigned int wait_period;
    std::thread work_thread;

    std::vector<DeviceInfo> device_list;

    std::vector<dev_callback> callbacks;

    void update_device_list ();

    void run ();

    void fire_device_lost (const DeviceInfo& d);

};


std::shared_ptr<DeviceIndex> get_device_index ();

} /* namespace tcam */

/** @} */

#endif /* TCAM_CAMERA_INDEX_H */
