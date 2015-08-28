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

#include "Error.h"

#include "logging.h"
#include "utils.h"
#include "serialization.h"

#include "CaptureDeviceImpl.h"

using namespace tcam;


CaptureDevice::CaptureDevice ()
    : impl(new CaptureDeviceImpl())
{}


CaptureDevice::CaptureDevice (const DeviceInfo& info)
    : impl(new CaptureDeviceImpl())
{
    impl->openDevice(info);
}


CaptureDevice::~CaptureDevice ()
{}


bool CaptureDevice::load_configuration (const std::string& filename)
{
    return impl->load_configuration(filename);
}


bool CaptureDevice::save_configuration (const std::string& filename)
{
    return impl->save_configuration(filename);
}


bool CaptureDevice::is_device_open () const
{
    return impl->isDeviceOpen ();
}


DeviceInfo CaptureDevice::get_device () const
{
    return impl->getDevice();
}


std::vector<Property*> CaptureDevice::get_available_properties ()
{
    return impl->getAvailableProperties();
}


std::vector<VideoFormatDescription> CaptureDevice::get_available_video_formats () const
{
    return impl->getAvailableVideoFormats();
}


bool CaptureDevice::set_video_format (const VideoFormat& new_format)
{
    return impl->setVideoFormat(new_format);
}


VideoFormat CaptureDevice::get_active_video_format () const
{
    return impl->getActiveVideoFormat();
}


bool CaptureDevice::start_stream (std::shared_ptr<SinkInterface> sink)
{
    return impl->startStream(sink);
}


bool CaptureDevice::stop_stream ()
{
    return impl->stopStream();
}
