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

#include "tcamgststrings.h"
#include "tcamgstbase.h" // videoformatsdescription_to_gst_caps_string
#include <gst/gst.h> // gst_init
#include <iostream>
#include <iomanip>
#include <regex>


void list_formats (const std::vector<VideoFormatDescription>& available_formats)
{
    std::cout << "Available format settings:" << std::endl;
    for (const VideoFormatDescription& f : available_formats)
    {
        auto desc = f.get_struct();

        std::cout << "Format: " << desc.description << " - Fourcc(" << desc.fourcc << ")" << std::endl;

        for (const auto& s : f.get_resolutions())
        {
            if (s.type == TCAM_RESOLUTION_TYPE_RANGE)
            {
                std::cout << "\tResolutionrange: "
                          << s.min_size.width << "x" << s.min_size.height << " - "
                          << s.max_size.width << "x" << s.max_size.height << std::endl;
            }
            else
            {
                std::cout << "\tResolution: " << s.min_size.width << "x" << s.min_size.height << std::endl;
            }
            for (const auto& fps : f.get_frame_rates(s))
            {
                std::cout << "\t\t" << std::setw(8) << std::fixed << std::setprecision(4)<< fps << " fps" << std::endl;
            }
        }

        std::cout << std::endl;
    }
}


void list_gstreamer_1_0_formats (const std::string& serial)
{
    GstElement* source = gst_element_factory_make("tcamsrc", "source");

    if (!source)
    {
        std::cerr << "Unable to create source element." << std::endl;
        return;
    }

    std::string str;

    GValue val = {};
    g_value_init(&val, G_TYPE_STRING);
    g_value_set_static_string(&val, serial.c_str());

    g_object_set_property(G_OBJECT(source), "serial", &val);

    gst_element_set_state(source, GST_STATE_READY);

    GstPad* pad = gst_element_get_static_pad(source, "src");

    GstCaps* caps = gst_pad_query_caps(pad, NULL);

    str = gst_caps_to_string(caps);

    if (caps)
    {
        // use a regex to insert line breaks for increased readability
        std::regex e ("; ");
        std::regex wb_e (" ");
        std::regex paren_e ("\\((string|int|fraction)\\)");
        std::cout << "Available gstreamer-1.0 caps:" << std::endl;

        std::string caps_str = std::regex_replace(str, e, ";\n");
        caps_str = std::regex_replace(caps_str, wb_e, "");
        caps_str = std::regex_replace(caps_str, paren_e, "");

        std::cout << caps_str << std::endl;
    }
    else
    {
        std::cerr << "Unable to display caps. Conversion failed." << str << std::endl;
    }

    gst_caps_unref(caps);
    gst_object_unref(pad);

    gst_element_set_state(source, GST_STATE_NULL);

    gst_object_unref(source);


}


void print_active_format (const VideoFormat& format)
{
    std::cout << "Active format:\n"
              << "Format: \t" << fourcc_to_description(format.get_fourcc())
              << "\nResolution: \t" << format.get_size().width << "x" << format.get_size().height
              << "\nFramerate: \t" << format.get_framerate() << "\n" << std::endl;
}


bool set_active_format (std::shared_ptr<CaptureDevice> dev, const std::string& new_format)
{
    VideoFormat v;

    bool ret = v.from_string(new_format);

    if (ret)
    {
        return dev->set_video_format(v);
    }
    else
    {
        std::cout << "Invalid string description!" << std::endl;
    }
    return false;
}
