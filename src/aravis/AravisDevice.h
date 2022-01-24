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
#include "../FormatHandlerInterface.h"

#include <arv.h>
#include <atomic>
#include <mutex>

VISIBILITY_INTERNAL

namespace tcam
{

namespace property
{
class AravisPropertyBackend;
}

class AravisDevice : public DeviceInterface
{

    class AravisFormatHandler : public FormatHandlerInterface
    {
        friend class AravisDevice;

    public:
        AravisFormatHandler(AravisDevice*);
        std::vector<double> get_framerates(const struct tcam_image_size&,
                                           int pixelformat = 0) final;

    protected:
        AravisDevice* device;
    };

public:
    AravisDevice(const DeviceInfo&);

    AravisDevice() = delete;

    ~AravisDevice();

    DeviceInfo get_device_description() const final;

    std::vector<std::shared_ptr<tcam::property::IPropertyBase>> get_properties() final
    {
        return properties_;
    }

    bool set_video_format(const VideoFormat&) final;

    VideoFormat get_active_video_format() const final;

    std::vector<VideoFormatDescription> get_available_video_formats() final;

    bool initialize_buffers(std::vector<std::shared_ptr<ImageBuffer>>) final;
    bool release_buffers() final;

    void requeue_buffer(const std::shared_ptr<ImageBuffer>&) final;

    bool start_stream(const std::shared_ptr<IImageBufferSink>&) final;

    void stop_stream() final;

private:
    // helper function to set lifetime of control channel
    // depending on env and auto negotiation
    void auto_set_control_lifetime();

    // helper function to set packet size
    // depending on env and auto negotiation
    void auto_set_packet_size();

    static void aravis_new_buffer_callback(ArvStream* stream, void* user_data);

    static void device_lost(ArvGvDevice* device, void* user_data);

    std::shared_ptr<AravisFormatHandler> format_handler_;

    ArvCamera* arv_camera_ = nullptr;

    std::weak_ptr<IImageBufferSink> sink_;

    std::vector<std::shared_ptr<tcam::property::IPropertyBase>> properties_;
    std::vector<std::shared_ptr<tcam::property::IPropertyBase>> internal_properties_;
    std::shared_ptr<tcam::property::AravisPropertyBackend> backend_;

    ArvStream* stream_ = nullptr;
    ArvGc* genicam_ = nullptr;

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

        std::vector<image_scaling> scales;

        ImageScalingType scale_type = ImageScalingType::Unknown;
    };

    device_scaling scale_;


    void determine_scaling();
    void generate_scales();
    bool set_scaling(const image_scaling& scale);
    image_scaling get_current_scaling();

    VideoFormat active_video_format_;

    std::vector<VideoFormatDescription> available_videoformats_;
    bool has_offset_ = false;


    // found nodes that contain format information
    std::vector<ArvGcNode*> format_nodes_;

    void determine_active_video_format();

    void index_genicam();
    void iterate_genicam(const char* feature);
    void index_genicam_format(ArvGcNode* /* node */);

    void index_properties(const char* name);

    tcam_image_size get_sensor_size() const;

    void complete_aravis_stream_buffer(ArvBuffer* buffer, bool is_incomplete);

}; /* class GigeCapture */

} /* namespace tcam */

VISIBILITY_POP

#endif /* TCAM_ARAVISDEVICE_H */
