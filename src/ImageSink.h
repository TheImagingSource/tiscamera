



#ifndef IMAGESINK_H_
#define IMAGESINK_H_

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

#endif /* IMAGESINK_H_ */
