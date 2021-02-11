/*
 * Copyright 2017 The Imaging Source Europe GmbH
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

#include "DevicePortEEPROM.h"

#include "ReportProgress.h"

#include <algorithm>
#include <set>


namespace lib33u
{
namespace firmware_update
{
DevicePortEEPROM::DevicePortEEPROM(const std::string& name, const pugi::xml_node& /*port_config*/)
    : name_ { name }
{
}

std::string DevicePortEEPROM::name()
{
    return name_;
}

void DevicePortEEPROM::upload(Camera& dev,
                              const std::vector<UploadItem>& items,
                              util::progress::IReportProgress& progress)
{
    try
    {
        upload_internal(dev, items, progress);
    }
    catch (std::exception&)
    {
        // Some error occurred... Re-try once
        upload_internal(dev, items, progress);
    }
}

namespace
{
static std::vector<uint8_t> ensure_length_alignment(const std::vector<uint8_t>& data, int alignment)
{
    std::vector<uint8_t> copy = data;

    while (copy.size() % alignment) { copy.push_back(0); }

    return copy;
}
} // namespace

void DevicePortEEPROM::upload_internal(Camera& cam,
                                       const std::vector<UploadItem>& items,
                                       util::progress::IReportProgress& progress)
{
    auto items_progress = util::progress::MapItemProgress(progress, items.size());

    for (auto& upload : items)
    {
        auto padded_data = ensure_length_alignment(upload.data, 4);

        cam.eeprom().write_verify(
            upload.offset, padded_data.data(), padded_data.size(), items_progress);

        items_progress.report_item();
    }
}
} /* namespace firmware_update */
} /* namespace lib33u */
