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

#include "../logging.h"
#include "../utils.h"

#include <algorithm> // std::find
#include <arv.h>
#include <dutils_img/image_fourcc.h>
#include <optional>
#include <regex>

// gige-daemon communication

#include "../../tools/tcam-gige-daemon/gige-daemon.h"
#include "../tcam-semaphores.h"

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

using namespace tcam;
using namespace tcam::tools;

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
    { FOURCC_POLARIZATION_BG12_PACKED_90_45_135_0, 0x810C0011 }, // GigE
    { FOURCC_POLARIZATION_BG12_PACKED_90_45_135_0, 0x810C000E }, // USB3Vision
    { FOURCC_POLARIZATION_BG16_90_45_135_0, 0x8110000F },

    { FOURCC_POLARIZATION_MONO8_90_45_135_0, 0x8108000A },
    { FOURCC_POLARIZATION_MONO12_PACKED_90_45_135_0, 0x810C0010 }, // GigE
    { FOURCC_POLARIZATION_MONO12_SPACKED_90_45_135_0, 0x810C000B }, // USB3Vision
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


static bool not_using_gige_deamon_message_reported = false;


bool is_blacklisted_gige(const std::string& manufacturer_info)
{
    if (tcam::is_environment_variable_set("TCAM_DISABLE_DEVICE_BLACKLIST"))
    {
        return false;
    }

    // Strings will in format:
    // @Type=1@Model=MT9P031 C T@
    // @Type=1@Model=MT9P031 C B@
    // @Type=1@Model=MT9P031 C Z12DC@

    // type=1 --> 23g
    // type=2 --> 23g
    // Z --> zoom
    // B --> zoom --> board camera

    // regex can be really cumbersome
    // blacklist devices,
    // but allow blacklisted devices with certain properties through

    static const std::regex blacklist [] = {
        std::regex(".*@Type=1@.*"),
        std::regex(".*@Type=2@.*"),
    };

    static const std::regex whitelist[] = {
        std::regex(".*\\sZ.*"),
        std::regex(".* B.*"),
    };

    for (const auto& bl_entry : blacklist)
    {
        if (std::regex_match(manufacturer_info, bl_entry))
        {
            for (const auto& wb_entry : whitelist)
            {
                if (std::regex_match(manufacturer_info, wb_entry))
                {
                    return false;
                }
            }
            return true;
        }
    }

    return false;
}


static std::optional<std::vector<DeviceInfo>> fetch_gige_daemon_device_list()
{
    try
    {
        key_t shmkey = ftok(gige_daemon::LOCK_FILE, 'G');
        if (shmkey == -1)
        {
            if (!not_using_gige_deamon_message_reported) // print this message only once and not every time we are queried
            {
                SPDLOG_INFO("Failed to create shmkey. Not using gige-daemon to enumerate devices.");
                not_using_gige_deamon_message_reported = true;
            }
            return std::nullopt;
        }

        not_using_gige_deamon_message_reported =
            false; // reset message when fetching the lock worked

        key_t shm_semaphore_key = ftok(gige_daemon::LOCK_FILE, 'S');
        if (shm_semaphore_key == -1)
        {
            SPDLOG_INFO("Failed to create shmkey. errno={}.", errno);
            return std::nullopt;
        }

        int shm_mem_id = shmget(shmkey, sizeof(gige_daemon::tcam_gige_device_list), 0644);
        if (shm_mem_id < 0)
        {
            SPDLOG_INFO("Unable to connect to gige-daemon. Using internal methods");
            return std::nullopt;
        }

        semaphore semaphore_instance = semaphore::create(shm_semaphore_key);
        std::lock_guard<semaphore> lck(semaphore_instance);

        auto ptr = shmat(shm_mem_id, NULL, 0);
        if (ptr == ((void*)-1))
        {
            SPDLOG_ERROR("shmat failed to map memory. errno={}", errno);
            return std::nullopt;
        }
        const gige_daemon::tcam_gige_device_list* shared_mem_list =
            static_cast<const gige_daemon::tcam_gige_device_list*>(ptr);
        if (shared_mem_list == nullptr)
        {
            SPDLOG_ERROR("shmat returned nullptr.");
            return std::nullopt;
        }

        std::vector<DeviceInfo> ret;

        ret.reserve(shared_mem_list->device_count);

        for (unsigned int i = 0; i < shared_mem_list->device_count; ++i)
        {
            ret.push_back(DeviceInfo(shared_mem_list->devices[i]));
        }

        shmdt(shared_mem_list);

        return ret;
    }
    catch (const std::exception& ex)
    {
        SPDLOG_INFO("Failed to fetch devices from gige-daemon, due to exception={}.", ex.what());
    }
    return std::nullopt;
}

/*
 * the following is for tracking
 * potential device losses
 * and adding tolerances towards
 * a non responce
 */

static const int lost_count_max = 10;

struct dev_life_tracking
{
    tcam::DeviceInfo dev;
    int lost_count;
};

static std::vector<dev_life_tracking> dev_life_tracking_list;


std::vector<DeviceInfo> tcam::get_gige_device_list()
{
    std::vector<DeviceInfo> current_devices;
    auto dev_list = fetch_gige_daemon_device_list();
    if (dev_list)
    {
        current_devices = dev_list.value();
    }
    else
    {
        current_devices = get_aravis_device_list();
    }

    // check for new devices
    // to be added to out watch current_devices
    // after everything else is done
    std::vector<DeviceInfo> to_add;
    for (auto& entry : current_devices)
    {
        auto known_dev =
            std::find_if(dev_life_tracking_list.begin(),
                         dev_life_tracking_list.end(),
                         [entry](const dev_life_tracking& d) { return d.dev == entry; });

        // known device
        if (known_dev != dev_life_tracking_list.end())
        {
            known_dev->lost_count = 0;
        }
        else
        {
            to_add.push_back(entry);
        }
    }

    // iterate known devices
    // increase loss count if necessary
    // and add to current_devices if considered still alive
    for (auto& entry : dev_life_tracking_list)
    {
        if (!std::any_of(current_devices.begin(),
                         current_devices.end(),
                         [entry](const DeviceInfo& d) { return d == entry.dev; }))
        {
            auto is_gige_dev = [](const DeviceInfo& dev)
            {
                std::string info = dev.get_info().additional_identifier;

                if (info == "USB3")
                {
                    return false;
                }
                return true;
            };

            if (!is_gige_dev(entry.dev))
            {
                continue;
            }

            entry.lost_count++;

            if (entry.lost_count < lost_count_max)
            {
                current_devices.push_back(entry.dev);
            }
        }
    }

    // add new entries
    for (auto& entry : to_add) { dev_life_tracking_list.push_back({ entry, 0 }); }

    // remove deprecated entried
    std::remove_if(dev_life_tracking_list.begin(),
                   dev_life_tracking_list.end(),
                   [](const auto& entry) { return entry.lost_count >= lost_count_max; });

    return current_devices;
}

unsigned int tcam::get_gige_device_count()
{
    return get_gige_device_list().size();
}

std::vector<DeviceInfo> tcam::get_aravis_device_list()
{
    arv_update_device_list();

    unsigned int number_devices = arv_get_n_devices();
    if (number_devices == 0)
    {
        return {};
    }

    std::vector<DeviceInfo> device_list;
    for (unsigned int i = 0; i < number_devices; ++i)
    {
        if (is_blacklisted_gige(arv_get_device_manufacturer_info(i)))
        {
            SPDLOG_DEBUG("{} is not a supported device type. Filtering... {}",
                         arv_get_device_id(i),
                         arv_get_device_manufacturer_info(i));
            continue;
        }

        tcam_device_info info = { TCAM_DEVICE_TYPE_ARAVIS, "", "", "", "" };
        auto device_id = arv_get_device_id(i);
        if (!device_id)
        {
            SPDLOG_WARN("Failed to fetch device_id for aravis device index #{}.", i);
            continue;
        }
        memcpy(info.identifier, device_id, sizeof(info.identifier) - 1);

        if (const auto device_model = arv_get_device_model(i); device_model != nullptr)
        {
            strncpy(info.name, device_model, sizeof(info.name) - 1);
        }
        else
        {
            SPDLOG_WARN("Unable to determine model name for device='{}'.", device_id);
        }

        if (auto serial = arv_get_device_serial_nbr(i); serial)
        {
            strncpy(info.serial_number, serial, sizeof(info.serial_number) - 1);
        }

        strncpy(info.additional_identifier,
                arv_get_device_address(i),
                sizeof(info.additional_identifier) - 1);

        device_list.push_back(DeviceInfo(info));
    }
    return device_list;
}

tcam::status tcam::aravis::translate_error(ArvDeviceError err)
{
    switch (err)
    {
        case ARV_DEVICE_ERROR_WRONG_FEATURE:
            return tcam ::status::PropertyNotImplemented;
        case ARV_DEVICE_ERROR_FEATURE_NOT_FOUND:
            return tcam ::status::PropertyNotImplemented;
        case ARV_DEVICE_ERROR_NOT_CONNECTED:
            return tcam ::status::DeviceAccessBlocked;
        case ARV_DEVICE_ERROR_PROTOCOL_ERROR:
            return tcam ::status::UndefinedError;
        case ARV_DEVICE_ERROR_TRANSFER_ERROR:
            return tcam ::status::UndefinedError;
        case ARV_DEVICE_ERROR_TIMEOUT:
            return tcam ::status::Timeout;
        case ARV_DEVICE_ERROR_NOT_FOUND:
            return tcam ::status::UndefinedError;
        case ARV_DEVICE_ERROR_INVALID_PARAMETER:
            return tcam::status::InvalidParameter;
        case ARV_DEVICE_ERROR_GENICAM_NOT_FOUND:
            return tcam::status::DeviceCouldNotBeOpened;
        case ARV_DEVICE_ERROR_NO_STREAM_CHANNEL:
            return tcam::status::DeviceCouldNotBeOpened;
        case ARV_DEVICE_ERROR_NOT_CONTROLLER:
            return tcam::status::DeviceAccessBlocked;
        case ARV_DEVICE_ERROR_UNKNOWN:
            return tcam::status::UndefinedError;
    }
    return tcam::status::UndefinedError;
}

tcam::status tcam::aravis::translate_error(ArvGcError err)
{
    switch (err)
    {
        case ARV_GC_ERROR_PROPERTY_NOT_DEFINED:
            return tcam ::status::PropertyNotImplemented;
        case ARV_GC_ERROR_PVALUE_NOT_DEFINED:
            return tcam ::status::PropertyNotImplemented;
        case ARV_GC_ERROR_INVALID_PVALUE:
            return tcam ::status::PropertyNotImplemented;
        case ARV_GC_ERROR_EMPTY_ENUMERATION:
            return tcam ::status::UndefinedError;
        case ARV_GC_ERROR_OUT_OF_RANGE:
            return tcam ::status::PropertyValueOutOfBounds;
        case ARV_GC_ERROR_NO_DEVICE_SET:
            return tcam ::status::DeviceCouldNotBeOpened;
        case ARV_GC_ERROR_NO_EVENT_IMPLEMENTATION:
            return tcam ::status::UndefinedError;
        case ARV_GC_ERROR_NODE_NOT_FOUND:
            return tcam ::status::PropertyNotImplemented;
        case ARV_GC_ERROR_ENUM_ENTRY_NOT_FOUND:
            return tcam ::status::PropertyValueOutOfBounds;
        case ARV_GC_ERROR_INVALID_LENGTH:
            return tcam ::status::InvalidParameter;
        case ARV_GC_ERROR_READ_ONLY:
            return tcam ::status::PropertyNotWriteable;
        case ARV_GC_ERROR_SET_FROM_STRING_UNDEFINED:
            return tcam::status::UndefinedError;
        case ARV_GC_ERROR_GET_AS_STRING_UNDEFINED:
            return tcam::status::UndefinedError;
        case ARV_GC_ERROR_INVALID_BIT_RANGE:
            return tcam::status::UndefinedError;
    }
    return tcam::status::UndefinedError;
}

tcam::status tcam::aravis::consume_GError(GError*& err)
{
    if (err == nullptr)
        return tcam::status::Success;

    tcam::status code = tcam::status::UndefinedError;

    if (err->domain == arv_device_error_quark())
    {
        code = tcam::aravis::translate_error(static_cast<ArvDeviceError>(err->code));
    }
    else if (err->domain == arv_gc_error_quark())
    {
        code = tcam::aravis::translate_error(static_cast<ArvGcError>(err->code));
    }
    else
    {
        code = tcam::status::UndefinedError;
    }

    g_error_free(err);
    err = nullptr;

    return code;
}
