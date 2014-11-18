


#ifndef BAYERRGBFILTER_H_
#define BAYERRGBFILTER_H_

#include <FilterBase.h>

#include <vector>

#include "compiler_defines.h"

VISIBILITY_INTERNAL

namespace tcam
{

class BayerRgbFilter : public FilterBase
{
public:
    BayerRgbFilter ();

    struct FilterDescription getDescription () const;

    // bool init (const VideoFormat&);

    bool transform (MemoryBuffer& in, MemoryBuffer& out);

    bool apply (std::shared_ptr<MemoryBuffer>);

    bool setStatus (TCAM_PIPELINE_STATUS);

    TCAM_PIPELINE_STATUS getStatus () const;

    void getVideoFormat (VideoFormat& in, VideoFormat& out) const;

    bool setVideoFormat (const VideoFormat&);

    bool setVideoFormat (const VideoFormat& in, const VideoFormat& out);

    void setDeviceProperties (std::vector<std::shared_ptr<Property>>);

    std::vector<std::shared_ptr<Property>> getFilterProperties ();

private:

    TCAM_PIPELINE_STATUS status;
    FilterDescription description;

    VideoFormat input_format;
    VideoFormat output_format;

};



extern "C"
{

   // FilterBase* create ();

    // void destroy (FilterBase*);

    FB* create ();

    void destroy (FB*);

}

} /* namespace tcam */

VISIBILITY_POP

#endif /* _BAYERRGBFILTER_H_ */
