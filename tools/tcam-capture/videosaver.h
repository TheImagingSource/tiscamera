/*
 * Copyright 2022 The Imaging Source Europe GmbH
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

#pragma once

#include "gst/gstelement.h"
#include "gst/gstmessage.h"
#include <QString>
#include <gst/gst.h>

#include "config.h"

namespace tcam::tools::capture
{

class VideoSaver
{
public:

    VideoSaver() = delete;
    VideoSaver(GstPipeline*, const QString& filename, VideoCodec);

    // create and attach saving bin
    void start_saving();
    // stop saving and wait for EOS
    void stop_saving();

    // remove and destroy saving bin
    // should be called once EOS has been
    // received on bus
    void destroy_pipeline();

    // return pointer of gst-element
    // responsible for messages on GstBus
    // typically the sink
    GstObject* gst_pointer() const;

private:

    GstElement* pipeline_ = nullptr;
    QString target_file_;
    VideoCodec codec_;

    GstElement* save_pipeline_ = nullptr;

    // tmp storage stuff
    GstElement* tee_ = nullptr;
    GstPad* tee_pad_ = nullptr;
    GstElement* queue_ = nullptr;
    GstPad* queue_pad_ = nullptr;
};

} // namespace tcam::tools::capture
