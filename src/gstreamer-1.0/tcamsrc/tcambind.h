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

#pragma once

#include "../../VideoFormatDescription.h"

#include <gst/gst.h>

namespace tcambind
{

/**
* returns true if serial and type have been found
* returns false if only serial has been found
*/
bool separate_serial_and_type(const std::string& input, std::string& serial, std::string& type);

std::pair<std::string, std::string> separate_serial_and_type(const std::string& input);


GstCaps* convert_videoformatsdescription_to_caps(
    const std::vector<tcam::VideoFormatDescription>& descriptions);

} // namespace tcambind