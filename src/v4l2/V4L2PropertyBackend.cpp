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

#include "../logging.h"
#include "../utils.h"

#include <linux/videodev2.h>


tcam::v4l2::V4L2PropertyBackend::V4L2PropertyBackend(int fd) : p_fd(fd) {}


static outcome::result<int64_t> v4l2_control_ioctl(int fd, unsigned int request, v4l2_control* ctrl)
{
    auto ret = tcam::tcam_xioctl(fd, request, ctrl);
    if (ret >= 0)
    {
        return ctrl->value;
    }

    std::string_view action = "GET";
    if (request == VIDIOC_S_CTRL)
    {
        action = "SET";
    }

    SPDLOG_ERROR(
        "ioctl returned {} reported error while {} ({}): {}", ret, action, errno, strerror(errno));
    switch (errno)
    {
        case EBUSY:
        {
            return tcam::status::DeviceAccessBlocked;
        }
        case EINTR:
        {
            return tcam::status::UndefinedError;
        }
        case ERANGE:
        {
            return tcam::status::PropertyValueOutOfBounds;
        }
        case ETIMEDOUT:
        {
            return tcam::status::Timeout;
        }
    }
    return tcam::status::UndefinedError;
}


outcome::result<int64_t> tcam::v4l2::V4L2PropertyBackend::write_control(int v4l2_id,
                                                                            int new_value)
{
    struct v4l2_control ctrl = {};
    ctrl.id = v4l2_id;
    ctrl.value = new_value;

    return v4l2_control_ioctl(p_fd, VIDIOC_S_CTRL, &ctrl);
}


outcome::result<int64_t> tcam::v4l2::V4L2PropertyBackend::read_control(int v4l2_id)
{
    struct v4l2_control ctrl = {};
    ctrl.id = v4l2_id;

    return v4l2_control_ioctl(p_fd, VIDIOC_G_CTRL, &ctrl);
}


auto tcam::v4l2::V4L2PropertyBackend::get_menu_entries(int v4l2_id, int max)
    -> std::vector<tcam::v4l2::menu_entry>
{
    std::vector<tcam::v4l2::menu_entry> rval;
    for (int i = 0; i <= max; i++)
    {
        v4l2_querymenu qmenu = {};
        qmenu.id = v4l2_id;
        qmenu.index = i;
        if (tcam_xioctl(p_fd, VIDIOC_QUERYMENU, &qmenu))
        {
            continue;
        }
        rval.push_back({ i, std::string((char*)qmenu.name) });
    }
    return rval;
}
