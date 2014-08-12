



#ifndef VIDEOFORMATDESCRIPTION_H_
#define VIDEOFORMATDESCRIPTION_H_

#include "base_types.h"

#include "VideoFormat.h"

#include <vector>

namespace tis_imaging
{

class VideoFormatDescription
{
public:

    VideoFormatDescription () = delete;

    VideoFormatDescription (const struct video_format_description&,
                            const std::vector<double>&);

    
    VideoFormatDescription (const VideoFormatDescription&);

    
    VideoFormatDescription& operator= (const VideoFormatDescription&);

    bool operator== (const VideoFormatDescription& other);
    bool operator!= (const VideoFormatDescription& other);

    
    ~VideoFormatDescription ();

    
    struct video_format_description getFormatDescription () const;


    SIZE getSizeMin () const;

    
    SIZE getSizeMax () const;

    
    bool isValidVideoFormat (const VideoFormat&) const;


    
    std::vector<double> getFrameRates () const;
    
private:

    video_format_description format;

    std::vector<double> framerates;
};

} /* namespace tis_imaging */

#endif /* VIDEOFORMATDESCRIPTION_H_ */
