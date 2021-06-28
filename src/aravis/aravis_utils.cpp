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

#include "aravis_utils.h"

#include "internal.h"
#include "logging.h"

#include <algorithm>
#include <dutils_img/image_fourcc.h>
#include <vector>

// gige-daemon communication

#include "gige-daemon.h"
#include "tcam-semaphores.h"

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

using namespace tcam;


struct aravis_fourcc
{
    uint32_t fourcc;
    uint32_t aravis;
};


static const aravis_fourcc arv_fourcc_conversion_table[] = {
    { FOURCC_Y800, ARV_PIXEL_FORMAT_MONO_8 },
    { 0, ARV_PIXEL_FORMAT_MONO_8_SIGNED },
    { 0, ARV_PIXEL_FORMAT_MONO_10 },
    { FOURCC_MONO10_SPACKED, ARV_PIXEL_FORMAT_MONO_10_PACKED },
    { 0, ARV_PIXEL_FORMAT_MONO_12 },
    { FOURCC_MONO12_PACKED, ARV_PIXEL_FORMAT_MONO_12_PACKED },
    { 0, ARV_PIXEL_FORMAT_MONO_14 },
    { FOURCC_Y16, ARV_PIXEL_FORMAT_MONO_16 },
    { FOURCC_GRBG8, ARV_PIXEL_FORMAT_BAYER_GR_8 },
    { FOURCC_RGGB8, ARV_PIXEL_FORMAT_BAYER_RG_8 },
    { FOURCC_GBRG8, ARV_PIXEL_FORMAT_BAYER_GB_8 },
    { FOURCC_BGGR8, ARV_PIXEL_FORMAT_BAYER_BG_8 },
    { FOURCC_GRBG12_PACKED, ARV_PIXEL_FORMAT_BAYER_GR_12_PACKED },
    { FOURCC_RGGB12_PACKED, ARV_PIXEL_FORMAT_BAYER_RG_12_PACKED },
    { FOURCC_GBRG12_PACKED, ARV_PIXEL_FORMAT_BAYER_GB_12_PACKED },
    { FOURCC_BGGR12_PACKED, ARV_PIXEL_FORMAT_BAYER_BG_12_PACKED },
    { FOURCC_GRBG16, ARV_PIXEL_FORMAT_BAYER_GR_16 },
    { FOURCC_RGGB16, ARV_PIXEL_FORMAT_BAYER_RG_16 },
    { FOURCC_GBRG16, ARV_PIXEL_FORMAT_BAYER_GB_16 },
    { FOURCC_BGGR16, ARV_PIXEL_FORMAT_BAYER_BG_16 },
    { FOURCC_BGR24, 0x02180015 },
    { 0, ARV_PIXEL_FORMAT_RGB_8_PACKED },
    { 0, ARV_PIXEL_FORMAT_BGR_8_PACKED },
    { 0, ARV_PIXEL_FORMAT_RGBA_8_PACKED },
    { 0, ARV_PIXEL_FORMAT_BGRA_8_PACKED },
    { 0, ARV_PIXEL_FORMAT_RGB_10_PACKED },
    { 0, ARV_PIXEL_FORMAT_BGR_10_PACKED },
    { 0, ARV_PIXEL_FORMAT_RGB_12_PACKED },
    { 0, ARV_PIXEL_FORMAT_BGR_12_PACKED },
    { 0, ARV_PIXEL_FORMAT_YUV_411_PACKED },
    { 0, ARV_PIXEL_FORMAT_YUV_422_PACKED },
    { 0, ARV_PIXEL_FORMAT_YUV_444_PACKED },
    { 0, ARV_PIXEL_FORMAT_RGB_8_PLANAR },
    { 0, ARV_PIXEL_FORMAT_RGB_10_PLANAR },
    { 0, ARV_PIXEL_FORMAT_RGB_12_PLANAR },
    { 0, ARV_PIXEL_FORMAT_RGB_16_PLANAR },
    { FOURCC_IYU1, ARV_PIXEL_FORMAT_YUV_422_YUYV_PACKED },
    { FOURCC_POLARIZATION_BG8_90_45_135_0, 0x8108000D },
    { FOURCC_POLARIZATION_BG12_PACKED_90_45_135_0, 0x8108000E },
    { FOURCC_POLARIZATION_BG16_90_45_135_0, 0x8108000F },
    { FOURCC_POLARIZATION_MONO8_90_45_135_0, 0x8108000A },
    { FOURCC_POLARIZATION_MONO12_PACKED_90_45_135_0, 0x810C0010 },
    { FOURCC_POLARIZATION_MONO16_90_45_135_0, 0x8110000C },
};


uint32_t tcam::aravis2fourcc(uint32_t aravis)
{
    for (auto&& e : arv_fourcc_conversion_table)
    {
        if (e.aravis == aravis)
        {
            return e.fourcc;
        }
    }

    return 0;
}


uint32_t tcam::fourcc2aravis(uint32_t fourcc)
{
    for (const auto& e : arv_fourcc_conversion_table)
    {
        if (e.fourcc == fourcc)
        {
            return e.aravis;
        }
    }

    return 0;
}

static const std::string gige_daemon_lock_file = "/var/lock/gige-daemon.lock";


std::vector<DeviceInfo> tcam::get_gige_device_list()
{
    bool is_running = is_process_running(get_pid_from_lockfile(gige_daemon_lock_file));

    if (!is_running)
    {
        SPDLOG_INFO("Could not find gige-daemon. Using internal methods");
        return get_aravis_device_list();
    }

    key_t shmkey = ftok("/tmp/tcam-gige-camera-list", 'G');
    key_t sem_key = ftok("/tmp/tcam-gige-semaphore", 'S');

    int shmid = shmget(shmkey, sizeof(struct tcam_gige_device_list), 0644);
    if (shmid < 0)
    {
        SPDLOG_INFO("Unable to connect to gige-daemon. Using internal methods");
        auto vec = get_aravis_device_list();
        SPDLOG_DEBUG("Aravis gave us {}", vec.size());
        return vec;
    }

    semaphore sem_id = semaphore::create(sem_key);
    std::lock_guard<semaphore> lck(sem_id);

    struct tcam_gige_device_list* d = (struct tcam_gige_device_list*)shmat(shmid, NULL, 0);

    if (d == nullptr)
    {
        shmdt(d);
        return std::vector<DeviceInfo>();
    }

    std::vector<DeviceInfo> ret;

    ret.reserve(d->device_count);

    for (unsigned int i = 0; i < d->device_count; ++i) { ret.push_back(DeviceInfo(d->devices[i])); }

    shmdt(d);

    return ret;
}


unsigned int tcam::get_gige_device_count()
{
    return get_gige_device_list().size();
}

unsigned int tcam::get_aravis_device_count()
{
    arv_update_device_list();

    return arv_get_n_devices();
}


std::vector<DeviceInfo> tcam::get_aravis_device_list()
{
    std::vector<DeviceInfo> device_list;

    arv_update_device_list();

    unsigned int number_devices = arv_get_n_devices();

    if (number_devices == 0)
    {
        return device_list;
    }

    for (unsigned int i = 0; i < number_devices; ++i)
    {
        tcam_device_info info = { TCAM_DEVICE_TYPE_ARAVIS, "", "", "", "" };
        std::string name = arv_get_device_id(i);
        memcpy(info.identifier, name.c_str(), name.size());

        const char* n = arv_get_device_model(i);

        if (n != NULL)
        {
            strncpy(info.name, n, sizeof(info.name) - 1);
        }
        else
        {
            SPDLOG_WARN("Unable to determine model name.");
        }

        strncpy(info.serial_number, arv_get_device_serial_nbr(i), sizeof(info.serial_number) - 1);

        device_list.push_back(DeviceInfo(info));
    }

    return device_list;
}
