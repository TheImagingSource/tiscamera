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

#include "../../logging.h"
#include "mainsrc_tcamprop_impl.h"
#include "tcambind.h"
#include "../tcamgstbase/tcamgststrings.h"
#include "../tcamgstbase/tcamgstbase.h"

#include <tcamprop1.0_gobject/tcam_property_serialize.h>

#define GST_CAT_DEFAULT tcam_mainsrc_debug


tcam::TCAM_MEMORY_TYPE tcam::mainsrc::io_mode_to_memory_type(GstTcamIOMode mode)
{
    switch (mode)
    {
        case GST_TCAM_IO_AUTO:
        case GST_TCAM_IO_USERPTR:
        {
            return tcam::TCAM_MEMORY_TYPE_USERPTR;
        }
        case GST_TCAM_IO_MMAP:
        {
            return tcam::TCAM_MEMORY_TYPE_MMAP;
        }
        // case GST_TCAM_IO_DMABUF:
        //     return tcam::TCAM_MEMORY_TYPE_DMA;
        // case GST_TCAM_IO_DMABUF_IMPORT:
        //     return tcam::TCAM_MEMORY_TYPE_DMA_IMPORT;
    }
    return tcam::TCAM_MEMORY_TYPE_USERPTR;
}

GstTcamIOMode tcam::mainsrc::memory_type_to_io_mode(tcam::TCAM_MEMORY_TYPE t)
{
    switch (t)
    {
        case tcam::TCAM_MEMORY_TYPE_USERPTR:
            return GST_TCAM_IO_USERPTR;
        case tcam::TCAM_MEMORY_TYPE_MMAP:
            return GST_TCAM_IO_MMAP;
        case tcam::TCAM_MEMORY_TYPE_DMA:
        //     return GST_TCAM_IO_DMABUF;
        case tcam::TCAM_MEMORY_TYPE_DMA_IMPORT:
            break;
        //     return GST_TCAM_IO_DMABUF_IMPORT;
    }
    return GST_TCAM_IO_USERPTR;
}


gboolean tcam::mainsrc::caps_to_format(GstCaps& c, tcam::tcam_video_format& format)
{

    GstStructure* structure = gst_caps_get_structure(&c, 0);

    int height = 0;
    int width = 0;
    gst_structure_get_int(structure, "width", &width);
    gst_structure_get_int(structure, "height", &height);
    const GValue* frame_rate = gst_structure_get_value(structure, "framerate");
    const char* format_string = gst_structure_get_string(structure, "format");

    uint32_t fourcc;
    if (format_string)
    {
        fourcc = tcam::gst::tcam_fourcc_from_gst_1_0_caps_string(gst_structure_get_name(structure),
                                                                 format_string);
    }
    else
    {
        fourcc =
            tcam::gst::tcam_fourcc_from_gst_1_0_caps_string(gst_structure_get_name(structure), "");
    }

    double framerate;
    if (frame_rate != nullptr)
    {
        auto fps_numerator = gst_value_get_fraction_numerator(frame_rate);
        auto fps_denominator = gst_value_get_fraction_denominator(frame_rate);
        gst_util_fraction_to_double(fps_numerator, fps_denominator, &framerate);
    }
    else
    {
        framerate = 1.0;
    }

    format.fourcc = fourcc;
    format.width = width;
    format.height = height;
    format.framerate = framerate;

    format.scaling = tcam::gst::caps_get_scaling(&c);

    return true;
}


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


bool device_state::configure_stream()
{
    auto conf_res = device_->configure_stream(format_, sink, buffer_pool);

    if (!conf_res)
    {
        GST_ELEMENT_ERROR(parent_, CORE, CAPS, ("Failed to configure stream."), (NULL));
        return FALSE;
    }
    return TRUE;
}


void device_state::start_stream()
{
    if (device_)
    {
        device_->start_stream();
    }
}


void device_state::stop_stream()
{
    if (device_ && is_streaming_)
    {
        device_->stop_stream();
    }
    is_streaming_ = false;
}


void device_state::stop_and_clear()
{
    if (device_ && is_streaming_)
    {
        device_->stop_stream();
    }
    while (!queue.empty())
    {
        auto ptr = queue.front();
        queue.pop();
        if (sink)
        {
            sink->requeue_buffer(ptr.tcam_buffer);
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
#if !NDEBUG
            bool found = std::any_of(tcamprop_interface_.tcamprop_properties.begin(),
                                     tcamprop_interface_.tcamprop_properties.end(),
                                     [name = prop->get_property_name()](auto& existing_prop)
                                     { return existing_prop->get_property_name() == name; });
            if (found)
            {
                SPDLOG_WARN("Property with name='{}' already in the property list.",
                            prop->get_property_name());
            }

#endif
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
        tcambind::convert_videoformatsdescription_to_caps(*dev, dev->get_available_video_formats());
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
