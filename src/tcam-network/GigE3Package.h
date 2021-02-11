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

#include "FirmwareUpgrade.h"
#include "GigE3DevicePort.h"
#include "GigE3UploadGroup.h"

#include <map>
#include <memory>
#include <pugi.h>
#include <vector>

class TiXmlDocument;

namespace FirmwareUpdate
{

namespace GigE3
{

struct UploadGroup;

class Package
{
public:
    static std::vector<std::string> FindModelNames(const std::string& packageFileName);

public:
    Status Load(const std::string& packageFileName);

    IDevicePort* find_port(const std::string& port_name);
    std::vector<UploadGroup>* find_upload_groups(const std::string& model_name);

private:
    std::string packageFileName_;

    int firmwareVersion_;
    int manifestVersion_;

    std::vector<std::shared_ptr<IDevicePort>> ports_;
    std::map<std::string, std::vector<UploadGroup>> device_types_;
    std::map<std::string, std::shared_ptr<std::vector<uint8_t>>> file_data_cache_;

private:
    Status ReadPackageInfo(const pugi::xml_document& doc);
    Status ReadDevicePorts(const pugi::xml_document& doc);
    Status ReadDeviceTypes(const pugi::xml_document& doc);
    Status ReadUploadGroup(const pugi::xml_node& uploadGroupElem, UploadGroup& group);
    Status ReadUploadItem(const pugi::xml_node& uploadItemElem, UploadItem& item);

    std::shared_ptr<IDevicePort> CreateDevicePort(const std::string& portType);

    std::shared_ptr<std::vector<uint8_t>> ExtractFile(const std::string& fileName);
}; /* class Package */

} /* namespace GigE3 */

} /* namespace FirmwareUpdate */
