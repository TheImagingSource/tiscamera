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

#include "DeviceInterface.h"
#include "SinkInterface.h"
#include "base_types.h"
#include "compiler_defines.h"

#include <chrono>
#include <memory>

VISIBILITY_INTERNAL

namespace tcam
{

class ImageSource : public SinkInterface, public std::enable_shared_from_this<ImageSource>
{

public:
    ImageSource();

    ~ImageSource();

    bool set_status(TCAM_PIPELINE_STATUS) override;

    TCAM_PIPELINE_STATUS get_status() const override;

    bool setDevice(std::shared_ptr<DeviceInterface>);

    bool setVideoFormat(const VideoFormat&) override;

    VideoFormat getVideoFormat() const override;

    void push_image(std::shared_ptr<ImageBuffer>) override;

    void requeue_buffer(std::shared_ptr<ImageBuffer>) override;

    bool setSink(std::shared_ptr<SinkInterface>);

    bool set_buffer_collection(const std::vector<std::shared_ptr<ImageBuffer>>& new_buffers);

    std::vector<std::shared_ptr<ImageBuffer>> get_buffer_collection() override;

    void drop_incomplete_frames(bool drop_them) override;
    bool should_incomplete_frames_be_dropped() const override;

private:
    TCAM_PIPELINE_STATUS current_status;

    std::shared_ptr<DeviceInterface> device;

    std::chrono::time_point<std::chrono::steady_clock> stream_start;

    std::vector<std::shared_ptr<ImageBuffer>> buffer;

    std::weak_ptr<SinkInterface> pipeline;

    bool drop_frames_ = true;
};

} /* namespace tcam */

VISIBILITY_POP

#endif /* TCAM_IMAGESOURCE_H */
