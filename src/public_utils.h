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

#ifndef TCAM_PUBLIC_UTILS_H
#define TCAM_PUBLIC_UTILS_H

#include "base_types.h"
#include "compiler_defines.h"

#include <cstdint>
#include <string>
#include <vector>

VISIBILITY_DEFAULT

namespace tcam
{

std::vector<TCAM_DEVICE_TYPE> get_device_type_list();


std::vector<std::string> get_device_type_list_strings();


std::string tcam_device_type_to_string(TCAM_DEVICE_TYPE type);


TCAM_DEVICE_TYPE tcam_device_from_string(const std::string& str);

std::vector<tcam_image_size> get_standard_resolutions(const tcam_image_size& min,
                                                      const tcam_image_size& max);

} /* namespace tcam */

VISIBILITY_POP

#endif /* TCAM_PUBLIC_UTILS_H */
