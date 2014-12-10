

#ifndef TCAM_SINKINTERFACE_H
#define TCAM_SINKINTERFACE_H

#include "MemoryBuffer.h"

#include <memory>

namespace tcam
{

class SinkInterface
{
public:

    virtual ~SinkInterface () {};

    virtual bool setStatus (TCAM_PIPELINE_STATUS) = 0;

    virtual TCAM_PIPELINE_STATUS getStatus () const = 0;

    virtual void pushImage (std::shared_ptr<MemoryBuffer>) = 0;

};

} /* namespace tcam */

#endif /* TCAM_SINKINTERFACE_H */
