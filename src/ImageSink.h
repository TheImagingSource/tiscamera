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

    bool setVideoFormat (const VideoFormat&);

    VideoFormat getVideoFormat () const;

    bool registerCallback (sink_callback, void*);
    bool registerCallback (c_callback, void*);

    void push_image (std::shared_ptr<MemoryBuffer>);

    bool set_buffer_number (size_t);

    bool set_buffer_collection (std::vector<std::shared_ptr<MemoryBuffer>> new_buffers);

    std::vector<std::shared_ptr<MemoryBuffer>> get_buffer_collection ();

    bool delete_buffer_collection ()
;

private:

    bool initialize_internal_buffer ();

    TCAM_PIPELINE_STATUS status;
    VideoFormat format;

    sink_callback callback;
    c_callback c_back;
    void* user_data;

    struct tcam_image_buffer last_image_buffer;

    bool external_buffer;

    size_t buffer_number;
    std::vector<std::shared_ptr<MemoryBuffer>> buffers;

};

} /* namespace tcam */

/** @} */

#endif /* TCAM_IMAGESINK_H */
