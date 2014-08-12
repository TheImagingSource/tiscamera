



#ifndef VIDEOFORMAT_H_
#define VIDEOFORMAT_H_

#include "base_types.h"

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

    struct video_format getFormatDescription () const;

    uint32_t getFourcc () const;

    struct SIZE getSize () const;
    
private:

    struct video_format format;
    
};


} /*namespace tis_imaging */

#endif /* VIDEOFORMAT_H_ */
