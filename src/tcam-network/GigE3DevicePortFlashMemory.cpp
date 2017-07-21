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

#include "GigE3DevicePortFlashMemory.h"

#include "GigE3Progress.h"

#include <tinyxml.h>
#include <cstdio> // sscanf

using namespace FirmwareUpdate;


bool parseAttribute (const TiXmlElement& elem,
                     const char* attributeName,
                     uint32_t& val)
{
    auto attrText = elem.Attribute (attributeName);
    if (!attrText)
    {
        return false;
    }

    if (sscanf(attrText, "0x%x", &val))
    {
        return true;
    }
    if (sscanf(attrText, "%d", &val))
    {
        return true;
    }

    return false;
}


FirmwareUpdate::Status GigE3::DevicePortFlashMemory::Configure (const std::string& name,
                                                                const TiXmlElement& portConfigElem)
{
    if (!parseAttribute(portConfigElem, "EraseAddress", eraseAddress_)) return Status::InvalidFile;
    if (!parseAttribute(portConfigElem, "UnlockCode", unlockCode_)) return Status::InvalidFile;
    if (!parseAttribute(portConfigElem, "UnlockAddress", unlockAddress_)) return Status::InvalidFile;
    if (!parseAttribute(portConfigElem, "BlockSize", blockSize_)) return Status::InvalidFile;
    if (!parseAttribute(portConfigElem, "Length", length_)) return Status::InvalidFile;
    if (!parseAttribute(portConfigElem, "BaseAddress", baseAddress_)) return Status::InvalidFile;

    name_ = name;
    return Status::Success;
}


FirmwareUpdate::Status GigE3::DevicePortFlashMemory::CheckItems (const std::vector<UploadItem>& items)
{
    for (auto&& i : items)
    {
        // The "Offset" param is required
        if (i.Params.find("Offset") == i.Params.end())
        {
            return Status::InvalidFile;
        }
    }
    return Status::Success;
}


std::vector<uint8_t> PadData (const std::vector<uint8_t>& data, int alignment)
{
    std::vector<uint8_t> result(data.begin(), data.end());

    while (result.size() % alignment)
        result.push_back(0);

    return result;
}


FirmwareUpdate::Status GigE3::DevicePortFlashMemory::UploadItems (IFirmwareWriter& dev,
                                                                  const std::vector<UploadItem>& items,
                                                                  tReportProgressFunc progressFunc )
{
    std::set<uint32_t> eraseRequests;
    for (auto&& i : items)
    {
        AddEraseRequests(i, eraseRequests);
    }

    if (!dev.write(unlockAddress_, unlockCode_))
    {
        return Status::WriteError;
    }

    mapItemProgress eraseProgress { progressFunc, 0, 30, (int)eraseRequests.size(), "" };

    for (auto&& erq : eraseRequests)
    {
        bool ret = dev.write(eraseAddress_, erq);

        eraseProgress.NextItem();
    }

    mapItemProgress uploadItemsProgress { progressFunc, 30, 100, (int)items.size(), "" };

    for (auto&& i : items)
    {
        auto data = PadData(*i.Data, 4);
        auto offset = i.Params.at("Offset");

        auto writeProgress = mapProgress(uploadItemsProgress, 0, 50);

        auto status = WriteDeviceMemory(dev, baseAddress_ + offset, data, writeProgress);
        if (failed(status))
        {
            return status;
        }

        auto readProgress = mapProgress(uploadItemsProgress, 50, 100);

        std::vector<uint8_t> verifyBuffer(data.size());
        status = ReadDeviceMemory(dev, baseAddress_ + offset, verifyBuffer, readProgress);
        if (failed(status))
        {
            return status;
        }

        if (data != verifyBuffer)
        {
            return Status::WriteVerificationError;
        }

        uploadItemsProgress.NextItem();
    }

    dev.write(unlockAddress_, 0);

    return Status::Success;
}


void GigE3::DevicePortFlashMemory::AddEraseRequests (const GigE3::UploadItem& item,
                                                     std::set<uint32_t>& requests)
{
    uint32_t startBlockOffset = item.Params.at("Offset") % blockSize_;
    requests.insert(item.Params.at("Offset") - startBlockOffset);

    uint32_t spaceToEndOfBlock = blockSize_ - startBlockOffset;
    if (item.Data->size() > spaceToEndOfBlock)
    {
        uint32_t remainingLength = (uint32_t) item.Data->size() - spaceToEndOfBlock;
        uint32_t offset = item.Params.at("Offset") + spaceToEndOfBlock;

        while (remainingLength > 0)
        {
            requests.insert(offset);

            uint32_t step = std::min( blockSize_, remainingLength );
            remainingLength -= step;
            offset += step;
        }
    }
}


FirmwareUpdate::Status GigE3::DevicePortFlashMemory::WriteDeviceMemory (IFirmwareWriter& dev,
                                                                        uint32_t address,
                                                                        const std::vector<uint8_t>& data,
                                                                        tReportProgressFunc progressFunc)
{
    //const size_t StepSize = 1024 * 16;
    const size_t StepSize = 512;
    size_t totalBytes = data.size();
    size_t bytesRemaining = totalBytes;
    size_t bytesWritten = 0;

    while (bytesRemaining > 0)
    {
        size_t stepBytes = std::min( bytesRemaining, StepSize );

        if (!dev.write((uint32_t)(address + bytesWritten),
                       (uint32_t*)(data.data() + bytesWritten),
                       (size_t) stepBytes, 0))
        {
            return Status::WriteError;
        }

        bytesWritten += stepBytes;
        bytesRemaining -= stepBytes;

        progressFunc((int)(bytesWritten * 100 / totalBytes), std::string());
    }

    return Status::Success;
}


FirmwareUpdate::Status GigE3::DevicePortFlashMemory::ReadDeviceMemory (IFirmwareWriter& dev,
                                                                       uint32_t address,
                                                                       std::vector<uint8_t>& buffer,
                                                                       tReportProgressFunc progressFunc)
{
    //const size_t StepSize = 1024 * 16;
    const size_t StepSize = 512;
    size_t totalBytes = buffer.size();
    size_t bytesRemaining = totalBytes;
    size_t bytesRead = 0;

    while (bytesRemaining > 0)
    {
        size_t stepBytes = std::min( bytesRemaining, StepSize );

        unsigned int stepBytesRead = 0;
        if (!dev.read((uint32_t)(address + bytesRead),
                      (uint32_t)stepBytes,
                      buffer.data() + bytesRead,
                      stepBytesRead))
        {
            return Status::WriteError;
        }

        if (stepBytesRead != stepBytes)
        {
            return Status::WriteError;
        }

        bytesRead += stepBytes;
        bytesRemaining -= stepBytes;

        progressFunc((int)(bytesRead * 100 / totalBytes), std::string());
    }

    return Status::Success;
}
