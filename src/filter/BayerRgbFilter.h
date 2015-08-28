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

#ifndef TCAM_BAYERRGBFILTER_H
#define TCAM_BAYERRGBFILTER_H

#include <FilterBase.h>

#include <vector>

#include "compiler_defines.h"

VISIBILITY_INTERNAL

namespace tcam
{

class BayerRgbFilter : public FilterBase
{
public:
    BayerRgbFilter ();

    struct FilterDescription getDescription () const;

    // bool init (const VideoFormat&);

    bool transform (MemoryBuffer& in, MemoryBuffer& out);

    bool apply (std::shared_ptr<MemoryBuffer>);

    bool setStatus (TCAM_PIPELINE_STATUS);

    TCAM_PIPELINE_STATUS getStatus () const;

    void getVideoFormat (VideoFormat& in, VideoFormat& out) const;

    bool setVideoFormat (const VideoFormat&);

    bool setVideoFormat (const VideoFormat& in, const VideoFormat& out);

    void setDeviceProperties (std::vector<std::shared_ptr<Property>>);

    std::vector<std::shared_ptr<Property>> getFilterProperties ();

private:

    TCAM_PIPELINE_STATUS status;
    FilterDescription description;

    VideoFormat input_format;
    VideoFormat output_format;

};



extern "C"
{

   // FilterBase* create ();

    // void destroy (FilterBase*);

    FB* create ();

    void destroy (FB*);

}

} /* namespace tcam */

VISIBILITY_POP

#endif /* _TCAM_BAYERRGBFILTER_H */
