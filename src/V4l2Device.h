


#ifndef USBCAPTURE_H_
#define USBCAPTURE_H_

#include "DeviceInterface.h"
#include "VideoFormat.h"
#include "VideoFormatDescription.h"

#include <linux/videodev2.h>
#include <memory>

namespace tis_imaging
{

class V4l2Device : public DeviceInterface, public std::enable_shared_from_this<V4l2Device>

{

public:

    V4l2Device (const CaptureDevice&);

    V4l2Device () = delete;

    ~V4l2Device ();

    CaptureDevice getDeviceDescription () const;

    std::vector<std::shared_ptr<Property>> getProperties ();

    bool isAvailable (const Property&);

    bool setProperty (const Property&);

    bool getProperty (Property&);
    
    bool setVideoFormat (const VideoFormat&);

    bool validateVideoFormat (const VideoFormat&);
    
    VideoFormat getActiveVideoFormat () const;

    std::vector<VideoFormatDescription> getAvailableVideoFormats () const;

    bool setFramerate (double framerate);
    
private:
    
    CaptureDevice device;

    int fd;

    VideoFormat active_video_format;
    
    std::vector<VideoFormatDescription> available_videoformats;


    // v4l2 uses fractions
    // to assure correct handling we store the values received by v4l2
    // to reuse them when necessary
    struct framerate_conv
    {
        double fps;
        int numerator;
        int denominator;
    };

    std::vector<framerate_conv> framerate_conversions;

    /**
     * @brief iterate over all v4l2 format descriptions and convert them
     *        into the internal representation
     */
    void index_formats ();

    std::vector<double> index_framerates (const struct v4l2_frmsizeenum& frms);


    struct property_description
    {
        int id; // v4l2 identification
        std::shared_ptr<Property> prop; 
    } ;

    std::vector<property_description> properties;

    void index_all_controls (std::shared_ptr<PropertyImpl> impl);

    int index_control (struct v4l2_queryctrl* qctrl, std::shared_ptr<PropertyImpl> impl);

    void add_control (struct v4l2_queryctrl* queryctrl,
                      struct v4l2_ext_control* ctrl,
                      std::shared_ptr<PropertyImpl> impl);

    bool propertyChangeEvent (const Property&);

    bool changeV4L2Control (const property_description&);
    
};

} /* namespace tis_imaging */

#endif /* USBCAPTURE_H_ */









