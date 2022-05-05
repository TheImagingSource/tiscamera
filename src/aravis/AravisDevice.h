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

#ifndef TCAM_ARAVISDEVICE_H
#define TCAM_ARAVISDEVICE_H

#include "../DeviceInterface.h"
#include "AravisAllocator.h"
#include "../FormatHandlerInterface.h"

#include <arv.h>
#include <atomic>
#include <mutex>
#include <optional>

VISIBILITY_INTERNAL


namespace tcam::aravis
{
class AravisPropertyBackend;
class AravisAllocator;
}

namespace tcam
{

class AravisDevice : public DeviceInterface
{
    friend aravis::AravisPropertyBackend;

public:
    AravisDevice(const DeviceInfo&);

    AravisDevice() = delete;

    ~AravisDevice();

    DeviceInfo get_device_description() const final
    {
        return device;
    }

    std::vector<std::shared_ptr<tcam::property::IPropertyBase>> get_properties() final
    {
        return properties_;
    }

    bool set_video_format(const VideoFormat&) final;

    VideoFormat get_active_video_format() const final
    {
        return active_video_format_;
    }

    std::vector<VideoFormatDescription> get_available_video_formats() final
    {
        return available_videoformats_;
    }

    std::shared_ptr<tcam::AllocatorInterface> get_allocator() final
    {
        return allocator_;
    };

    bool initialize_buffers(std::shared_ptr<BufferPool> pool) final;
    bool release_buffers() final;

    void requeue_buffer(const std::shared_ptr<ImageBuffer>&) final;

    bool start_stream(const std::shared_ptr<IImageBufferSink>&) final;

    void stop_stream() final;

    outcome::result<tcam::framerate_info> get_framerate_info(const VideoFormat& fmt) final;

private:
    tcam::VideoFormat read_camera_current_video_format();

    // helper function to set lifetime of control channel
    // depending on env and auto negotiation
    void auto_set_control_lifetime();

    // helper function to set packet size
    // depending on env and auto negotiation
    void auto_set_packet_size();

    static void aravis_new_buffer_callback(ArvStream* stream, void* user_data);

    static void device_lost(ArvGvDevice* device, void* user_data);

    std::recursive_mutex arv_camera_access_mutex_;

    ArvCamera* arv_camera_ = nullptr;
    ArvStream* stream_ = nullptr;
    ArvGc* genicam_ = nullptr;

    std::shared_ptr<tcam::aravis::AravisAllocator> allocator_ = nullptr;
    std::weak_ptr<IImageBufferSink> sink_;

    std::vector<std::shared_ptr<tcam::property::IPropertyBase>> properties_;
    std::vector<std::shared_ptr<tcam::property::IPropertyBase>> internal_properties_;
    std::shared_ptr<tcam::aravis::AravisPropertyBackend> backend_;


    struct buffer_info
    {
        AravisDevice* parent = nullptr;

        // The reference to the actual ImageBuffer
        std::shared_ptr<ImageBuffer> buffer;

        // contains the ArvBuffer used to queue/dequeue in ArbStream
        // This will be set to nullptr when the ArvStream owns this when we stop
        // otherwise this is != nullptr
        ArvBuffer* arv_buffer = nullptr;

#if !defined NDEBUG
        bool is_queued = false;
#endif
    };

    static void clear_buffer_info_arb_buffer(buffer_info& info);

    std::vector<buffer_info> buffer_list_;
    std::mutex buffer_list_mtx_;

    long frames_delivered_ = 0;
    long frames_dropped_ = 0;
    std::atomic<bool> is_lost_ = false;

    struct device_scaling
    {
        std::vector<std::shared_ptr<tcam::property::IPropertyBase>> properties;

        std::vector<image_scaling> scaling_info_list;

        ImageScalingType scale_type = ImageScalingType::Unknown;
    };

    device_scaling scale_;


    void determine_scaling_type();
    void generate_scaling_information();
    bool set_scaling(const image_scaling& scale);
    image_scaling get_current_scaling();

    VideoFormat active_video_format_;

    std::vector<VideoFormatDescription> available_videoformats_;

    void index_genicam();
    void generate_video_formats();

    void generate_properties_from_genicam();
    void create_property_list_from_genicam_categories();

    tcam_image_size get_sensor_size() const;

    void complete_aravis_stream_buffer(ArvBuffer* buffer, bool is_incomplete);

    bool has_offset_ = false;

    bool has_test_format_interface_ = false;
    bool has_test_binning_h_ = false;
    bool has_test_binning_v_ = false;
    bool has_test_skipping_h_ = false;
    bool has_test_skipping_v_ = false;
    bool has_FPS_enum_interface_ = false;

    auto fetch_test_itf_framerates(const VideoFormat& fmt) -> outcome::result<tcam::framerate_info>;

    bool has_genicam_property(const char* name) const;
    ArvGcNode* get_genicam_property_node(const char* name) const;

    template<class TItf> std::shared_ptr<TItf> find_cam_property(std::string_view name) const
    {
        auto ptr = tcam::property::find_property<TItf>(properties_, name);
        if (ptr)
            return ptr;
        return tcam::property::find_property<TItf>(internal_properties_, name);
    }

    void disable_chunk_mode();

}; /* class GigeCapture */

} /* namespace tcam */

VISIBILITY_POP

#endif /* TCAM_ARAVISDEVICE_H */
