


#ifndef TCAM_ARAVISDEVICE_H
#define TCAM_ARAVISDEVICE_H

#include "DeviceInterface.h"

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

public:

    AravisDevice (const DeviceInfo&);

    AravisDevice () = delete;

    ~AravisDevice ();

    DeviceInfo get_device_description () const;

    std::vector<std::shared_ptr<Property>> getProperties () ;

    bool set_property (const Property&);

    bool get_property (Property&);

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

    DeviceInfo device;

    std::shared_ptr<AravisPropertyHandler> handler;

    ArvCamera* arv_camera;

    std::shared_ptr<SinkInterface> external_sink;

    ArvStream* stream;
    ArvGc* genicam;

    std::vector<std::shared_ptr<MemoryBuffer>> buffers;
    unsigned int current_buffer;

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

    static uint32_t aravis2fourcc (uint32_t aravis);
    static uint32_t fourcc2aravis (uint32_t fourcc);

    void index_genicam ();
    void iterate_genicam (const char* feature);
    void index_genicam_format (ArvGcNode* /* node */ );

}; /* class GigeCapture */

} /* namespace tcam */

VISIBILITY_POP

#endif /* TCAM_ARAVISDEVICE_H */
