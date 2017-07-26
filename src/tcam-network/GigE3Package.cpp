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

#include "GigE3DevicePortFlashMemory.h"
#include "GigE3DevicePortMachXO2.h"
#include "GigE3UploadGroup.h"
#include "GigE3UploadItem.h"

#include "FirmwarePackage.h"

#include <tinyxml.h>

#include <memory>

using namespace FirmwareUpdate;


std::vector<std::string> GigE3::Package::FindModelNames (const std::string& packageFileName)
{
    std::vector<std::string> result;

    auto xmlManifestData = FirmwarePackage::extractTextFile(packageFileName, "Manifest.xml");

    TiXmlDocument xdoc;
    xdoc.Parse(xmlManifestData.c_str());
    if (xdoc.Error())
    {
        return result;
    }

    TiXmlHandle docHandle(const_cast<TiXmlDocument*>(&xdoc));

    auto deviceTypeElement = docHandle.FirstChild("FirmwarePackage").FirstChild("DeviceTypes").FirstChild("DeviceType").ToElement();
    for ( ; deviceTypeElement; deviceTypeElement = deviceTypeElement->NextSiblingElement("DeviceType"))
    {
        auto modelName = deviceTypeElement->Attribute("Name");
        if (modelName)
        {
            result.push_back(modelName);
        }
    }
    return result;
}


GigE3::IDevicePort* GigE3::Package::find_port (const std::string& port_name)
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


std::vector<GigE3::UploadGroup>* GigE3::Package::find_upload_groups (const std::string& model_name)
{
    auto it = device_types_.find (model_name);
    if (it == device_types_.end())
    {
        return nullptr;
    }
    return &it->second;
}


FirmwareUpdate::Status GigE3::Package::Load (const std::string& packageFileName)
{
    packageFileName_ = packageFileName;

    auto xmlManifestData = FirmwarePackage::extractTextFile(packageFileName_, "manifest.xml");

    TiXmlDocument xdoc;
    xdoc.Parse(xmlManifestData.c_str());

    if (xdoc.Error())
    {
        return Status::InvalidFile;
    }

    auto status = ReadPackageInfo(xdoc);

    if (failed(status))
    {
        return status;
    }
    status = ReadDevicePorts(xdoc);

    if (failed(status))
    {
        return status;
    }
    status = ReadDeviceTypes(xdoc);

    if (failed(status))
    {
        return status;
    }
    return Status::Success;
}


FirmwareUpdate::Status GigE3::Package::ReadPackageInfo (const TiXmlDocument& doc)
{
    if (!doc.RootElement()->Attribute("FirmwareVersion", &firmwareVersion_))
    {
        return Status::InvalidFile;
    }
    if (!doc.RootElement()->Attribute("ManifestVersion", &manifestVersion_))
    {
        return Status::InvalidFile;
    }
    // Don't know how to handle manifest > 1
    if (manifestVersion_ > 1)
    {
        return Status::InvalidFile;
    }
    return Status::Success;
}


FirmwareUpdate::Status GigE3::Package::ReadDevicePorts (const TiXmlDocument& doc)
{
    ports_.clear();

    TiXmlHandle docHandle(const_cast<TiXmlDocument*>(&doc));

    auto portElement = docHandle.FirstChild("FirmwarePackage").FirstChild("DevicePorts").FirstChild("DevicePort").ToElement();
    for ( ; portElement; portElement = portElement->NextSiblingElement("DevicePort"))
    {
        auto portName = portElement->Attribute("Name");
        auto portType = portElement->Attribute("Type");
        auto portConfigElem = portElement->FirstChildElement("PortConfiguration");

        if (!portName || !portType || !portConfigElem)
        {
            return Status::InvalidFile;
        }
        auto port = CreateDevicePort( portType );
        if (!port)
        {
            return Status::InvalidFile;
        }
        auto status = port->Configure( portName, *portConfigElem );
        if (failed(status))
        {
            return status;
        }
        ports_.push_back( std::move(port) );
    }

    return Status::Success;
}


FirmwareUpdate::Status GigE3::Package::ReadDeviceTypes (const TiXmlDocument& doc)
{
    device_types_.clear();

    TiXmlHandle docHandle( const_cast<TiXmlDocument*>( &doc ) );

    auto deviceTypeElement = docHandle.FirstChild("FirmwarePackage").FirstChild("DeviceTypes").FirstChild("DeviceType").ToElement();
    for ( ; deviceTypeElement; deviceTypeElement = deviceTypeElement->NextSiblingElement("DeviceType"))
    {
        auto modelName = deviceTypeElement->Attribute("Name");
        if (!modelName)
        {
            return Status::InvalidFile;
        }
        std::vector<UploadGroup> groups;

        auto* uploadGroupElem = deviceTypeElement->FirstChildElement("UploadGroup");
        for ( ; uploadGroupElem; uploadGroupElem = uploadGroupElem->NextSiblingElement("UploadGroup"))
        {
            UploadGroup group;

            auto status = ReadUploadGroup( *uploadGroupElem, group );
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


static bool parseHexOrDecimal (const char* text, uint32_t& val)
{
    if (sscanf(text, "0x%x", &val))
    {
        return true;
    }
    if (sscanf(text, "%d", &val))
    {
        return true;
    }

    return false;
}


static bool parseAttribute (const TiXmlElement& elem, const char* attributeName, uint32_t& val)
{
    auto attrText = elem.Attribute(attributeName);
    if (!attrText)
    {
        return false;
    }
    return parseHexOrDecimal(attrText, val);
}


FirmwareUpdate::Status GigE3::Package::ReadUploadGroup (const TiXmlElement& uploadGroupElem,
                                                        UploadGroup& group)
{
    auto groupName = uploadGroupElem.Attribute("Name");
    auto destinationName = uploadGroupElem.Attribute("Destination");
    if (!groupName || !destinationName)
    {
        return Status::InvalidFile;
    }
    auto port = find_port( destinationName );
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

    auto* uploadItemElem = uploadGroupElem.FirstChildElement("Upload");
    for ( ; uploadItemElem != nullptr; uploadItemElem = uploadItemElem->NextSiblingElement("Upload"))
    {
        UploadItem item;

        auto status = ReadUploadItem(*uploadItemElem, item);
        if (failed( status))
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


FirmwareUpdate::Status GigE3::Package::ReadUploadItem (const TiXmlElement& uploadItemElem,
                                                       UploadItem& item)
{
    for (auto* attr = uploadItemElem.FirstAttribute(); attr; attr = attr->Next())
    {
        if (attr->Name() == std::string("File"))
        {
            if (item.Data != nullptr)
            {
                return Status::InvalidFile; // No two "data" elements allowed
            }
            item.Data = ExtractFile( attr->Value() );
            if (item.Data->empty())
            {
                int a = 0;
            }
        }
        else if (attr->Name() == std::string("String"))
        {
            if (item.Data != nullptr)
            {
                return Status::InvalidFile; // No two "data" elements allowed
            }
            auto str = attr->ValueStr();
            item.Data = std::make_shared<std::vector<uint8_t>>();
            item.Data->resize(str.length());
            memcpy(item.Data->data(), str.data(), str.length());
        }
        else if (attr->Name() == std::string("U32"))
        {
            if (item.Data != nullptr)
            {
                return Status::InvalidFile; // No two "data" elements allowed
            }
            uint32_t data = 0;
            if (!parseHexOrDecimal(attr->Value(), data))
            {
                return Status::InvalidFile;
            }
            item.Data = std::make_shared<std::vector<uint8_t>>();
            item.Data->resize( sizeof(data) );
            memcpy( item.Data->data(), &data, sizeof(data) );
        }
        else
        {
            uint32_t val = 0;
            if (!parseHexOrDecimal(attr->Value(), val))
            {
                return Status::InvalidFile;
            }
            item.Params[attr->Name()] = val;
        }
    }

    auto len = item.Params.find("Length");
    if (len != item.Params.end())
    {
        item.Data->resize( len->second, 0 );
    }

    return Status::Success;
}


std::shared_ptr<GigE3::IDevicePort> GigE3::Package::CreateDevicePort (const std::string& portType)
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

std::shared_ptr<std::vector<uint8_t>> GigE3::Package::ExtractFile( const std::string& fileName )
{
    auto& cached = file_data_cache_[fileName];
    if( cached == nullptr )
    {
        cached = std::make_shared<std::vector<uint8_t>>( FirmwarePackage::extractFile(packageFileName_, fileName) );
    }
    return cached;
}
