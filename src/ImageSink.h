



#ifndef IMAGESINK_H_
#define IMAGESINK_H_

#include "base_types.h"
#include "SinkInterface.h"

#include <memory>

typedef void (*sink_callback)(std::shared_ptr<tis_imaging::MemoryBuffer>, void*);

namespace tis_imaging
{

class ImageSink : public SinkInterface
{
public:

    ImageSink ();

    bool setStatus (PIPELINE_STATUS);
    PIPELINE_STATUS getStatus () const;

    bool registerCallback (sink_callback, void*);

    void pushImage(std::shared_ptr<MemoryBuffer>);

private:

    PIPELINE_STATUS status;

    sink_callback callback;
    void* user_data;
};

} /* namespace tis_imaging */


#endif /* IMAGESINK_H_ */










