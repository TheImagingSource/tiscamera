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

#include "ImageSink.h"

#include <iostream>

using namespace tcam;


ImageSink::ImageSink ()
    : status(TCAM_PIPELINE_UNDEFINED), callback(nullptr), user_data(nullptr)
{}


bool ImageSink::set_status (TCAM_PIPELINE_STATUS s)
{
    if (status == s)
        return true;

    status = s;

    if (status == TCAM_PIPELINE_PLAYING)
    {
        std::cout << "Pipeline started playing" << std::endl;
    }
    else if (status == TCAM_PIPELINE_STOPPED)
    {
        std::cout << "Pipeline stopped playing" << std::endl;
    }

    return true;
}


TCAM_PIPELINE_STATUS ImageSink::get_status () const
{
    return status;
}


bool ImageSink::registerCallback (sink_callback sc, void* ud)
{
    this->callback = sc;
    this->user_data = ud;

    return true;
}


bool ImageSink::registerCallback (c_callback cc, void* ud)
{
    this->c_back = cc;
    this->user_data = ud;

    return true;
}


void ImageSink::push_image (std::shared_ptr<MemoryBuffer> buffer)
{
    last_image_buffer = buffer->getImageBuffer();
    if (callback != nullptr)
    {
        this->callback(&*buffer, user_data);
    }
    else if (c_back != nullptr)
    {
        this->c_back(&last_image_buffer, user_data);
    }
}


bool ImageSink::set_buffer_number (size_t)
{
    return false;
}


bool ImageSink::add_buffer_collection (std::vector<MemoryBuffer>)
{
    return false;
}


bool ImageSink::delete_buffer_collection ()
{
    return false;
}
