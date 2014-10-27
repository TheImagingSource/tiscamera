



#ifndef VIDEOFORMATDESCRIPTION_H_
#define VIDEOFORMATDESCRIPTION_H_

#include "base_types.h"

#include "VideoFormat.h"

#include <vector>

namespace tcam
{


struct res_fps
{
    IMG_SIZE resolution;
    std::vector<double> fps;
};


class VideoFormatDescription
{
public:

    VideoFormatDescription () = delete;

    VideoFormatDescription (const struct video_format_description&,
                            const std::vector<res_fps>&);

    VideoFormatDescription (const VideoFormatDescription&);


    VideoFormatDescription& operator= (const VideoFormatDescription&);

    bool operator== (const VideoFormatDescription& other) const;
    bool operator!= (const VideoFormatDescription& other) const;

    struct video_format_description getFormatDescription () const;

    uint32_t getFourcc () const;

    std::vector<res_fps> getResolutionsFramesrates () const;

    std::vector<IMG_SIZE> getResolutions () const;

    IMG_SIZE getSizeMin () const;
    IMG_SIZE getSizeMax () const;

    std::vector<double> getFrameRates (const IMG_SIZE& size) const;

    VideoFormat createVideoFormat (unsigned int width,
                                   unsigned int height,
                                   double framerate) const;

    bool isValidVideoFormat (const VideoFormat&) const;

    bool isValidFramerate (const double framerate) const;

    bool isValidResolution (unsigned int width, unsigned int height) const;

private:

    video_format_description format;

    std::vector<res_fps> rf;

};

} /* namespace tcam */

#endif /* VIDEOFORMATDESCRIPTION_H_ */
