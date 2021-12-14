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

#include "CaptureDevice.h"

#include "CaptureDeviceImpl.h"
#include "DeviceIndex.h"
#include "logging.h"
#include "utils.h"

using namespace tcam;


CaptureDevice::CaptureDevice() : impl(new CaptureDeviceImpl()) {}


CaptureDevice::CaptureDevice(const DeviceInfo& info) : impl(new CaptureDeviceImpl())
{
    impl->open_device(info);
}


CaptureDevice::~CaptureDevice() {}


bool CaptureDevice::is_device_open() const
{
    return impl->is_device_open();
}


DeviceInfo CaptureDevice::get_device() const
{
    return impl->get_device();
}


bool CaptureDevice::register_device_lost_callback(tcam_device_lost_callback callback,
                                                  void* user_data)
{
    return impl->register_device_lost_callback(callback, user_data);
}


std::vector<std::shared_ptr<tcam::property::IPropertyBase>> CaptureDevice::get_properties()
{
    return impl->get_properties();
}


std::shared_ptr<tcam::property::IPropertyBase> CaptureDevice::get_property(const std::string& name)
{
    return impl->get_property(name);
}


std::vector<VideoFormatDescription> CaptureDevice::get_available_video_formats() const
{
    return impl->get_available_video_formats();
}


bool CaptureDevice::set_video_format(const VideoFormat& new_format)
{
    return impl->set_video_format(new_format);
}


VideoFormat CaptureDevice::get_active_video_format() const
{
    return impl->get_active_video_format();
}


bool CaptureDevice::start_stream(std::shared_ptr<SinkInterface> sink)
{
    return impl->start_stream(sink);
}


bool CaptureDevice::stop_stream()
{
    return impl->stop_stream();
}


std::shared_ptr<CaptureDevice> tcam::open_device(const std::string& serial, TCAM_DEVICE_TYPE type)
{
    auto _open = [](const DeviceInfo& info) -> std::shared_ptr<CaptureDevice> {
        try
        {
            return std::make_shared<CaptureDevice>(info);
        }
        catch (const std::exception& err)
        {
            SPDLOG_ERROR("Could not open CaptureDevice. Exception:\"{}\"", err.what());
            return nullptr;
        }
    };


    DeviceIndex index;
    for (const auto& d : index.get_device_list())
    {
        if ((d.get_serial().compare(serial) == 0 || serial.empty())
            && (type == TCAM_DEVICE_TYPE_UNKNOWN || d.get_device_type() == type))
        {
            return _open(d);
        }
    }
    return nullptr;
}
