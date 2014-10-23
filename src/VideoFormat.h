



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

    explicit VideoFormat (const struct video_format&);

    VideoFormat (const VideoFormat&);

    VideoFormat& operator= (const VideoFormat&);

    bool operator== (const VideoFormat&) const;

    bool operator!= (const VideoFormat& other) const;

    struct video_format getFormatDescription () const;

    uint32_t getFourcc () const;

    void setFourcc (uint32_t);

    double getFramerate () const;

    void setFramerate (double);

    struct IMG_SIZE getSize () const;

    void setSize (unsigned int width, unsigned int height);

    unsigned int getBinning () const;

    void setBinning (unsigned int binning);

    std::string toString () const;

    bool fromString (const std::string&);

    uint64_t getRequiredBufferSize () const;

    uint32_t getPitchSize () const;

private:

    struct video_format format;

};


} /*namespace tis_imaging */

#endif /* VIDEOFORMAT_H_ */
