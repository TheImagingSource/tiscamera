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

#include "CameraDiscovery.h"

#include "tcam-semaphores.h"

#include <sys/sem.h>     /* semaphore functions and structs.     */
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>

#include "gige-daemon.h"

#include <stdlib.h>
#include <sstream>
#include <cstring>

#include <sys/types.h>   /* various type definitions.            */
#include <sys/ipc.h>     /* general SysV IPC structures          */

using namespace tis;


CameraListHolder::CameraListHolder ()
    : continue_loop(true), data(nullptr)
{
    // to understand shared memory use this guide:
    // http://beej.us/guide/bgipc/output/html/multipage/shm.html

    shmkey = ftok("/tmp/tcam-gige-camera-list", 'G');

    /* the daemon should work as a system daemon,
       thus we require the same rights for all users to prevent 'permission denied'
       errors due to server/client being run by different users. */
    shmid = shmget(shmkey, sizeof(struct tcam_gige_device_list), 0666 | IPC_CREAT);

    if (shmid == -1)
    {
        throw;
    }

    semaphore_key = ftok("/tmp/tcam-gige-semaphore", 'S');
    semaphore_id = tcam::semaphore_create(semaphore_key);

    work_thread = std::thread(&CameraListHolder::index_loop, this);
}


CameraListHolder::~CameraListHolder ()
{
    continue_loop = false;
    if (work_thread.joinable())
    {
        work_thread.join();
    }

    struct tcam_gige_device_list* d = (struct tcam_gige_device_list*)shmat(shmid, NULL, NULL);
    d->device_count = 0;
    memset(d->devices, 0, sizeof(struct tcam_gige_device_list) * TCAM_DEVICE_LIST_MAX);
    shmdt(d);

    shmctl(shmid, IPC_RMID, NULL);

}


CameraListHolder& CameraListHolder::get_instance ()
{
    static CameraListHolder instance;

    return instance;
}


std::vector<DeviceInfo> CameraListHolder::get_camera_list () const
{
    tcam::semaphore_lock(semaphore_id);
    struct tcam_gige_device_list* d = (struct tcam_gige_device_list*)shmat(shmid, NULL, NULL);

    if (d == nullptr)
    {
        tcam::semaphore_unlock(semaphore_id);

        return std::vector<DeviceInfo>();
    }

    std::vector<DeviceInfo> ret(d->device_count);

    for (unsigned int i = 0; i < d->device_count; ++i)
    {
        ret.push_back(DeviceInfo(d->devices[i]));
    }

    tcam::semaphore_unlock(semaphore_id);

    return ret;
}


std::vector<std::string> CameraListHolder::get_interface_list () const
{
    return interface_list;
}

void CameraListHolder::set_interface_list (std::vector<std::string> interfaces)
{
    // std::mutex cam_lock;
    std::lock_guard<std::mutex> mutex_lock(mtx);
    interface_list = interfaces;
}


void CameraListHolder::run ()
{
    data = shmat(shmid, nullptr, 0);
}


void CameraListHolder::stop ()
{
    this->continue_loop = false;
    mtx.lock();
    cv.notify_all();
    mtx.unlock();
    shmctl(shmid, IPC_RMID, NULL);
    data = nullptr;

    if (this->work_thread.joinable())
    {
        this->work_thread.join();
    }
}


void CameraListHolder::index_loop ()
{
    while (continue_loop)
    {
        loop_function();
    }
}


camera_list getCameraList (std::vector<std::string> interfaces)
{
    camera_list cameras;
    std::mutex cam_lock;

    std::function<void(std::shared_ptr<Camera>)> f = [&cameras, &cam_lock] (std::shared_ptr<Camera> camera)
        {
            std::lock_guard<std::mutex> mutex_lock(cam_lock);
            cameras.push_back(camera);
        };

    if (interfaces.empty())
    {
        discoverCameras(f);
    }
    else
    {
        discoverCameras(interfaces, f);
    }
    return cameras;
}


std::vector<struct tcam_device_info> get_gige_device_list (std::vector<std::string> interfaces)
{
    // out of the tcam-network lib
    auto l = getCameraList(interfaces);

    std::vector<struct tcam_device_info> ret;

    ret.reserve(l.size());

    for (const auto& c : l)
    {
        struct tcam_device_info info;

        info.type = TCAM_DEVICE_TYPE_ARAVIS;

        strncpy(info.serial_number, c->getSerialNumber().c_str(), sizeof(info.serial_number));
        strncpy(info.name, c->getModelName().c_str(), sizeof(info.name));

        std::string identifier = c->getVendorName() + "-" + c->getSerialNumber();

        strncpy(info.identifier, identifier.c_str(), sizeof(info.identifier));

        ret.push_back(info);
    }

    return ret;
}


void CameraListHolder::loop_function ()
{
    std::unique_lock<std::mutex> lck(mtx);

    auto res = cv.wait_for(lck, std::chrono::seconds(2));

    // preemptiv stop
    if (res == std::cv_status::no_timeout)
    {
        return;
    }

    std::vector<struct tcam_device_info> aravis_list = get_gige_device_list(interface_list);

    if (aravis_list.size() > TCAM_DEVICE_LIST_MAX)
    {
        return;
    }

    tcam::semaphore_lock(semaphore_id);

    struct tcam_gige_device_list* tmp_ptr = (struct tcam_gige_device_list*) shmat(shmid, NULL, NULL);

    tmp_ptr->device_count = aravis_list.size();

    struct tcam_device_info* ptr = tmp_ptr->devices;
    for (const auto dev : aravis_list)
    {
        memcpy(ptr, &dev, sizeof(struct tcam_device_info));
        ++ptr;
    }

    // force update to change time
    struct shmid_ds ds = {};
    shmctl(shmid, IPC_STAT, &ds);
    shmctl(shmid, IPC_SET, &ds);

    // release dataptr
    shmdt(tmp_ptr);

    tcam::semaphore_unlock(semaphore_id);
}
