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

#include "GigE3Package.h"

#include "FirmwarePackage.h"
#include "GigE3DevicePortFlashMemory.h"
#include "GigE3DevicePortMachXO2.h"
#include "GigE3UploadGroup.h"
#include "GigE3UploadItem.h"

#include <memory>
#include <pugi.h>

using namespace FirmwareUpdate;


std::vector<std::string> GigE3::Package::FindModelNames(const std::string& packageFileName)
{
    std::vector<std::string> result;

    pugi::xml_document doc;
    auto load_res =
        doc.load_string(FirmwarePackage::extractTextFile(packageFileName, "manifest.xml").c_str());

    if (load_res != pugi::status_ok)
    {
        return result;
    }

    auto deviceTypeElement = doc.child("FirmwarePackage").child("DeviceTypes").child("DeviceType");
    for (; deviceTypeElement; deviceTypeElement = deviceTypeElement.next_sibling("DeviceType"))
    {
        auto modelName = deviceTypeElement.attribute("Name").as_string();
        if (modelName)
        {
            result.push_back(modelName);
        }
    }
    return result;
}


GigE3::IDevicePort* GigE3::Package::find_port(const std::string& port_name)
{
    for (auto&& port : ports_)
    {
        if (port->name() == port_name)
        {
            return port.get();
        }
    }
    return nullptr;
}


std::vector<GigE3::UploadGroup>* GigE3::Package::find_upload_groups(const std::string& model_name)
{
    auto it = device_types_.find(model_name);
    if (it == device_types_.end())
    {
        return nullptr;
    }
    return &it->second;
}


FirmwareUpdate::Status GigE3::Package::Load(const std::string& packageFileName)
{
    packageFileName_ = packageFileName;

    pugi::xml_document doc;
    auto manifest = FirmwarePackage::extractTextFile(packageFileName, "manifest.xml");
    auto load_res = doc.load_string(manifest.c_str());

    if (load_res.status != pugi::status_ok)
    {
        return Status::InvalidFile;
    }

    auto status = ReadPackageInfo(doc);

    if (failed(status))
    {
        return status;
    }
    status = ReadDevicePorts(doc);

    if (failed(status))
    {
        return status;
    }
    status = ReadDeviceTypes(doc);

    if (failed(status))
    {
        return status;
    }
    return Status::Success;
}


FirmwareUpdate::Status GigE3::Package::ReadPackageInfo(const pugi::xml_document& doc)
{
    auto root = doc.child("FirmwarePackage");

    firmwareVersion_ = root.attribute("FirmwareVersion").as_int();

    if (firmwareVersion_ <= 0)
    {
        return Status::InvalidFile;
    }

    manifestVersion_ = root.attribute("ManifestVersion").as_int();

    // Don't know how to handle manifest > 1
    if (manifestVersion_ > 1)
    {
        return Status::InvalidFile;
    }
    return Status::Success;
}


FirmwareUpdate::Status GigE3::Package::ReadDevicePorts(const pugi::xml_document& doc)
{
    ports_.clear();

    auto portElement = doc.child("FirmwarePackage").child("DevicePorts").child("DevicePort");
    for (; portElement; portElement = portElement.next_sibling("DevicePort"))
    {
        auto portName = portElement.attribute("Name").as_string();
        auto portType = portElement.attribute("Type").as_string();
        auto portConfigElem = portElement.child("PortConfiguration");

        if (!portName || !portType || !portConfigElem)
        {
            return Status::InvalidFile;
        }
        auto port = CreateDevicePort(portType);
        if (!port)
        {
            return Status::InvalidFile;
        }
        auto status = port->Configure(portName, portConfigElem);
        if (failed(status))
        {
            return status;
        }
        ports_.push_back(std::move(port));
    }

    return Status::Success;
}


FirmwareUpdate::Status GigE3::Package::ReadDeviceTypes(const pugi::xml_document& doc)
{
    device_types_.clear();

    auto deviceTypeElement = doc.child("FirmwarePackage").child("DeviceTypes").child("DeviceType");
    for (; deviceTypeElement; deviceTypeElement = deviceTypeElement.next_sibling("DeviceType"))
    {
        auto modelName = deviceTypeElement.attribute("Name").as_string();
        if (!modelName)
        {
            return Status::InvalidFile;
        }
        std::vector<UploadGroup> groups;

        auto uploadGroupElem = deviceTypeElement.child("UploadGroup");
        for (; uploadGroupElem; uploadGroupElem = uploadGroupElem.next_sibling("UploadGroup"))
        {
            UploadGroup group;

            auto status = ReadUploadGroup(uploadGroupElem, group);
            if (failed(status))
            {
                return status;
            }
            groups.push_back(std::move(group));
        }

        device_types_[modelName] = std::move(groups);
    }

    return Status::Success;
}


static bool parseHexOrDecimal(const char* text, uint32_t& val)
{
    if (sscanf(text, "0x%x", &val))
    {
        return true;
    }
    if (sscanf(text, "%u", &val))
    {
        return true;
    }

    return false;
}


static bool parseAttribute(const pugi::xml_node& elem, const char* attributeName, uint32_t& val)
{
    auto attrText = elem.attribute(attributeName).as_string();
    if (!attrText)
    {
        return false;
    }
    return parseHexOrDecimal(attrText, val);
}


FirmwareUpdate::Status GigE3::Package::ReadUploadGroup(const pugi::xml_node& uploadGroupElem,
                                                       UploadGroup& group)
{
    auto groupName = uploadGroupElem.attribute("Name").as_string();
    auto destinationName = uploadGroupElem.attribute("Destination").as_string();
    if (!groupName || !destinationName)
    {
        return Status::InvalidFile;
    }
    auto port = find_port(destinationName);
    if (!port)
    {
        return Status::InvalidFile;
    }
    if (!parseAttribute(uploadGroupElem, "VersionCheckRegister", group.VersionCheckRegister))
    {
        return Status::InvalidFile;
    }
    if (!parseAttribute(uploadGroupElem, "Version", group.Version))
    {
        return Status::InvalidFile;
    }
    group.DestionationPort = port;
    group.Name = groupName;

    auto uploadItemElem = uploadGroupElem.child("Upload");
    for (; uploadItemElem != nullptr; uploadItemElem = uploadItemElem.next_sibling("Upload"))
    {
        UploadItem item;

        auto status = ReadUploadItem(uploadItemElem, item);
        if (failed(status))
        {
            return status;
        }
        group.Items.push_back(std::move(item));
    }

    auto status = port->CheckItems(group.Items);
    if (failed(status))
    {
        return status;
    }
    return Status::Success;
}


FirmwareUpdate::Status GigE3::Package::ReadUploadItem(const pugi::xml_node& uploadItemElem,
                                                      UploadItem& item)
{
    for (auto attr : uploadItemElem.attributes())
    {
        if (attr.name() == std::string("File"))
        {
            if (item.Data != nullptr)
            {
                return Status::InvalidFile; // No two "data" elements allowed
            }
            item.Data = ExtractFile(attr.value());
            if (item.Data->empty()) {}
        }
        else if (attr.name() == std::string("String"))
        {
            if (item.Data != nullptr)
            {
                return Status::InvalidFile; // No two "data" elements allowed
            }
            auto str = std::string(attr.value());
            item.Data = std::make_shared<std::vector<uint8_t>>();
            item.Data->resize(str.length());
            memcpy(item.Data->data(), str.data(), str.length());
        }
        else if (attr.name() == std::string("U32"))
        {
            if (item.Data != nullptr)
            {
                return Status::InvalidFile; // No two "data" elements allowed
            }
            uint32_t data = 0;
            if (!parseHexOrDecimal(attr.value(), data))
            {
                return Status::InvalidFile;
            }
            item.Data = std::make_shared<std::vector<uint8_t>>();
            item.Data->resize(sizeof(data));
            memcpy(item.Data->data(), &data, sizeof(data));
        }
        else
        {
            uint32_t val = 0;
            if (!parseHexOrDecimal(attr.value(), val))
            {
                return Status::InvalidFile;
            }
            item.Params[attr.name()] = val;
        }
    }

    auto len = item.Params.find("Length");
    if (len != item.Params.end())
    {
        item.Data->resize(len->second, 0);
    }

    return Status::Success;
}


std::shared_ptr<GigE3::IDevicePort> GigE3::Package::CreateDevicePort(const std::string& portType)
{
    if (portType == "Flash")
    {
        return std::make_shared<GigE3::DevicePortFlashMemory>();
    }
    if (portType == "MachXO2")
    {
        return std::make_shared<GigE3::DevicePortMachXO2>();
    }
    return nullptr;
}

std::shared_ptr<std::vector<uint8_t>> GigE3::Package::ExtractFile(const std::string& fileName)
{
    auto& cached = file_data_cache_[fileName];
    if (cached == nullptr)
    {
        cached = std::make_shared<std::vector<uint8_t>>(
            FirmwarePackage::extractFile(packageFileName_, fileName));
    }
    return cached;
}
