/*
 * Copyright 2016 The Imaging Source Europe GmbH
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

#include "GigE3UploadItem.h"

namespace FirmwareUpdate
{
namespace GigE3
{
class IDevicePort;

struct UploadGroup
{
    IDevicePort* DestionationPort;
    std::string Name;
    uint32_t Version;
    uint32_t VersionCheckRegister;

    std::vector<UploadItem> Items;
}; /* struct UploadGroup */

} /*namespace GigE3 */\

} /* namespace FirmwareUpdate */
