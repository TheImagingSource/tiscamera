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

#include "DeviceInterface.h"
#include "FormatHandlerInterface.h"

#include <arv.h>
#include <atomic>

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
        std::vector<double> get_framerates(const struct tcam_image_size&, int pixelformat = 0);

    protected:
        AravisDevice* device;
    };

public:
    AravisDevice(const DeviceInfo&);

    AravisDevice() = delete;

    ~AravisDevice();

    DeviceInfo get_device_description() const override;

    std::vector<std::shared_ptr<tcam::property::IPropertyBase>> get_properties() final
    {
        return m_properties;
    };

    bool set_video_format(const VideoFormat&) override;

    VideoFormat get_active_video_format() const override;

    std::vector<VideoFormatDescription> get_available_video_formats() override;

    bool set_sink(std::shared_ptr<SinkInterface>) override;

    bool initialize_buffers(std::vector<std::shared_ptr<ImageBuffer>>) override;
    bool release_buffers() override;

    void requeue_buffer(std::shared_ptr<ImageBuffer>) override;

    bool start_stream() override;

    bool stop_stream() override;

private:
    // helper function to set
    // aravis packet-request-ratio
    void determine_packet_request_ratio();

    // helper function to set packet size
    // depending on env and auto negotiation
    void auto_set_packet_size();

    static void callback(ArvStream* stream, void* user_data);

    static void device_lost(ArvGvDevice* device, void* user_data);

    std::shared_ptr<AravisFormatHandler> format_handler;

    ArvCamera* arv_camera;

    std::weak_ptr<SinkInterface> external_sink;

    std::vector<std::shared_ptr<tcam::property::IPropertyBase>> m_properties;
    std::shared_ptr<tcam::property::AravisPropertyBackend> m_backend;

    ArvStream* stream;
    ArvGc* genicam;

    struct buffer_info
    {
        std::shared_ptr<ImageBuffer> buffer;
        ArvBuffer* arv_buffer;
        bool is_queued;
    };

    std::vector<buffer_info> buffers;

    struct tcam_stream_statistics statistics;
    std::atomic<bool> is_lost = false;
    struct aravis_options
    {
        bool auto_socket_buffer;
        unsigned int packet_timeout;
        unsigned int frame_retention;
        double packet_request_ratio;
    };

    // these options are used to define aravis behaviour
    // they are generally set to assure well defined interaction
    struct aravis_options arv_options;

    VideoFormat active_video_format;

    std::vector<VideoFormatDescription> available_videoformats;


    // found nodes that contain format information
    std::vector<ArvGcNode*> format_nodes;

    void determine_active_video_format();

    void index_genicam();
    void iterate_genicam(const char* feature);
    void index_genicam_format(ArvGcNode* /* node */);

    void index_properties(const char* name);

}; /* class GigeCapture */

} /* namespace tcam */

VISIBILITY_POP

#endif /* TCAM_ARAVISDEVICE_H */
