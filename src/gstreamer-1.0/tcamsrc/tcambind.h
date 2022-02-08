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

#include "../../CaptureDevice.h"
#include "../../VideoFormatDescription.h"

#include <gst-helper/gst_ptr.h>
#include <gst/gst.h>

namespace tcambind
{
/**
* Separates a string of layout 'serial-type' into first == serial, second == type.
* If no '-' is found, first will contain the full input
*/
std::pair<std::string, std::string> separate_serial_and_type(const std::string& input);


gst_helper::gst_ptr<GstCaps> convert_videoformatsdescription_to_caps(
    tcam::CaptureDevice& device,
    const std::vector<tcam::VideoFormatDescription>& descriptions);

} // namespace tcambind