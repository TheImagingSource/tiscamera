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

#include "GigE3DevicePort.h"

#include <set>

namespace FirmwareUpdate
{

namespace GigE3
{

class DevicePortFlashMemory : public IDevicePort
{
    std::string name_;

    uint32_t eraseAddress_;
    uint32_t unlockCode_;
    uint32_t unlockAddress_;
    uint32_t blockSize_;
    uint32_t length_;
    uint32_t baseAddress_;

public:
    virtual std::string name () override { return name_; }

    virtual Status Configure (const std::string& name,
                              const TiXmlElement& portConfigElem) override;
    virtual Status CheckItems (const std::vector<UploadItem>& items) override;
    virtual Status UploadItems (IFirmwareWriter& dev,
                                const std::vector<UploadItem>& items,
                                tReportProgressFunc progressFunc) override;

private:
    void AddEraseRequests (const UploadItem& item, std::set<uint32_t>& requests);

    Status WriteDeviceMemory (IFirmwareWriter& dev,
                              uint32_t address,
                              const std::vector<uint8_t>& data,
                              tReportProgressFunc progressFunc);

    Status ReadDeviceMemory (IFirmwareWriter& dev,
                             uint32_t address,
                             std::vector<uint8_t>& buffer,
                             tReportProgressFunc progressFunc);

}; /* class DevicePortFlashMemory */

} /* namespace GigE3 */

} /* namespace FirmwareUpdate */
