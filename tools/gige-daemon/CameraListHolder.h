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
#include <thread>
#include <mutex>
#include <condition_variable>

#include "tcam.h"

using namespace tcam;

class CameraListHolder
{
public:

    ~CameraListHolder ();

    static CameraListHolder& get_instance ();

    std::vector<DeviceInfo> get_camera_list () const;

    std::vector<std::string> get_interface_list () const;

    void set_interface_list (std::vector<std::string>);

    void run ();

    void stop ();

private:
    CameraListHolder ();

    void index_loop ();

    void loop_function ();

    std::vector<struct tcam_device_info> get_aravis_device_list ();

    std::vector<DeviceInfo> camera_list;

    std::vector<std::string> interface_list;

    bool continue_loop;
    std::thread work_thread;
    std::mutex mtx;
    std::mutex real_mutex;
    std::condition_variable cv;

    // memory sharing keys
    int shmid;
    key_t shmkey;
    void* data;

    key_t semaphore_key;
    int semaphore_id;
};
