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

#include "multimedia.h"

#include <iostream>
#include <unistd.h>

using namespace tcam;


bool tcam::save_image (CaptureDevice& g, const std::string& filename)
{

    struct data
    {
        CaptureDevice* g;
        std::string filename;
        int count;
        bool wait;
    };

    auto f = [] (MemoryBuffer* buf, void* userdata)
        {
            data* d = static_cast<data*>(userdata);

            if (d->count >= 2)
            {
                //save_jpeg(*buf, d->filename);
                d->wait = false;
            }
            else
            {
                d->count++;
            }
        };

    auto sink = std::make_shared<ImageSink>();

    data d;
    d.g = &g;
    d.filename = filename;
    d.count = 0;
    d.wait = true;

    sink->registerCallback(f, &d);

    VideoFormat v;
    v.from_string("format=RGB24,width=640,height=480,binning=0,framerate=30.000000");
    g.set_video_format(v);
    g.start_stream(sink);

    // wait until image was captured
    while(d.wait)
    {
        usleep(500);
    }
    d.g->stop_stream();

    std::cout << "Successfully saved image" << std::endl;

    return true;
}


bool tcam::save_stream (CaptureDevice& g, const std::string& filename)
{
    return false;
}
