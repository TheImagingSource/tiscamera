/*
 * Copyright 2018 The Imaging Source Europe GmbH
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

#include "uvc-extension-loader.h"

#include "src/tcam-config.h"

#include "json.hpp"

#include <fcntl.h>
#include <fstream>
#include <limits.h> // LONG_MAX
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <uuid/uuid.h>
#include <stdlib.h> // getenv

using json = nlohmann::json;


// copied from src/utils.cpp tcam_xioctl
// that function was the only reason for linking
// libtcam.so. This way the extension unit loader
// can be built seperately.
static int xioctl(int fd, unsigned int request, void* arg)
{
    constexpr int IOCTL_RETRY = 4;

    int ret = 0;
    int tries = IOCTL_RETRY;
    do {
        ret = ioctl(fd, request, arg);
        // ret = v4l2_ioctl(fd, request, arg);
    } while (ret && tries-- && ((errno == EINTR) || (errno == EAGAIN) || (errno == ETIMEDOUT)));

    return (ret);
}


int tcam::uvc::map(int fd, uvc_xu_control_mapping* ctrl)
{
    return xioctl(fd, UVCIOC_CTRL_MAP, ctrl);
}


void tcam::uvc::apply_mappings(int fd,
                               std::vector<tcam::uvc::description>& mappings,
                               std::function<void(const std::string&)> cb)
{
    for (auto& m : mappings)
    {
        if (m.mapping.v4l2_type == V4L2_CTRL_TYPE_MENU)
        {
            m.mapping.menu_info = m.entries.data();
            m.mapping.menu_count = m.entries.size();
        }

        int ret = map(fd, &m.mapping);

        if (ret != 0)
        {
            std::string msg = "Error while mapping '" + std::string((char*)m.mapping.name)
                              + "': errno: " + std::to_string(errno) + " - " + strerror(errno);
            cb(msg);
        }
    }
}


static __u8 string_to_u8(const std::string& input)
{
    char* p;
    long n = strtol(input.c_str(), &p, 16);

    // this means an error occured
    if (n == LONG_MAX || n == LONG_MIN || n > UCHAR_MAX)
    {
        return 0;
    }

    return n;
}


static __u32 string_to_u32(const std::string& input)
{
    char* p;
    long n = strtol(input.c_str(), &p, 16);

    // this means an error occured
    if (n == LONG_MAX || n == LONG_MIN)
    {
        return 0;
    }

    return n;
}


static __u8 parse_uvc_type(const std::string& str)
{
    if (str == "unsigned")
    {
        return UVC_CTRL_DATA_TYPE_UNSIGNED;
    }
    else if (str == "signed")
    {
        return UVC_CTRL_DATA_TYPE_SIGNED;
    }
    else if (str == "raw")
    {
        return UVC_CTRL_DATA_TYPE_RAW;
    }
    else if (str == "enum")
    {
        return UVC_CTRL_DATA_TYPE_ENUM;
    }
    else if (str == "boolean")
    {
        return UVC_CTRL_DATA_TYPE_BOOLEAN;
    }
    else if (str == "bitmask")
    {
        return UVC_CTRL_DATA_TYPE_BITMASK;
    }

    return 0;
}


static __u8 parse_v4l2_type(const std::string& str)
{
    if (str == "bitmask")
    {
        return V4L2_CTRL_TYPE_BITMASK;
    }
    else if (str == "boolean")
    {
        return V4L2_CTRL_TYPE_BOOLEAN;
    }
    else if (str == "button")
    {
        return V4L2_CTRL_TYPE_BUTTON;
    }
    else if (str == "integer")
    {
        return V4L2_CTRL_TYPE_INTEGER;
    }
    else if (str == "menu")
    {
        return V4L2_CTRL_TYPE_MENU;
    }
    else if (str == "string")
    {
        return V4L2_CTRL_TYPE_STRING;
    }

    return 0;
}

static std::vector<std::string> get_search_directories()
{
    std::vector<std::string> directories;

    directories.push_back(get_current_dir_name());

    char* env_entry = getenv("TCAM_UVC_EXTENSION_DIR");
    if (env_entry)
    {
        directories.push_back(env_entry);
    }

    directories.push_back(TCAM_INSTALL_UVC_EXTENSION);

    return directories;
}


std::string tcam::uvc::determine_extension_file(const std::string& pid_in)
{
    std::string filename;

    // interpret nuber as hex
    int pid = strtol(pid_in.c_str(), NULL, 16);

    const int PID_BASE_MASK_33U = 0xFC00;
    const int PID_BASE_MASK_LEGACY = 0xFF00;

    if ((pid & PID_BASE_MASK_33U) == 0x9000
        || (pid & PID_BASE_MASK_33U) == 0x9800)
    {
        // usb 33/38
        filename = "usb33.json";
    }
    else if ((pid & PID_BASE_MASK_33U) == 0x9400)
    {
        // usb 37
        filename = "usb37.json";
    }
    else if ((pid & PID_BASE_MASK_LEGACY) == 0x8300)
    {
        filename = "usb2.json";
    }
    else if ((pid & PID_BASE_MASK_LEGACY) == 0x8400
             || (pid & PID_BASE_MASK_LEGACY) == 0x8500
             || (pid & PID_BASE_MASK_LEGACY) == 0x8600
             || (pid & PID_BASE_MASK_LEGACY) == 0x8700)
    {
        // usb 23
        filename = "usb23.json";
    }

    if (filename.empty())
    {
        return filename;
    }

    std::vector<std::string> search_path = get_search_directories();

    // old school method
    // std::filesystem not used for backwards compatability
    for (const auto& sp : search_path)
    {
        std::string name = sp + "/" + filename;
        struct stat buffer;
        if (stat (name.c_str(), &buffer) == 0)
        {
            return name;
        }
    }
    return "";

}


std::vector<tcam::uvc::description> tcam::uvc::load_description_file(
    const std::string& filename,
    std::function<void(const std::string&)> cb)
{
    static const int max_string_length = 31;

    uuid_t guid;

    std::vector<description> mappings;

    std::ifstream ifs(filename);

    if (!ifs.good())
    {
        return mappings;
    }

    std::string tmp_content(std::istreambuf_iterator<char>{ifs}, {});

    json json_desc;

    try
    {
        json_desc = json::parse(tmp_content,
                                nullptr, // callback
                                true, // allow exceptions
                                true); //ignore comments
    }
    catch (json::parse_error& err)
    {
        std::string msg =
            std::string(err.what()) + " - This occurred around byte " + std::to_string(err.byte);
        cb(msg);
        return mappings;
    }

    uuid_parse(json_desc.at("guid").get<std::string>().c_str(), guid);

    for (const auto& m : json_desc.at("mappings"))
    {
        struct description desc;
        desc.mapping = {};

        auto& map = desc.mapping;

        map.id = string_to_u32(m.at("id").get<std::string>());

        if (map.id == 0)
        {
            std::string msg =
                "Could not convert id field to valid u8 for " + m.at("name").get<std::string>();
            cb(msg);
            continue;
        }

        if (m.at("name").get<std::string>().size() > max_string_length)
        {
            std::string msg = "V4L2 name is to long! " + m.at("name").get<std::string>()
                              + " will be cut off. Reduce to 31 Characters or less.";
            cb(msg);
        }
        strcpy((char*)map.name, m.at("name").get<std::string>().c_str());

        map.selector = string_to_u8(m.at("selector").get<std::string>());

        map.size = m.at("size_bits").get<int>();
        map.offset = m.at("offset_bits").get<int>();

        map.data_type = parse_uvc_type(m.at("uvc_type").get<std::string>());

        map.v4l2_type = parse_v4l2_type(m.at("v4l2_type").get<std::string>());
        if (map.v4l2_type == 0)
        {
            std::string msg =
                "v4l2_type for '" + m.at("name").get<std::string>() + "' does not make sense.";
            cb(msg);
            continue;
        }

        memcpy(map.entity, guid, sizeof(map.entity));

        if (map.v4l2_type == V4L2_CTRL_TYPE_MENU)
        {
            for (auto& e : m.at("entries"))
            {
                uvc_menu_info entry = {};

                entry.value = e.at("value").get<int>();
                // TODO length check

                std::string str = e.at("entry").get<std::string>();

                if (str.size() > max_string_length)
                {
                    std::string msg =
                        "Menu Entry '" + str + "' is too long. Reduce to 31 Characters or less.";
                    cb(msg);
                }

                strcpy((char*)entry.name, str.c_str());

                desc.entries.push_back(entry);
            }
        }
        mappings.push_back(desc);
    }

    return mappings;
}
