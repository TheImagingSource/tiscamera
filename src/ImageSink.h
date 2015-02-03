



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
typedef void (*c_callback)(const struct tcam_image_buffer*, void*);

namespace tcam
{

class ImageSink : public SinkInterface
{
public:

    ImageSink ();

    bool setStatus (TCAM_PIPELINE_STATUS);
    TCAM_PIPELINE_STATUS getStatus () const;

    bool registerCallback (sink_callback, void*);
    bool registerCallback (c_callback, void*);

    void pushImage(std::shared_ptr<MemoryBuffer>);

private:

    TCAM_PIPELINE_STATUS status;

    sink_callback callback;
    c_callback c_back;
    void* user_data;

    struct tcam_image_buffer last_image_buffer;
};

} /* namespace tcam */

/** @} */

#endif /* TCAM_IMAGESINK_H */
