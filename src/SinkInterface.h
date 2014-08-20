

#ifndef SINKINTERFACE_H_
#define SINKINTERFACE_H_

#include "MemoryBuffer.h"

#include <memory>

namespace tis_imaging
{

class SinkInterface
{
public:

    virtual ~SinkInterface () {};

    virtual void pushImage (std::shared_ptr<MemoryBuffer>) = 0;
    
};

} /* namespace tis_imaging */

#endif /* SINKINTERFACE_H_ */

