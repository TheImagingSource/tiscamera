/*
 * Copyright 2021 The Imaging Source Europe GmbH
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

#include "mainsrc_device_state.h"

#include "mainsrc_tcamprop_impl.h"
#include "tcambind.h"

#include <tcamprop1.0_gobject/tcam_property_serialize.h>

#define GST_CAT_DEFAULT tcam_mainsrc_debug

bool device_state::set_device_serial(const std::string& str) noexcept
{
    std::lock_guard lck { device_open_mutex_ };
    if (is_device_open())
    {
        return false;
    }
    device_serial_to_open_ = str;
    return true;
}

bool device_state::set_device_type(tcam::TCAM_DEVICE_TYPE type) noexcept
{
    std::lock_guard lck { device_open_mutex_ };
    if (is_device_open())
    {
        return false;
    }
    device_type_to_open_ = type;
    return true;
}

void device_state::set_tcam_properties(const GstStructure* ptr) noexcept
{
    std::lock_guard lck { device_open_mutex_ };

    if (!is_device_open())
    {
        if (ptr)
        {
            prop_init_ = gst_helper::make_ptr(gst_structure_copy(ptr));
        }
        else
        {
            prop_init_ = {};
        }
    }
    else
    {
        if (ptr != nullptr)
        {
            apply_properties(*ptr);
        }
    }
}

gst_helper::gst_ptr<GstStructure> device_state::get_tcam_properties() noexcept
{
    if (!is_device_open())
    {
        if (!prop_init_.empty())
        {
            return gst_helper::make_ptr(gst_structure_copy(prop_init_.get()));
        }
        return gst_helper::make_ptr(gst_structure_new_empty("tcam"));
    }

    auto ptr = gst_helper::make_ptr(gst_structure_new_empty("tcam"));

    tcamprop1_gobj::serialize_properties(TCAM_PROPERTY_PROVIDER(parent_), *ptr);

    return ptr;
}

std::string device_state::get_device_serial() const noexcept
{
    std::lock_guard lck { device_open_mutex_ };
    if (is_device_open())
    {
        return device_->get_device().get_serial();
    }
    return device_serial_to_open_;
}

tcam::TCAM_DEVICE_TYPE device_state::get_device_type() const noexcept
{
    std::lock_guard lck { device_open_mutex_ };
    if (is_device_open())
    {
        return device_->get_device().get_device_type();
    }
    return device_type_to_open_;
}

GstCaps* device_state::get_device_caps() const
{
    std::lock_guard lck { device_open_mutex_ };
    if (all_caps_ == nullptr)
    {
        return nullptr;
    }
    return gst_caps_copy(all_caps_.get());
}

void device_state::stop_and_clear()
{
    if (device_)
    {
        device_->stop_stream();
    }
    while (!queue.empty())
    {
        auto ptr = queue.front();
        queue.pop();
        if (sink)
        {
            sink->requeue_buffer(ptr);
        }
    }
}

void device_state::close()
{
    std::lock_guard<std::mutex> lck(stream_mtx_);

    // clear list to ensure property users get a no device error
    tcamprop_container_.clear_list();
    tcamprop_interface_.clear();
    if (device_)
    {
        stop_and_clear();

        device_ = nullptr;
        sink = nullptr;
        all_caps_.reset();
    }
}

void device_state::populate_tcamprop_interface()
{
    auto properties = device_->get_properties();

    tcamprop_interface_.tcamprop_properties.reserve(properties.size());

    for (auto& p : properties)
    {
        auto prop = tcam::mainsrc::make_wrapper_instance(p);
        if (prop)
        {
            tcamprop_interface_.tcamprop_properties.push_back(std::move(prop));
        }
    }

    tcamprop_container_.create_list(&tcamprop_interface_);
}


void device_state::apply_properties(const GstStructure& strct)
{
    tcamprop1_gobj::apply_properties(
        TCAM_PROPERTY_PROVIDER(parent_),
        strct,
        [this](const GError& err, const std::string& prop_name, const GValue*)
        {
            GST_WARNING_OBJECT(parent_,
                               "Failed to init property named '%s' due to: '%s'",
                               prop_name.c_str(),
                               err.message);
        });
}

bool device_state::open_camera()
{
    std::lock_guard lck { device_open_mutex_ };

    GST_DEBUG_OBJECT(parent_,
                     "Trying to open device with serial='%s and type='%s'.",
                     device_serial_to_open_.c_str(),
                     tcam::tcam_device_type_to_string(device_type_to_open_).c_str());

    auto dev = tcam::open_device(device_serial_to_open_, device_type_to_open_);
    if (!dev)
    {
        GST_ELEMENT_ERROR(parent_, RESOURCE, NOT_FOUND, ("Failed to open device."), (NULL));
        close();
        return false;
    }

    auto caps =
        tcambind::convert_videoformatsdescription_to_caps(dev->get_available_video_formats());
    if (caps == nullptr || gst_caps_get_size(caps.get()) == 0)
    {
        GST_ELEMENT_ERROR(parent_, CORE, CAPS, ("Failed to create caps for device."), (NULL));
        close();
        return false;
    }

    device_ = dev;
    all_caps_ = caps;

    GST_DEBUG_OBJECT(
        parent_, "Device provides the following caps: %s", gst_helper::to_string(*caps).c_str());

    // reset the init variables
    device_serial_to_open_ = {};
    device_type_to_open_ = tcam::TCAM_DEVICE_TYPE_UNKNOWN;

    populate_tcamprop_interface();

    if (prop_init_)
    {
        apply_properties(*prop_init_);
        prop_init_.reset();
    }

    return true;
}
