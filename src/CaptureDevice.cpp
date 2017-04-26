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
    impl->open_device(info);
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
    return impl->is_device_open ();
}


DeviceInfo CaptureDevice::get_device () const
{
    return impl->get_device();
}


bool CaptureDevice::register_device_lost_callback (tcam_device_lost_callback callback, void* user_data)
{
    return impl->register_device_lost_callback(callback, user_data);
}


std::vector<Property*> CaptureDevice::get_available_properties ()
{
    return impl->get_available_properties();
}


Property* CaptureDevice::get_property (TCAM_PROPERTY_ID id)
{
    auto properties = get_available_properties();

    for (auto& p :  properties)
    {
        if (p->get_ID() == id)
        {
            return p;
        }
    }
    return nullptr;
}


Property* CaptureDevice::get_property_by_name (const std::string& name)
{
    if (name.empty())
    {
        return nullptr;
    }

    auto properties = get_available_properties();

    for (auto& p :  properties)
    {
        if (p->get_name().compare(name) == 0)
        {
            return p;
        }
    }
    return nullptr;
}


bool CaptureDevice::set_property (TCAM_PROPERTY_ID id, const int64_t& value)
{
    auto vec = get_available_properties();

    for (const auto& v : vec)
    {
        if (id == v->get_ID())
        {
            if (v->get_type() == TCAM_PROPERTY_TYPE_INTEGER)
            {
                return v->set_value(value);
            }
        }
    }

    return false;
}


bool CaptureDevice::set_property (TCAM_PROPERTY_ID id, const double& value)
{
    auto vec = get_available_properties();

    for (const auto& v : vec)
    {
        if (id == v->get_ID())
        {
            if (v->get_type() == TCAM_PROPERTY_TYPE_DOUBLE)
            {
                return v->set_value(value);
            }
        }
    }

    return false;
}


bool CaptureDevice::set_property (TCAM_PROPERTY_ID id, const bool& value)
{
    auto vec = get_available_properties();

    for (const auto& v : vec)
    {
        if (id == v->get_ID())
        {
            if (v->get_type() == TCAM_PROPERTY_TYPE_BOOLEAN)
            {
                return v->set_value(value);
            }
        }
    }

    return false;
}


bool CaptureDevice::set_property (TCAM_PROPERTY_ID id, const std::string& value)
{
    auto vec = get_available_properties();

    for (const auto& v : vec)
    {
        if (id == v->get_ID())
        {
            if (v->get_type() == TCAM_PROPERTY_TYPE_STRING)
            {
                return v->set_value(value);
            }
        }
    }

    return false;
}


std::vector<VideoFormatDescription> CaptureDevice::get_available_video_formats () const
{
    return impl->get_available_video_formats();
}


bool CaptureDevice::set_video_format (const VideoFormat& new_format)
{
    return impl->set_video_format(new_format);
}


VideoFormat CaptureDevice::get_active_video_format () const
{
    return impl->get_active_video_format();
}


bool CaptureDevice::start_stream (std::shared_ptr<SinkInterface> sink)
{
    return impl->start_stream(sink);
}


bool CaptureDevice::stop_stream ()
{
    return impl->stop_stream();
}


std::shared_ptr<CaptureDevice> tcam::open_device (const std::string& serial)
{
    for (const auto& d : get_device_list())
    {
        if (d.get_serial().compare(serial) == 0)
        {
            try
            {
                return std::make_shared<CaptureDevice>(d);
            }
            catch (const std::exception& err)
            {
                // TODO: set up error
                tcam_log(TCAM_LOG_ERROR, "Could not open CaptureDevice. Exception:\"%s\"", err.what());
                return nullptr;
            }
        }
    }

    return nullptr;
}
