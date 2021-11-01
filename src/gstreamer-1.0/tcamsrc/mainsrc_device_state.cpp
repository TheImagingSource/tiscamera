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

void device_state::stop_and_clear()
{
    if (dev)
    {
        dev->stop_stream();
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
    if (dev)
    {
        stop_and_clear();

        dev = nullptr;
        sink = nullptr;
        all_caps.reset();
    }
}

void device_state::populate_tcamprop_interface()
{
    auto properties = dev->get_properties();

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

bool mainsrc_init_camera(GstTcamMainSrc* self)
{
    self->device->dev = nullptr;
    self->device->all_caps.reset();

    GST_DEBUG_OBJECT(self,
                     "Trying to open device with serial='%s and type='%s'.",
                     self->device->device_serial.c_str(),
                     tcam::tcam_device_type_to_string(self->device->device_type).c_str());

    self->device->dev = tcam::open_device(self->device->device_serial, self->device->device_type);
    if (!self->device->dev)
    {
        GST_ELEMENT_ERROR(self, RESOURCE, NOT_FOUND, ("Failed to open device."), (NULL));
        self->device->close();
        return false;
    }

    auto format = self->device->dev->get_available_video_formats();

    auto caps = tcambind::convert_videoformatsdescription_to_caps(format);
    if (caps == nullptr || gst_caps_get_size(caps.get()) == 0)
    {
        GST_ELEMENT_ERROR(self, CORE, CAPS, ("Failed to create caps for device."), (NULL));
        self->device->close();
        return false;
    }

    GST_DEBUG_OBJECT(
        self, "Device provides the following caps: %s", gst_helper::to_string(*caps).c_str());

    self->device->all_caps = caps;

    self->device->device_serial = self->device->dev->get_device().get_serial();
    self->device->device_type = self->device->dev->get_device().get_device_type();

    self->device->populate_tcamprop_interface();

    return true;
}
