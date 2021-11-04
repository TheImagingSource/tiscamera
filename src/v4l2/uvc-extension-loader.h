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

#ifndef TCAM_SRC_V4L2_UVC_EXTENSION_LOADER_H
#define TCAM_SRC_V4L2_UVC_EXTENSION_LOADER_H

#include "../compiler_defines.h"

#include <functional>
#include <linux/usb/video.h>
#include <linux/uvcvideo.h>
#include <linux/videodev2.h>
#include <string>
#include <vector>

namespace tcam::uvc
{

/**
 * @name description
 * entries may be empty
 */
struct description
{
    uvc_xu_control_mapping mapping;
    std::vector<uvc_menu_info> entries;
};

/**
 * @name map
 * @param fd - file descriptor of the device
 * @param ctrl - control description that shall be used
 * @return 0 on success, -1 on error, errno will be set
 */
int map(int fd, uvc_xu_control_mapping* ctrl);


/**
 * @name determind_description_file
 * @param pid - product id of the camera that is used
 * @return filename to use, empty on error
 */
std::string determine_extension_file(const std::string& pid);


/**
 * @name apply_mappings
 * @param fd - file descriptor of the device
 * @param mappings - descriptions that shall be used
 * @param cb - callback function for warning/error messages for single mappings
 */
void apply_mappings(int fd,
                    std::vector<description>& mappings,
                    std::function<void(const std::string&)> cb);


/**
 * @name load_description_file
 * @param filename - absolute filepath of the description file
 * @param cb - callback function for error/warning messages
 * @return vector of description structs for the mapping provided by the file
 *         will be empty on error
 */
std::vector<description> load_description_file(const std::string& filename,
                                               std::function<void(const std::string&)> cb);

} // namespace tcam::uvc

#endif /* TCAM_SRC_V4L2_UVC_EXTENSION_LOADER_H */
