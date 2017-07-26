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

#include "GigE3DevicePortMachXO2.h"

#include "GigE3Progress.h"

#include "MachXO2.h"
#include "JedecFile.h"

#include <exception>

using namespace FirmwareUpdate;

FirmwareUpdate::Status GigE3::DevicePortMachXO2::Configure (const std::string& name,
                                                            const TiXmlElement& portConfigElem)
{
    name_ = name;

    return Status::Success;
}


FirmwareUpdate::Status GigE3::DevicePortMachXO2::CheckItems (const std::vector<UploadItem>& items)
{
    // We cannot upload 2 items into MachXO2
    if (items.size() > 1)
        return Status::InvalidFile;

    for (auto&& i : items)
    {
        auto jedec = MachXO2::JedecFile::Parse( *i.Data );

        // If the data is not valid jedec, the device type should not be detectable
        if (jedec.deviceType() == MachXO2::DeviceType::MachXO2_Unknown)
        {
            return Status::InvalidFile;
        }
    }

    return Status::Success;
}

namespace
{

std::function<void(int)> forwardSimpleProgress (std::function<void(int, const std::string&)> progressFunc)
{
    return [=](int progress) { progressFunc(progress, std::string()); };
}


std::function<void(const char*, int)> forwardAdvancedProgress(std::function<void(int,
                                                                                 const std::string& )> progressFunc)
{
    return [=](const char* msg, int progress) { progressFunc(progress, msg); };
}


I2C::DataArray s_i2cWriteData;


size_t AlignBufferSize (size_t size, int alignment)
{
    return ((size + (alignment - 1)) / alignment) * alignment;
}


I2C::DataArray AlignExpandBuffer (const I2C::DataArray& buffer, int alignment)
{
    auto result = buffer;

    size_t newSize = AlignBufferSize(buffer.size(), alignment);
    result.resize(newSize);

    return result;
}


I2C::DataArray I2CTransaction (IFirmwareWriter& itf,
                               uint8_t i2cDev,
                               const I2C::DataArray& writeData,
                               uint16_t readRequestLength)
{
    uint32_t maxWriteLength = 0;
    uint32_t maxReadLength = 0;
    if (!itf.read(0xE0000004, maxWriteLength) || !itf.read(0xE0000008, maxReadLength))
    {
        throw std::runtime_error("The device does not support I2C");
    }

    if (writeData.size() > maxWriteLength)
    {
        throw std::runtime_error( "writeData.size() > maxWriteLength" );
    }

    if (readRequestLength > maxReadLength)
    {
        throw std::runtime_error( "readRequestLength > maxReadLength" );
    }

    auto writeBuffer = AlignExpandBuffer(writeData, 4);
    if (!itf.write( 0xE0001000, writeBuffer.data(), writeBuffer.size()))
    {
        throw std::runtime_error( "The device did not accept the write buffer" );
    }

    uint32_t cmd = ((uint32_t)readRequestLength << 16) | ((uint32_t)writeData.size() << 8) | i2cDev;

    if(!itf.write(0xE0000000, cmd))
    {
        throw std::runtime_error( "The device returned an error when trying to issue an I2C command" );
    }

    size_t readBufferSize = AlignBufferSize(readRequestLength, 4);
    I2C::DataArray readBuffer(readBufferSize);

    if (readBufferSize)
    {
        unsigned int readCount = 0;
        if(!itf.read( 0xE0002000, (unsigned)readBuffer.size(), readBuffer.data(), readCount))
        {
            throw std::runtime_error("Failed to read the I2C read buffer form the device");
        }
    }

    readBuffer.resize(readRequestLength);
    return readBuffer;
}


I2C::WriteAction forwardI2CWrite (IFirmwareWriter& itf)
{
    return [&](uint8_t i2cDev, const I2C::DataArray& command, bool combineWithRead)
    {
        if (combineWithRead)
        {
            s_i2cWriteData = command;
        }
        else
        {
            I2CTransaction(itf, i2cDev, command, 0);
        }
    };
}


I2C::ReadAction forwardI2CRead (IFirmwareWriter& itf)
{
    return [&](uint8_t i2cDev, uint16_t requestLength, bool combineWithWrite) -> I2C::DataArray
    {
        if (!combineWithWrite)
        {
            throw std::runtime_error("combineWithWrite has to be <true> for GigE cameras");
        }

        if (s_i2cWriteData.empty())
        {
            throw std::runtime_error("I2CWrite has to be called with combineWithRead == <true> before calling I2CRead for GigE cameras");
        }

        auto result = I2CTransaction(itf, i2cDev, s_i2cWriteData, requestLength);

        s_i2cWriteData.clear();

        return result;
    };
}


size_t queryMaxI2cReadLength (IFirmwareWriter& itf)
{
    uint32_t maxReadLength = 0;
    if (!itf.read(0xE0000008, maxReadLength))
        throw std::runtime_error("The device does not support I2C");

    return maxReadLength;
}

} // namespace


FirmwareUpdate::Status GigE3::DevicePortMachXO2::UploadItems (IFirmwareWriter& dev,
                                                              const std::vector<UploadItem>& items,
                                                              tReportProgressFunc progressFunc)
{
    auto&& item = items.front();

    try
    {
        auto jedec = MachXO2::JedecFile::Parse(*item.Data);

        I2C::I2CDevice i2c(0x80, forwardI2CWrite(dev), forwardI2CRead(dev), queryMaxI2cReadLength(dev));
        MachXO2::MachXO2Device dev(i2c);

        if (dev.UpdateConfiguration(jedec, forwardAdvancedProgress(progressFunc)))
        {
            // UpdateConfiguration returns false if no upgrade was necessary
            return Status::Success;
        }
        else
        {
            return FirmwareUpdate::Status::SuccessNoActionRequired;
        }
    }
    catch(const std::exception&)
    {
        return FirmwareUpdate::Status::MachXO2UpdateFailed;
    }
}
