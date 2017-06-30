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

VISIBILITY_INTERNAL

namespace tcam
{

class AravisDevice : public DeviceInterface
{
    struct property_mapping
    {
        std::shared_ptr<Property> prop;
        std::string arv_ident;
    };

    class AravisPropertyHandler : public PropertyImpl
    {
        friend class AravisDevice;
    public:
        AravisPropertyHandler (AravisDevice*);

        bool get_property (Property&);
        bool set_property (const Property&);

    protected:
        std::vector<property_mapping> properties;
        std::vector<property_mapping> special_properties;


        AravisDevice* device;
    };

    class AravisFormatHandler : public FormatHandlerInterface
    {
        friend class AravisDevice;

    public:
        AravisFormatHandler (AravisDevice*);
        std::vector<double> get_framerates (const struct tcam_image_size&, int pixelformat=0);
    protected:
        AravisDevice* device;
    };

public:

    AravisDevice (const DeviceInfo&);

    AravisDevice () = delete;

    ~AravisDevice ();

    DeviceInfo get_device_description () const;

    std::vector<std::shared_ptr<Property>> getProperties () ;

    bool set_property (const Property&);

    bool get_property (Property&);

    void update_property (struct property_mapping& mapping);

    bool set_video_format (const VideoFormat&);

    VideoFormat get_active_video_format () const;

    std::vector<VideoFormatDescription> get_available_video_formats ();

    bool set_sink (std::shared_ptr<SinkInterface>);

    bool initialize_buffers (std::vector<std::shared_ptr<MemoryBuffer>>);
    bool release_buffers ();

    bool start_stream ();

    bool stop_stream ();

private:

    static void callback(ArvStream* stream, void* user_data);

    static void device_lost (ArvGvDevice* device, void* user_data);

    std::shared_ptr<AravisPropertyHandler> handler;

    std::shared_ptr<AravisFormatHandler> format_handler;

    ArvCamera* arv_camera;

    std::weak_ptr<SinkInterface> external_sink;

    ArvStream* stream;
    ArvGc* genicam;

    struct buffer_info
    {
        std::shared_ptr<MemoryBuffer> buffer;
        bool is_queued;
    };

    //std::vector<std::shared_ptr<MemoryBuffer>> buffers;
    std::vector<buffer_info> buffers;
    unsigned int current_buffer;
    struct tcam_stream_statistics statistics;

    struct aravis_options
    {
        bool auto_socket_buffer;
        bool no_packet_resend;
        unsigned int packet_timeout;
        unsigned int frame_retention;
    };

    // these options are used to define aravis behaviour
    // they are generally set to assure well defined interaction
    struct aravis_options arv_options;

    VideoFormat active_video_format;

    std::vector<VideoFormatDescription> available_videoformats;


    // found nodes that contain format information
    std::vector<ArvGcNode*> format_nodes;

    void determine_active_video_format ();

    void index_genicam ();
    void iterate_genicam (const char* feature);
    void index_genicam_format (ArvGcNode* /* node */ );

}; /* class GigeCapture */

} /* namespace tcam */

VISIBILITY_POP

#endif /* TCAM_ARAVISDEVICE_H */
