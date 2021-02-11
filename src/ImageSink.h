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

#include "SinkInterface.h"
#include "base_types.h"

#include <memory>
#include <vector>

/**
 * @addtogroup API
 * @{
 * Main header
 */

typedef void (*shared_callback)(std::shared_ptr<tcam::ImageBuffer>, void*);
typedef void (*sink_callback)(tcam::ImageBuffer*, void*);
typedef void (*c_callback)(const struct tcam_image_buffer*, void*);

namespace tcam
{

class ImageSink : public SinkInterface
{
public:
    ImageSink();

    bool set_status(TCAM_PIPELINE_STATUS) override;
    TCAM_PIPELINE_STATUS get_status() const override;

    bool setVideoFormat(const VideoFormat&) override;

    VideoFormat getVideoFormat() const override;

    bool registerCallback(shared_callback, void*);
    bool registerCallback(sink_callback, void*);
    bool registerCallback(c_callback, void*);

    void push_image(std::shared_ptr<ImageBuffer>) override;

    void requeue_buffer(std::shared_ptr<ImageBuffer>) override;

    bool set_buffer_number(size_t);

    bool set_buffer_collection(std::vector<std::shared_ptr<ImageBuffer>> new_buffers);

    std::vector<std::shared_ptr<ImageBuffer>> get_buffer_collection() override;

    bool delete_buffer_collection();

    /**
     * used to set the pipelinemanager instance that is called
     * for things like requeue_buffer
     */
    void set_source(std::weak_ptr<SinkInterface>) override;

    void drop_incomplete_frames(bool drop_them) override;
    bool should_incomplete_frames_be_dropped() const override;


private:
    bool initialize_internal_buffer();

    std::weak_ptr<SinkInterface> source_;

    TCAM_PIPELINE_STATUS status;
    VideoFormat format;

    shared_callback sh_callback;
    sink_callback callback;
    c_callback c_back;
    void* user_data;

    struct tcam_image_buffer last_image_buffer;

    bool external_buffer;

    size_t buffer_number;
    std::vector<std::shared_ptr<ImageBuffer>> buffers;
};

} /* namespace tcam */

/** @} */

#endif /* TCAM_IMAGESINK_H */
