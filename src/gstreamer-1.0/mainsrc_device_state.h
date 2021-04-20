/*
 * Copyright 2021 The Imaging Source Europe GmbH
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

#include "gst_helper.h"
#include "gsttcammainsrc.h"
#include "tcam.h"

#include <memory>
#include <queue>

struct device_state
{
    DeviceIndex index_;

    std::shared_ptr<tcam::CaptureDevice> dev;
    std::shared_ptr<tcam::ImageSink> sink;
    std::queue<std::shared_ptr<tcam::ImageBuffer>> queue;

    gst_helper::gst_unique_ptr<GstCaps> all_caps;

    std::mutex mtx;
    std::condition_variable cv;

    void stop_and_clear()
    {
        if (dev)
        {
            dev->stop_stream();
        }
        while (!queue.empty())
        {
            auto ptr = queue.front();
            queue.pop();
            if (sink)
            {
                sink->requeue_buffer(ptr);
            }
        }
    }

    void close()
    {
        stop_and_clear();

        dev = nullptr;
        sink = nullptr;
    }
};


bool mainsrc_init_camera(GstTcamMainSrc* self);

void mainsrc_close_camera(GstTcamMainSrc* self);
