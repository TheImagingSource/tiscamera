/*
 * Copyright 2016 The Imaging Source Europe GmbH
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

#include "CameraListHolder.h"

#include "../../src/aravis/aravis_utils.h"
#include "../../src/tcam-network/CameraDiscovery.h"
#include "../../src/tcam-semaphores.h"
#include "gige-daemon.h"

#include <cstring>
#include <sstream>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/ipc.h> /* general SysV IPC structures          */
#include <sys/msg.h>
#include <sys/sem.h> /* semaphore functions and structs.     */
#include <sys/shm.h>
#include <sys/types.h> /* various type definitions.            */
#include <stdexcept>

using namespace tis;


tcam::tools::gige_daemon::CameraListHolder::CameraListHolder() : continue_loop(true)
{
    // to understand shared memory use this guide:
    // https://beej.us/guide/bgipc/html/single/bgipc.html#semaphores
    // https://beej.us/guide/bgipc/html/single/bgipc.html#mqftok

    errno=0;

    // use LOCK_FILE as ftok key and nothing else
    // LOCK_FILE is created and deleted by this process
    // it is thus guaranteed to be a unique file that is not
    // used by anything else
    shmkey = ftok(LOCK_FILE, 'G');

    if (shmkey == -1)
    {
        throw std::runtime_error("Unable to generate shmkey");
    }

    /* the daemon should work as a system daemon,
       thus we require the same rights for all users to prevent 'permission denied'
       errors due to server/client being run by different users. */
    shmid = shmget(shmkey, sizeof(struct tcam_gige_device_list), 0666 | IPC_CREAT);

    if (shmid == -1)
    {
        throw std::runtime_error("Unable to allocate shared memory segment");
    }

    semaphore_key = ftok(LOCK_FILE, 'S');

    if (semaphore_key == -1)
    {
        throw std::runtime_error("Unable to generate semaphore key");
    }

    semaphore_id = tcam::semaphore::create(semaphore_key);

    work_thread = std::thread(&CameraListHolder::index_loop, this);
}


tcam::tools::gige_daemon::CameraListHolder::~CameraListHolder()
{
    continue_loop = false;
    if (work_thread.joinable())
    {
        work_thread.join();
    }

    shmctl(shmid, IPC_RMID, NULL);
}


tcam::tools::gige_daemon::CameraListHolder& tcam::tools::gige_daemon::CameraListHolder::get_instance()
{
    static CameraListHolder instance;

    return instance;
}


std::vector<DeviceInfo> tcam::tools::gige_daemon::CameraListHolder::get_camera_list()
{
    std::lock_guard<semaphore> lck(semaphore_id);

    struct tcam_gige_device_list* d = (struct tcam_gige_device_list*)shmat(shmid, NULL, 0);

    if (d == nullptr)
    {
        return std::vector<DeviceInfo>();
    }

    std::vector<DeviceInfo> ret;

    for (unsigned int i = 0; i < d->device_count; ++i) { ret.push_back(DeviceInfo(d->devices[i])); }

    return ret;
}


std::vector<std::string> tcam::tools::gige_daemon::CameraListHolder::get_interface_list() const
{
    return interface_list;
}


void tcam::tools::gige_daemon::CameraListHolder::set_interface_list(std::vector<std::string>& interfaces)
{
    std::lock_guard<std::mutex> mutex_lock(mtx);
    interface_list = interfaces;
}


void tcam::tools::gige_daemon::CameraListHolder::stop()
{
    this->continue_loop = false;
    cv.notify_all();
}


void tcam::tools::gige_daemon::CameraListHolder::index_loop()
{
    while (continue_loop) { loop_function(); }
}


static std::vector<tcam::DeviceInfo> get_aravis_list()
{
    return tcam::get_aravis_device_list();
}


void tcam::tools::gige_daemon::CameraListHolder::loop_function()
{
    std::unique_lock<std::mutex> lck(mtx);

    auto res = cv.wait_for(lck, std::chrono::seconds(2));

    // preemptiv stop
    if (res == std::cv_status::no_timeout)
    {
        return;
    }

    std::vector<tcam::DeviceInfo> aravis_list = get_aravis_list();
    std::vector<struct tcam_device_info> arv_list;
    arv_list.reserve(aravis_list.size());
    for (const auto& e : aravis_list) { arv_list.push_back(e.get_info()); }

    if (arv_list.size() > TCAM_DEVICE_LIST_MAX)
    {
        return;
    }

    std::lock_guard<semaphore> lck2(semaphore_id);

    struct tcam_gige_device_list* tmp_ptr = (struct tcam_gige_device_list*)shmat(shmid, NULL, 0);

    tmp_ptr->device_count = arv_list.size();

    struct tcam_device_info* ptr = tmp_ptr->devices;
    for (const auto& dev : arv_list)
    {
        memcpy(ptr, &dev, sizeof(struct tcam_device_info));
        ++ptr;
    }

    // force update to change time
    struct shmid_ds ds = {};
    shmctl(shmid, IPC_STAT, &ds);
    shmctl(shmid, IPC_SET, &ds);

    // release data ptr
    shmdt(tmp_ptr);
}
