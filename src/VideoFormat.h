



#ifndef VIDEOFORMAT_H_
#define VIDEOFORMAT_H_

#include "base_types.h"

#include <string>

namespace tis_imaging
{

class VideoFormat
{

public:
    
    VideoFormat ();
    
    VideoFormat (const struct video_format&);
    
    VideoFormat (const VideoFormat&);

    VideoFormat& operator= (const VideoFormat&);
    
    ~VideoFormat ();

    bool operator== (const VideoFormat&) const;

    bool operator!= (const VideoFormat& other) const;

    struct video_format getFormatDescription () const;

    uint32_t getFourcc () const;

    void setFourcc (const uint32_t&);

    double getFramerate () const;

    void setFramerate (const double&);

    struct IMG_SIZE getSize () const;

    void setSize (const unsigned int& width, const unsigned int& height);

    unsigned int getBinning () const;

    void setBinning (const unsigned int binning);

    std::string toString () const;

    bool fromString (const std::string&);
    
private:

    struct video_format format;
    
};


} /*namespace tis_imaging */

#endif /* VIDEOFORMAT_H_ */
