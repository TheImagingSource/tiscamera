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

#ifndef TCAM_SERIALIZATION_H
#define TCAM_SERIALIZATION_H


#include "CaptureDevice.h"

#include <string>

namespace tcam
{

/**
 * @brief store given list of capture devices in the specified file
 * @param[in] device_list - devices that shall be stored as xml
 * @param[in] filename - file in which the created xml structure shall be stored
 * @return true on success, else false
 */
bool export_device_list (const std::vector<DeviceInfo>& device_list,
                         const std::string& filename);


bool load_xml_description (const std::string& filename,
                           const DeviceInfo& device,
                           VideoFormat& format,
                           std::vector<std::shared_ptr<Property>>& properties);

bool save_xml_description (const std::string& filename,
                           const DeviceInfo& device,
                           const VideoFormat& format,
                           const std::vector<std::shared_ptr<Property>>& properties);

} /* namespace tcam */

#endif /* TCAM_SERIALIZATION_H */
