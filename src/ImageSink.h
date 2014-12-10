



#ifndef TCAM_IMAGESINK_H
#define TCAM_IMAGESINK_H

#include "base_types.h"
#include "SinkInterface.h"

#include <memory>

/**
 * @addtogroup API
 * @{
 * Main header
 */

typedef void (*sink_callback)(tcam::MemoryBuffer*, void*);

namespace tcam
{

class ImageSink : public SinkInterface
{
public:

    ImageSink ();

    bool setStatus (TCAM_PIPELINE_STATUS);
    TCAM_PIPELINE_STATUS getStatus () const;

    bool registerCallback (sink_callback, void*);

    void pushImage(std::shared_ptr<MemoryBuffer>);

private:

    TCAM_PIPELINE_STATUS status;

    sink_callback callback;
    void* user_data;
};

} /* namespace tcam */

/** @} */

#endif /* TCAM_IMAGESINK_H */
