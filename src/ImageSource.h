



#ifndef TCAM_IMAGESOURCE_H
#define TCAM_IMAGESOURCE_H

#include "base_types.h"
#include "SinkInterface.h"
#include "DeviceInterface.h"

#include <chrono>
#include <memory>

#include "compiler_defines.h"

VISIBILITY_INTERNAL

namespace tcam
{

class ImageSource : public SinkInterface, public std::enable_shared_from_this<ImageSource>
{

public:

    ImageSource ();

    ~ImageSource ();

    bool set_status (TCAM_PIPELINE_STATUS);

    TCAM_PIPELINE_STATUS get_status () const;

    bool setDevice (std::shared_ptr<DeviceInterface>);

    bool setVideoFormat (const VideoFormat&);

    VideoFormat getVideoFormat () const;

    void push_image (std::shared_ptr<MemoryBuffer>);

    void initialize_buffers ();

    bool setSink (std::shared_ptr<SinkInterface>);

private:

    TCAM_PIPELINE_STATUS current_status;

    std::shared_ptr<DeviceInterface> device;

    unsigned int n_buffers;

    std::chrono::time_point<std::chrono::steady_clock> stream_start;

    std::vector<std::shared_ptr<MemoryBuffer>> buffers;

    std::weak_ptr<SinkInterface> pipeline;

};

} /* namespace tcam */

VISIBILITY_POP

#endif /* TCAM_IMAGESOURCE_H */
