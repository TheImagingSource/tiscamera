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

#pragma once

#include "../../../libs/gst-helper/include/tcamprop1.0_gobject/tcam_property_provider.h"
#include "../../tcam.h"
#include "gsttcammainsrc.h"

#include <condition_variable>
#include <gst-helper/helper_functions.h>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <tcamprop1.0_base/tcamprop_property_interface.h>
#include <vector>

namespace tcam::mainsrc
{
struct src_interface_list : tcamprop1::property_list_interface
{
    std::vector<std::unique_ptr<tcamprop1::property_interface>> tcamprop_properties;

    auto get_property_list() -> std::vector<std::string_view> final
    {
        std::vector<std::string_view> ret;

        ret.reserve(tcamprop_properties.size());

        for (const auto& v : tcamprop_properties) { ret.push_back(v->get_property_name()); }
        return ret;
    }
    auto find_property(std::string_view name) -> tcamprop1::property_interface* final
    {
        for (const auto& v : tcamprop_properties)
        {
            if (name == v->get_property_name())
            {
                return v.get();
            }
        }
        return nullptr;
    }
    void clear() noexcept
    {
        tcamprop_properties.clear();
    }
};
} // namespace tcam::mainsrc

struct device_state
{
    std::shared_ptr<tcam::CaptureDevice> dev;
    std::shared_ptr<tcam::ImageSink> sink;
    std::queue<std::shared_ptr<tcam::ImageBuffer>> queue;

    gst_helper::gst_ptr<GstCaps> all_caps;

    std::mutex stream_mtx_;
    std::condition_variable stream_cv_;

    std::string device_serial;
    tcam::TCAM_DEVICE_TYPE device_type = tcam::TCAM_DEVICE_TYPE_UNKNOWN;

    std::atomic<bool> is_running = false;

    int n_buffers = -1;
    uint64_t frame_count = 0;

    tcam::mainsrc::src_interface_list tcamprop_interface_;
    tcamprop1_gobj::tcam_property_provider tcamprop_container_;

    gst_helper::gst_ptr<GstStructure> prop_init_;

    bool is_device_open() const noexcept
    {
        return dev != nullptr;
    }

    void stop_and_clear();

    void close();

    void populate_tcamprop_interface();
};

bool mainsrc_init_camera(GstTcamMainSrc* self);
