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

#include "base_types.h"
#include "logging.h"

#include <algorithm>
#include <memory>


#ifdef HAVE_ARAVIS
#include "aravis/aravis_api.h"
#endif

#ifdef HAVE_V4L2
#include "v4l2/v4l2_api.h"
#endif

#ifdef HAVE_LIBUSB
#include "libusb/libusb_api.h"
#endif

#ifdef HAVE_VIRTCAM
#include "virtcam/virtcam_api.h"
#endif


using namespace tcam;


std::vector<DeviceInfo> tcam::get_device_list()
{
    std::vector<DeviceInfo> ret;

#ifdef HAVE_ARAVIS

    auto arv_list = AravisBackend::get_instance()->get_device_list();
    ret.insert(ret.end(), arv_list.begin(), arv_list.end());

#endif

#ifdef HAVE_V4L2

    auto v4l2_list = V4L2Backend::get_instance()->get_device_list();
    ret.insert(ret.end(), v4l2_list.begin(), v4l2_list.end());

#endif

#ifdef HAVE_LIBUSB

    auto usb_list = LibUsbBackend::get_instance()->get_device_list();
    ret.insert(ret.end(), usb_list.begin(), usb_list.end());

#endif

#ifdef HAVE_VIRTCAM

    auto virtcam_list = virtcam::VirtBackend::get_instance()->get_device_list();
    ret.insert(ret.end(), virtcam_list.begin(), virtcam_list.end());

#endif

    return ret;
}


std::shared_ptr<DeviceInterface> tcam::open_device_interface(const DeviceInfo& device)
{
    try
    {
        switch (device.get_device_type())
        {
            case TCAM_DEVICE_TYPE_ARAVIS:
            {
#ifdef HAVE_ARAVIS
                return AravisBackend::get_instance()->open_device(device);
#else
                SPDLOG_ERROR("Aravis has not been enabled as a backend. Compile tiscamera with "
                             "aravis enabled.");
#endif
            }
            case TCAM_DEVICE_TYPE_V4L2:
            {

#ifdef HAVE_V4L2
                return V4L2Backend::get_instance()->open_device(device);
#else
                SPDLOG_ERROR("V4L2 has not been enabled as a backend. Compile tiscamera with "
                             "v4l2 enabled.");
#endif
            }
            case TCAM_DEVICE_TYPE_LIBUSB:
            {

#ifdef HAVE_LIBUSB
                return LibUsbBackend::get_instance()->open_device(device);
#else
                SPDLOG_ERROR("LibUsb has not been enabled as a backend. Compile tiscamera with "
                             "libusb enabled.");
#endif
            }
            case TCAM_DEVICE_TYPE_VIRTCAM:
            {

#ifdef HAVE_VIRTCAM
                return virtcam::VirtBackend::get_instance()->open_device(device);
#else
                SPDLOG_ERROR("Virtcam has not been enabled as a backend. Compile tiscamera with "
                             "virtcam enabled.");
#endif
            }
            default:
            {
                SPDLOG_ERROR("Device type not handled.");
                break;
            }
        }
    }
    catch (const std::runtime_error& err)
    {
        SPDLOG_ERROR("Encountered Error while creating device interface. {}", err.what());
        return nullptr;
    }
    catch (...)
    {
        SPDLOG_ERROR("Caught unhandled exception while opening device.");
    }
    return nullptr;
}

outcome::result<tcam::framerate_info> DeviceInterface::get_framerate_info(const VideoFormat& fmt)
{
    for (auto&& desc : get_available_video_formats())
    {
        if (desc.get_fourcc() == fmt.get_fourcc())
        {
            auto lst = desc.get_framerates(fmt);
            if (!lst.empty())
            {
                return tcam::framerate_info { lst };
            }
        }
    }
    return tcam::status::FormatInvalid;
}
