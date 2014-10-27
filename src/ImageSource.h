



#ifndef IMAGESOURCE_H_
#define IMAGESOURCE_H_

#include "base_types.h"
#include "SinkInterface.h"
#include "DeviceInterface.h"

#include <ctime>
#include <memory>

namespace tcam
{

class ImageSource : public SinkInterface, public std::enable_shared_from_this<ImageSource>
{

public:

    ImageSource ();

    ~ImageSource ();

    bool setStatus (PIPELINE_STATUS);

    PIPELINE_STATUS getStatus () const;

    bool setDevice (std::shared_ptr<DeviceInterface>);

    bool setVideoFormat (const VideoFormat&);

    VideoFormat getVideoFormat () const;

    void pushImage (std::shared_ptr<MemoryBuffer>);

    void initialize_buffers ();

    bool setSink (std::shared_ptr<SinkInterface>);

private:

    PIPELINE_STATUS current_status;

    std::shared_ptr<DeviceInterface> device;

    unsigned int n_buffers;

    uint64_t frame_count;
    time_t second_count;

    std::vector<std::shared_ptr<MemoryBuffer>> buffers;

    std::weak_ptr<SinkInterface> pipeline;

};

} /* namespace tcam */

#endif /* IMAGESOURCE_H_ */
