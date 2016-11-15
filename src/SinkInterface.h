/*
 * Copyright 2014 The Imaging Source Europe GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TCAM_SINKINTERFACE_H
#define TCAM_SINKINTERFACE_H

#include "MemoryBuffer.h"

#include "VideoFormat.h"

#include <memory>
#include <vector>

namespace tcam
{

class SinkInterface
{
public:

    virtual ~SinkInterface () {};

    virtual bool set_status (TCAM_PIPELINE_STATUS) = 0;

    virtual TCAM_PIPELINE_STATUS get_status () const = 0;

    virtual bool setVideoFormat (const VideoFormat&) = 0;

    virtual VideoFormat getVideoFormat () const = 0;

    virtual void push_image (std::shared_ptr<MemoryBuffer>) = 0;

    virtual std::vector<std::shared_ptr<MemoryBuffer>> get_buffer_collection () = 0;

};

} /* namespace tcam */

#endif /* TCAM_SINKINTERFACE_H */
