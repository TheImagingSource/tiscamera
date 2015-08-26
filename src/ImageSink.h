



#ifndef TCAM_IMAGESINK_H
#define TCAM_IMAGESINK_H

#include "base_types.h"
#include "SinkInterface.h"

#include <memory>
#include <vector>

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

    bool set_status (TCAM_PIPELINE_STATUS);
    TCAM_PIPELINE_STATUS get_status () const;

    bool registerCallback (sink_callback, void*);
    bool registerCallback (c_callback, void*);

    void push_image (std::shared_ptr<MemoryBuffer>);

    bool set_buffer_number (size_t);

    bool add_buffer_collection (std::vector<MemoryBuffer>);
    bool delete_buffer_collection ()
;

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
