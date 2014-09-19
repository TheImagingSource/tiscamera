


#ifndef BAYERRGBFILTER_H_
#define BAYERRGBFILTER_H_

#include <FilterBase.h>

#include <vector>

namespace tis_imaging
{

class BayerRgbFilter : public FilterBase
{
public:
    BayerRgbFilter ();
    
    struct FilterDescription getDescription () const;

    // bool init (const VideoFormat&);
    
    bool transform (MemoryBuffer& in, MemoryBuffer& out);

    bool apply (std::shared_ptr<MemoryBuffer>);
    
    bool setStatus (const PIPELINE_STATUS&);

    PIPELINE_STATUS getStatus () const;

    void getVideoFormat (VideoFormat& in, VideoFormat& out) const;

    bool setVideoFormat (const VideoFormat&);

    bool setVideoFormat (const VideoFormat& in, const VideoFormat& out);
    
    void setDeviceProperties (std::vector<std::shared_ptr<Property>>);

    std::vector<std::shared_ptr<Property>> getFilterProperties ();

private:

    PIPELINE_STATUS status;
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

}
#endif /* _BAYERRGBFILTER_H_ */

