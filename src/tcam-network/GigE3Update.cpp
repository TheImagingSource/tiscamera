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

#include <iostream>

#include "GigE3Update.h"

#include "GigE3Package.h"
#include "GigE3Progress.h"

#include "FirmwarePackage.h"

namespace FirmwareUpdate
{

bool groupUploadRequired (IFirmwareWriter& dev, const GigE3::UploadGroup& group)
{
    uint32_t version = 0;
    dev.read( group.VersionCheckRegister, version );

    return version != group.Version;
}


Status initiateColdReboot (IFirmwareWriter& dev)
{
    if (dev.write(0xEF000004, 0xC07DB007))
    {
        return Status::Success;
    }
    else
    {
        // Camera does not support cold boot, request a user reconnect
        return Status::SuccessDisconnectRequired;
    }
}


Status GigE3::upgradeFirmware (IFirmwareWriter& dev,
                               const std::string& fileName,
                               const std::string& modelName,
                               const std::string& originalModelName,
                               tReportProgressFunc progressFunc)
{
    Package package;
    auto status = package.Load(fileName);
    if (failed(status))
    {
        return status;
    }

    auto modelUploadGroups = package.find_upload_groups(modelName);
    if (!modelUploadGroups)
    {
        return Status::NoMatchFoundInPackage;
    }

    std::vector<UploadGroup> necessaryGroups;
    for (auto&& group : *modelUploadGroups)
    {
        //if (groupUploadRequired(dev, group) || (modelName != originalModelName))
        {
            necessaryGroups.push_back(group);
        }
    }
    if (necessaryGroups.empty())
    {
        return Status::SuccessNoActionRequired;
    }
    mapItemProgress groupItemProgress (progressFunc, 0, 100,
                                       (int) necessaryGroups.size(),
                                       std::string());
    for (auto&& group : necessaryGroups)
    {
        groupItemProgress( 0, "Updating " + group.Name );

        status = group.DestionationPort->UploadItems(dev, group.Items, groupItemProgress);
        if (failed(status))
            return status;

        groupItemProgress.NextItem();
    }

    return initiateColdReboot( dev );
}


std::vector<std::string> GigE3::getModelNamesFromPackage (const std::string& fileName)
{
    return GigE3::Package::FindModelNames(fileName);
}

} /* namespace FirmwareUpdate */
