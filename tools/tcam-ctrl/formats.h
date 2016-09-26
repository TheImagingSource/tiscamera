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

#ifndef FORMATS_H
#define FORMATS_H

#include "tcam.h"

#include <vector>
#include <string>
#include <memory>

using namespace tcam;


/**
 * @brief print function for VideoFormatDescriptions
 * @param available_formats - format descriptions that shell be printed
 */
void list_formats (const std::vector<VideoFormatDescription>& available_formats);


/**
 * @brief print function for VideoFormatDescriptions as gstreamer-1.0 caps
 * @param available_formats - format descriptions that shell be printed
 */
void list_gstreamer_1_0_formats (const std::vector<VideoFormatDescription>& available_formats);


/**
 * @brief print function for VideoFormat
 * @param format - VideoFormat that shall be printed
 */
void print_active_format (const VideoFormat& format);


/**
 * @brief Set video format for device
 * @param g - CaptureDevice of the device which shall be used
 * @param new_format - string describing the format that shall be set
 * @return true on success
 */
bool set_active_format (std::shared_ptr<CaptureDevice> dev, const std::string& new_format);

#endif /* FORMATS_H */
