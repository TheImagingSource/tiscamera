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

#include "../../tcam.h"
#include "gsttcambufferpool.h"
#include "gsttcammainsrc.h"

#include <condition_variable>
#include <gst-helper/helper_functions.h>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <tcamprop1.0_base/tcamprop_property_interface.h>
#include <tcamprop1.0_gobject/tcam_property_provider.h>
#include <vector>

namespace tcam::mainsrc
{


tcam::TCAM_MEMORY_TYPE io_mode_to_memory_type(GstTcamIOMode mode);

GstTcamIOMode memory_type_to_io_mode(tcam::TCAM_MEMORY_TYPE t);

gboolean caps_to_format(GstCaps& c, tcam::tcam_video_format& format);


struct buffer_info
{
    void* addr = nullptr;
    GstBuffer* gst_buffer = nullptr;
    std::shared_ptr<tcam::ImageBuffer> tcam_buffer;
    bool pooled;
};

//std::vector<buffer_info> get_buffer_collection(GstTcamBufferPool* pool);

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
public:
    device_state(GstTcamMainSrc* parent) noexcept : parent_ { parent } {}

public: // data members currently still used in gstmainsrc.cpp, maybe move them into this
    std::shared_ptr<tcam::CaptureDevice> device_;
    std::shared_ptr<tcam::ImageSink> sink;

    std::shared_ptr<tcam::BufferPool> buffer_pool;
    tcam::VideoFormat format_;

    GstTcamIOMode io_mode_ = GST_TCAM_IO_AUTO;

public: // streaming stuff
    std::mutex stream_mtx_;
    std::condition_variable stream_cv_;
    std::atomic<bool> is_streaming_ = false;

    std::queue<tcam::mainsrc::buffer_info> queue;

public: // sink init properties, should be moved into this object
    int imagesink_buffers_ = 10;
    bool drop_incomplete_frames_ = true;

public: // members used for num-buffers functionality
    int n_buffers_ = -1;
    uint64_t n_buffers_delivered_ = 0;

public: // init properties get/set methods. Note: These take the device_open_mutex_ lock internally
    bool set_device_serial(const std::string& str) noexcept;
    bool set_device_type(tcam::TCAM_DEVICE_TYPE type) noexcept;

    void set_tcam_properties(const GstStructure* ptr) noexcept;
    auto get_tcam_properties() noexcept -> gst_helper::gst_ptr<GstStructure>;

    std::string get_device_serial() const noexcept;
    tcam::TCAM_DEVICE_TYPE get_device_type() const noexcept;

    // Returns a copy of the device caps when open, otherwise nullptr
    GstCaps* get_device_caps() const;

public:
    bool is_device_open() const noexcept
    {
        return device_ != nullptr;
    }

    bool configure_stream();
    void start_stream();
    void stop_stream();

    void stop_and_clear();
    void close();
    bool open_camera();

    void apply_properties(const GstStructure& strct);

    auto get_container() -> tcamprop1_gobj::tcam_property_provider&
    {
        return tcamprop_container_;
    }

private:
    mutable std::mutex
        device_open_mutex_; // this is mutable because we want to use this in logically const methods

    // init properties, these get cleared on successful open
    std::string device_serial_to_open_;
    tcam::TCAM_DEVICE_TYPE device_type_to_open_ = tcam::TCAM_DEVICE_TYPE_UNKNOWN;
    gst_helper::gst_ptr<GstStructure> prop_init_;


    // cache for the device caps
    gst_helper::gst_ptr<GstCaps> all_caps_;

    // Reference to the GstElement owning this device_state instance
    GstTcamMainSrc* parent_ = nullptr;

    // stuff for TcamPropertyProvider
    tcam::mainsrc::src_interface_list tcamprop_interface_;
    tcamprop1_gobj::tcam_property_provider tcamprop_container_;

    void populate_tcamprop_interface();
};
