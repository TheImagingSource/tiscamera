/*
 * Copyright 2021 The Imaging Source Europe GmbH
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

#include "V4L2PropertyBackend.h"

#include "logging.h"
#include "utils.h"

#include <linux/videodev2.h>


tcam::property::V4L2PropertyBackend::V4L2PropertyBackend(int fd) : p_fd(fd) {}


int tcam::property::V4L2PropertyBackend::write_control(int v4l2_id, int new_value)
{
    struct v4l2_control ctrl = {};
    ctrl.id = v4l2_id;
    ctrl.value = new_value;

    return tcam_xioctl(p_fd, VIDIOC_S_CTRL, &ctrl);
}


int tcam::property::V4L2PropertyBackend::read_control(int v4l2_id, int64_t& new_value)
{
    struct v4l2_control ctrl = {};
    ctrl.id = v4l2_id;

    if (tcam_xioctl(p_fd, VIDIOC_G_CTRL, &ctrl) != 0)
    {
        SPDLOG_WARN("Could not retrieve current value. ioctl return '{}'",
                    //              desc.prop->get_name().c_str(),
                    strerror(errno));
        return -1;
    }
    new_value = ctrl.value;
    return 0;
}


std::map<int, std::string> tcam::property::V4L2PropertyBackend::get_menu_entries(int v4l2_id,
                                                                                 int max)
{
    std::map<int, std::string> entries;

    struct v4l2_querymenu qmenu = {};
    qmenu.id = v4l2_id;

    for (int i = 0; i <= max; i++)
    {
        qmenu.index = i;
        if (tcam_xioctl(p_fd, VIDIOC_QUERYMENU, &qmenu))
            continue;

        std::string map_string((char*)qmenu.name);
        entries.emplace(i, map_string);
    }

    return entries;
}
