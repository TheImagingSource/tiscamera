



#ifndef VIDEOFORMAT_H_
#define VIDEOFORMAT_H_

#include "base_types.h"

#include <string>

namespace tcam
{

/**
 * @class VideoFormat
 * Description of a specific format
 */
class VideoFormat
{

public:

    VideoFormat ();

    explicit VideoFormat (const struct tcam_video_format&);

    VideoFormat (const VideoFormat&);

    VideoFormat& operator= (const VideoFormat&);

    bool operator== (const VideoFormat&) const;

    bool operator!= (const VideoFormat& other) const;


    /**
     * Returns a struct representation of the format
     * @return tcam_video_format
     */
    struct tcam_video_format getStruct () const;

    /**
     * Returns the used pixel format
     * @return uint32 containing the fourcc
     */
    uint32_t getFourcc () const;

    void setFourcc (uint32_t);

    /**
     * @return framerate in frames/second
     */
    double getFramerate () const;

    void setFramerate (double);

    struct tcam_image_size getSize () const;

    void setSize (unsigned int width, unsigned int height);

    std::string toString () const;

    bool fromString (const std::string&);

    /**
     * Description for getRequiredBufferSize.
     * @return size in bytes an image with this format will have
     */
    uint64_t getRequiredBufferSize () const;

    /**
     * Description for getPitchSize.
     * @return the size og an image line
     */
    uint32_t getPitchSize () const;

private:

    struct tcam_video_format format;

};


} /*namespace tcam */

#endif /* VIDEOFORMAT_H_ */
