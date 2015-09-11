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

    bool setSink (std::shared_ptr<SinkInterface>);

    bool set_buffer_collection (std::vector<std::shared_ptr<MemoryBuffer>> new_buffers);

    std::vector<std::shared_ptr<MemoryBuffer>> get_buffer_collection ();

private:

    TCAM_PIPELINE_STATUS current_status;

    std::shared_ptr<DeviceInterface> device;

    std::chrono::time_point<std::chrono::steady_clock> stream_start;

    std::vector<std::shared_ptr<MemoryBuffer>> buffer;

    std::weak_ptr<SinkInterface> pipeline;

};

} /* namespace tcam */

VISIBILITY_POP

#endif /* TCAM_IMAGESOURCE_H */
