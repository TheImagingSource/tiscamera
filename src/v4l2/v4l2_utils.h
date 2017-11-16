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

#ifndef TCAM_V4L2_UTILS_H
#define TCAM_V4L2_UTILS_H

#include "Properties.h"
#include "DeviceInfo.h"

#include <linux/videodev2.h>

#include "compiler_defines.h"

VISIBILITY_INTERNAL

namespace tcam
{

uint32_t convert_v4l2_flags (uint32_t v4l2_flags);

/**
 * @brief Create Property and return shared_ptr to base class
 *
 * @param fd - file descriptor for responsible device
 * @param queryctrl
 * @param ctrl
 * @param impl - shared_ptr of the responsible implementation
 *
 * @return shared_ptr to newly created Property; nullptr on failure
 */
std::shared_ptr<Property> create_property (int fd,
                                          struct v4l2_queryctrl* queryctrl,
                                          struct v4l2_ext_control* ctrl,
                                          std::shared_ptr<PropertyImpl> impl);


/**
 * @name get_v4l2_device_list
 * @brief lists all supported v4l2 devices
 * @return vector containing all found v4l2 devices
 */
std::vector<DeviceInfo> get_v4l2_device_list ();

} /* namespace tcam */

VISIBILITY_POP

#endif /* TCAM_V4L2_UTILS_H */
