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

#include "DeviceInterface.h"
#include "logging.h"

#if HAVE_USB
#include "V4l2Device.h"
#endif

#if HAVE_ARAVIS

#include "AravisDevice.h"

#endif

#include <algorithm>
#include <memory>

using namespace tcam;


std::shared_ptr<DeviceInterface> tcam::openDeviceInterface (const DeviceInfo& device)
{

    try
    {
        TCAM_DEVICE_TYPE type = device.get_device_type();

        switch (type)
        {
            case TCAM_DEVICE_TYPE_V4L2:
            {
#if HAVE_USB
                return std::make_shared<V4l2Device>(device);
#else
                break;
#endif
            }
            case TCAM_DEVICE_TYPE_ARAVIS:
            {
#if HAVE_ARAVIS
                return std::make_shared<AravisDevice>(device);
#else
                break;
#endif
            }
            case TCAM_DEVICE_TYPE_UNKNOWN:
            default:
            {
                throw std::runtime_error("Unsupported device");
            }
        }

    }
    catch (...)
    {
        tcam_log(TCAM_LOG_ERROR, "Encountered Error while creating device interface.");
        return nullptr;
    }
    return nullptr;
}
