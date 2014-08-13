


#ifndef GIGECAPTURE_H_
#define GIGECAPTURE_H_

#include "DeviceInterface.h"

#include <arv.h>

namespace tis_imaging
{

class GigECapture : public DeviceInterface, public std::enable_shared_from_this<GigECapture>
{

public:

    GigECapture (const CaptureDevice&);

    GigECapture () = delete;

    ~GigECapture ();

    CaptureDevice getDeviceDescription () const;

    std::vector<std::shared_ptr<Property>> getProperties () ;

    bool isAvailable (const Property&);

    bool setProperty (const Property&);
    
    bool getProperty (Property&);

    
    
    bool setVideoFormat (const VideoFormat&);

    VideoFormat getActiveVideoFormat () const;

    std::vector<VideoFormatDescription> getAvailableVideoFormats () const;

private:

    CaptureDevice device;
    
    ArvCamera* arv_camera;


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
    // std::shared_ptr<> buffer;

    std::vector<VideoFormatDescription> available_videoformats;


    // found nodes that contain format information
    std::vector<ArvGcNode*> format_nodes;

    void index_genicam ();
    void iterate_genicam (const char* feature);
    void index_genicam_format (ArvGcNode* /* node */ );

}; /* class GigeCapture */

} /* namespace tis_imaging */

#endif /* GIGECAPTURE_H_ */

