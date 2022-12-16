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

#include "CaptureDeviceImpl.h"

#include "logging.h"

#include <exception>

using namespace tcam;

namespace
{

bool prevent_software_properties (const DeviceInfo& dev)
{
    if (dev.get_device_type() == tcam::TCAM_DEVICE_TYPE_V4L2)
    {
        // DFG/HDMI
        if (std::string(dev.get_info().additional_identifier) == "9c97")
        {
            return true;
        }
    }
    else if (dev.get_device_type() == tcam::TCAM_DEVICE_TYPE_VIRTCAM)
    {
        return true;
    }

    return false;
}

} // namepsace


struct bad_device : std::exception
{
    const char* what() const noexcept final
    {
        return "Device did not comply with commands.";
    }
};

CaptureDeviceImpl::CaptureDeviceImpl(const DeviceInfo& device_desc)
{
    device_ = tcam::open_device_interface(device_desc);
    if (device_ == nullptr)
    {
        throw bad_device();
    }

    available_output_formats_ = device_->get_available_video_formats();
    if (available_output_formats_.empty())
    {
        SPDLOG_ERROR("Device '{}-{}-{}' has no video formats",
                     device_desc.get_name(),
                     device_desc.get_serial(),
                     device_desc.get_device_type_as_string());
        throw bad_device();
    }
    apply_software_properties_ = !prevent_software_properties(device_desc);

    if (apply_software_properties_)
    {
        property_filter_.setup(device_->get_properties(), available_output_formats_);
    }
    const auto serial = device_->get_device_description().get_serial();
    index_.register_device_lost(deviceindex_lost_cb, this, serial);
}

CaptureDeviceImpl::~CaptureDeviceImpl()
{
    stop_stream();

    available_output_formats_.clear();

    index_.remove_device_lost(deviceindex_lost_cb);

    device_.reset();
}

bool CaptureDeviceImpl::is_device_open() const
{
    return device_ != nullptr;
}

DeviceInfo CaptureDeviceImpl::get_device() const
{
    return device_->get_device_description();
}

bool CaptureDeviceImpl::register_device_lost_callback(tcam_device_lost_callback callback,
                                                      void* user_data)
{
    device_lost_callback_data_ = { callback, user_data };

    return device_->register_device_lost_callback(callback, user_data);
}

void CaptureDeviceImpl::deviceindex_lost_cb(const DeviceInfo& info, void* user_data)
{
    auto self = (CaptureDeviceImpl*)user_data;

    auto data = self->device_lost_callback_data_;
    if (data.callback)
    {
        const auto i = info.get_info();

        (*data.callback)(&i, data.user_data);
    }
}

std::vector<std::shared_ptr<tcam::property::IPropertyBase>> CaptureDeviceImpl::get_properties()
{
    if (apply_software_properties_)
    {
        return property_filter_.getProperties();
    }
    else
    {
        return device_->get_properties();
    }
}

std::vector<VideoFormatDescription> CaptureDeviceImpl::get_available_video_formats() const
{
    return available_output_formats_;
}

bool CaptureDeviceImpl::set_video_format(const VideoFormat& new_format)
{
    return device_->set_video_format(new_format);
}

VideoFormat CaptureDeviceImpl::get_active_video_format() const
{
    return device_->get_active_video_format();
}


bool CaptureDeviceImpl::configure_stream(const VideoFormat& format,
                                         std::shared_ptr<ImageSink>& sink,
                                         std::shared_ptr<BufferPool> pool)
{
    if (!device_->set_video_format(format))
    {
        return false;
    }

    if (apply_software_properties_)
    {
        property_filter_.setVideoFormat(device_->get_active_video_format());
    }
    // if no pool is provided allocate an internal
    // default to userptr as all devices support that
    if (!pool)
    {
        pool_ = std::make_shared<BufferPool>(TCAM_MEMORY_TYPE_USERPTR, device_->get_allocator());
        auto ret = pool_->allocate(device_->get_active_video_format(), 10);

        // TODO: error handling
        if (!ret)
        {
            SPDLOG_ERROR("{}", ret.error().message());
            return false;
        }
    }
    else
    {
        SPDLOG_INFO("External pool");
        pool_ = pool;
        auto ret = pool_->allocate();

        if (!ret)
        {
            SPDLOG_ERROR("Error while allocating buffer");
            return false;
        }
    }

    device_->initialize_buffers(pool_);

    sink_ = sink;

    return true;
}


bool CaptureDeviceImpl::free_stream()
{
    // TODO: check if stream is running
    //if ()
    device_->release_buffers();
    sink_.reset();

    return true;
}


bool CaptureDeviceImpl::start_stream()
{
    if (sink_ == nullptr)
    {
        SPDLOG_ERROR("No viable sink configured.");
        return false;
    }

    if (!sink_->start_stream(device_))
    {
        return false;
    }

    if (!device_->start_stream(shared_from_this()))
    {
        SPDLOG_ERROR("Unable to start stream from device.");

        stop_stream();

        return false;
    }
    return true;
}

void CaptureDeviceImpl::stop_stream()
{
    device_->stop_stream();
    //device_->release_buffers();
    //sink_.reset();
}

void CaptureDeviceImpl::set_drop_incomplete_frames(bool b)
{
    device_->set_drop_incomplete_frames(b);
}

void CaptureDeviceImpl::push_image(const std::shared_ptr<ImageBuffer>& buffer)
{
    if (apply_software_properties_)
    {
        property_filter_.apply(*buffer);
    }

    sink_->push_image(buffer);
}

outcome::result<tcam::framerate_info> CaptureDeviceImpl::get_framerate_info(const VideoFormat& fmt)
{
    return device_->get_framerate_info(fmt);
}

std::shared_ptr<tcam::AllocatorInterface> CaptureDeviceImpl::get_allocator()
{
    return device_->get_allocator();
}
