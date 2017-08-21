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

#ifndef TCAM_ARAVIS_UTILS_H
#define TCAM_ARAVIS_UTILS_H

#include "Properties.h"

#include "arv.h"

#include "DeviceInfo.h"

#include "compiler_defines.h"

VISIBILITY_INTERNAL

namespace tcam
{

std::shared_ptr<Property> create_property (ArvCamera* camera,
                                          ArvGcNode* node,
                                          std::shared_ptr<PropertyImpl> impl);


uint32_t aravis2fourcc (uint32_t aravis);
uint32_t fourcc2aravis (uint32_t fourcc);

unsigned int get_gige_device_count ();

std::vector<DeviceInfo> get_gige_device_list ();

unsigned int get_aravis_device_count ();

std::vector<DeviceInfo> get_aravis_device_list ();

}; /* namespace tcam */

VISIBILITY_POP

#endif /* TCAM_ARAVIS_UTILS_H */
