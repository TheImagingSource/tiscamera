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

#include "formats.h"

#include <iostream>
#include <iomanip>

void list_formats (const std::vector<VideoFormatDescription>& available_formats)
{
    std::cout << "Available format settings:" << std::endl;
    for (const VideoFormatDescription& f : available_formats)
    {
        auto desc = f.getStruct();

        std::cout << "Format: " << desc.description << " - Fourcc(" << desc.fourcc << ")" << std::endl;
        for (const auto& s : f.getResolutionsFramesrates())
        {
            std::cout << "\tResolution: " << s.resolution.width << "x" << s.resolution.height << std::endl;
            for (const auto& fps : s.fps)
            {
                std::cout << "\t\t" << std::setw(8) << std::fixed << std::setprecision(4)<< fps << " fps" << std::endl;
            }
        }
        std::cout << std::endl;
    }
}


void print_active_format (const VideoFormat& format)
{
    std::cout << "Active format:\n"
              << "Format: \t" << tcam_fourcc_to_description(format.getFourcc())
              << "\nResolution: \t" << format.getSize().width << "x" << format.getSize().height
              << "\nFramerate: \t" << format.getFramerate() << "\n" << std::endl;
}


bool set_active_format (CaptureDevice& g, const std::string& new_format)
{
    VideoFormat v;

    bool ret = v.fromString(new_format);

    if (ret)
    {
        return g.set_video_format(v);
    }
    else
    {
        std::cout << "Invalid string description!" << std::endl;
    }
    return false;
}
