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

#include "tcambind.h"

#include "../../logging.h"
#include "../../public_utils.h"
#include "../tcamgstbase/tcamgststrings.h"

#include <dutils_img/fcc_to_string.h>

std::pair<std::string, std::string> tcambind::separate_serial_and_type(const std::string& input)
{
    auto pos = input.find("-");

    if (pos != std::string::npos)
    {
        return std::make_pair(input.substr(0, pos), input.substr(pos + 1));
    }
    return std::make_pair(input, std::string {});
}

static void fill_structure_fixed_resolution(tcam::CaptureDevice& device,
                                            GstStructure* structure,
                                            const tcam::VideoFormatDescription& format,
                                            tcam::tcam_image_size max_size,
                                            const tcam::image_scaling& scaling)
{

    auto framerate_info_res =
        device.get_framerate_info(tcam::VideoFormat { format.get_fourcc(), max_size });
    if (framerate_info_res.has_error())
    {
        return;
    }

    GValue fps_list = G_VALUE_INIT;
    g_value_init(&fps_list, GST_TYPE_LIST);

    for (auto rate : framerate_info_res.value().to_list())
    {
        int frame_rate_numerator = 0;
        int frame_rate_denominator = 0;
        gst_util_double_to_fraction(rate, &frame_rate_numerator, &frame_rate_denominator);

        GValue fraction = G_VALUE_INIT;
        g_value_init(&fraction, GST_TYPE_FRACTION);
        gst_value_set_fraction(&fraction, frame_rate_numerator, frame_rate_denominator);
        gst_value_list_append_value(&fps_list, &fraction);
        g_value_unset(&fraction);
    }

    gst_structure_set(structure,
                      "width",
                      G_TYPE_INT,
                      max_size.width,
                      "height",
                      G_TYPE_INT,
                      max_size.height,
                      NULL);

    if (!scaling.is_default())
    {
        if (scaling.binning_h != 1 || scaling.binning_v != 1)
        {
            std::string binning =
                std::to_string(scaling.binning_h) + "x" + std::to_string(scaling.binning_v);

            gst_structure_set(structure, "binning", G_TYPE_STRING, binning.c_str(), nullptr);
        }
        if (scaling.skipping_h != 1 || scaling.skipping_v != 1)
        {
            std::string skipping = std::to_string(scaling.skipping_h) + "x"
                                   + std::to_string(scaling.skipping_v);

            gst_structure_set(structure, "skipping", G_TYPE_STRING, skipping.c_str(), nullptr);
        }
    }

    gst_structure_take_value(structure, "framerate", &fps_list);
}

static void append_gst_structs_for_res_type_range(
    tcam::CaptureDevice& device,
    GstCaps* caps,
    const tcam::VideoFormatDescription& desc,
    const char* caps_string,
    const tcam::tcam_resolution_description& reso_desc)
{
    auto resolution_list = tcam::get_standard_resolutions(reso_desc.min_size, reso_desc.max_size);

    // check if min/max are already in the vector.
    // some devices return std resolutions as max
    if (reso_desc.min_size != resolution_list.front())
    {
        resolution_list.insert(resolution_list.begin(), reso_desc.min_size);
    }

    if (reso_desc.max_size != resolution_list.back())
    {
        resolution_list.push_back(reso_desc.max_size);
    }

    // small hack to keep lists short
    // basically only list explicit resolutions
    // when binning/skipping are off
    // the rest is handled via the range description
    if (!reso_desc.scaling.is_default())
    {
        resolution_list.clear();
    }

    for (const auto& fixed_resolution : resolution_list)
    {
        std::vector<double> framerates;
        if (auto framerate_info_res = device.get_framerate_info(
                tcam::VideoFormat { desc.get_fourcc(), fixed_resolution });
            framerate_info_res.has_error())
        {
            continue;
        }
        else
        {
            framerates = framerate_info_res.value().to_list();
        }

        GstStructure* structure = gst_structure_from_string(caps_string, NULL);

        GValue fps_list = G_VALUE_INIT;
        g_value_init(&fps_list, GST_TYPE_LIST);

        for (const auto& f : framerates)
        {
            int frame_rate_numerator = 0;
            int frame_rate_denominator = 0;
            gst_util_double_to_fraction(f, &frame_rate_numerator, &frame_rate_denominator);

            if ((frame_rate_denominator == 0) || (frame_rate_numerator == 0))
            {
                continue;
            }

            GValue fraction = G_VALUE_INIT;
            g_value_init(&fraction, GST_TYPE_FRACTION);
            gst_value_set_fraction(&fraction, frame_rate_numerator, frame_rate_denominator);
            gst_value_list_append_value(&fps_list, &fraction);
            g_value_unset(&fraction);
        }

        gst_structure_set(structure,
                          "width",
                          G_TYPE_INT,
                          fixed_resolution.width,
                          "height",
                          G_TYPE_INT,
                          fixed_resolution.height,
                          NULL);

        gst_structure_take_value(structure, "framerate", &fps_list);
        gst_caps_append_structure(caps, structure);
    }

    // finally also add the range to allow unusual settings like 1920x96@90fps

    // build binning/skipping information
    std::string binning;
    std::string skipping;
    if (!reso_desc.scaling.is_default())
    {
        if (reso_desc.scaling.binning_h != 1 || reso_desc.scaling.binning_v != 1)
        {
            binning = std::to_string(reso_desc.scaling.binning_h) + "x"
                      + std::to_string(reso_desc.scaling.binning_v);
        }
        if (reso_desc.scaling.skipping_h != 1 || reso_desc.scaling.skipping_v != 1)
        {
            skipping = std::to_string(reso_desc.scaling.skipping_h) + "x"
                       + std::to_string(reso_desc.scaling.skipping_v);
        }
    }

    double min_fps = 0.;
    double max_fps = 0.;
    if (auto framerates_res_for_min_dim =
            device.get_framerate_info(tcam::VideoFormat { desc.get_fourcc(), reso_desc.min_size });
        framerates_res_for_min_dim.has_error())
    {
        return;
    }
    else
    {
        min_fps = framerates_res_for_min_dim.value().min();
        max_fps = framerates_res_for_min_dim.value().max();
    }
    if (auto framerates_res_for_max_dim =
            device.get_framerate_info(tcam::VideoFormat { desc.get_fourcc(), reso_desc.max_size });
        framerates_res_for_max_dim.has_error())
    {
        return;
    }
    else
    {
        min_fps = std::min(min_fps, framerates_res_for_max_dim.value().min());
        max_fps = std::max(max_fps, framerates_res_for_max_dim.value().max());
    }

    int fps_min_num = 0;
    int fps_min_den = 0;
    int fps_max_num = 0;
    int fps_max_den = 0;
    gst_util_double_to_fraction(min_fps, &fps_min_num, &fps_min_den);
    gst_util_double_to_fraction(max_fps, &fps_max_num, &fps_max_den);

    GValue f = G_VALUE_INIT;
    g_value_init(&f, GST_TYPE_FRACTION_RANGE);

    gst_value_set_fraction_range_full(&f, fps_min_num, fps_min_den, fps_max_num, fps_max_den);

    GValue w = G_VALUE_INIT;
    if (reso_desc.min_size.width < reso_desc.max_size.width)
    {
        g_value_init(&w, GST_TYPE_INT_RANGE);
        gst_value_set_int_range_step(
            &w, reso_desc.min_size.width, reso_desc.max_size.width, reso_desc.width_step_size);
    }
    else
    {
        g_value_init(&w, G_TYPE_INT);
        g_value_set_int(&w, reso_desc.min_size.width);
    }

    GValue h = G_VALUE_INIT;

    if (reso_desc.min_size.height < reso_desc.max_size.height)
    {
        g_value_init(&h, GST_TYPE_INT_RANGE);
        gst_value_set_int_range_step(
            &h, reso_desc.min_size.height, reso_desc.max_size.height, reso_desc.height_step_size);
    }
    else
    {
        g_value_init(&h, G_TYPE_INT);
        g_value_set_int(&h, reso_desc.min_size.height);
    }

    // Create the actual GstStructure for this range format

    GstStructure* structure = gst_structure_from_string(caps_string, NULL);

    if (!binning.empty())
        gst_structure_set(structure, "binning", G_TYPE_STRING, binning.c_str(), nullptr);
    if (!skipping.empty())
        gst_structure_set(structure, "skipping", G_TYPE_STRING, skipping.c_str(), nullptr);

    gst_structure_take_value(structure, "width", &w);
    gst_structure_take_value(structure, "height", &h);
    gst_structure_take_value(structure, "framerate", &f);

    gst_caps_append_structure(caps, structure); // add the structure to the caps
}

static bool is_totally_ordered_less(const tcam::image_scaling& lhs, const tcam::image_scaling& rhs)
{
    if (lhs.binning_h == rhs.binning_h)
    {
        if (lhs.binning_v == rhs.binning_v)
        {
            if (lhs.skipping_h == rhs.skipping_h)
            {
                return lhs.skipping_v < rhs.skipping_v;
            }
            return lhs.skipping_h < rhs.skipping_h;
        }
        return lhs.binning_v < rhs.binning_v;
    }
    return lhs.binning_h < rhs.binning_h;
}


static bool is_totally_ordered_less(const tcam::tcam_image_size& lhs, const tcam::tcam_image_size& rhs)
{
    if (lhs.width == rhs.width)
    {
        return lhs.height < rhs.height;
    }
    return lhs.width < rhs.width;
}

gst_helper::gst_ptr<GstCaps> tcambind::convert_videoformatsdescription_to_caps(
    tcam::CaptureDevice& device,
    const std::vector<tcam::VideoFormatDescription>& descriptions)
{
    GstCaps* caps = gst_caps_new_empty();

    for (const auto& desc : descriptions)
    {
        if (desc.get_fourcc() == 0)
        {
            SPDLOG_INFO("Format has empty fourcc. Format-desc='{}'",
                        desc.get_video_format_description_string());
            continue;
        }

        const auto caps_string = tcam::gst::tcam_fourcc_to_gst_1_0_caps_string(desc.get_fourcc());
        if (caps_string.empty())
        {
            SPDLOG_INFO("Format has empty caps string. Ignoring {}",
                        img::fcc_to_string(desc.get_fourcc()));
            continue;
        }

        auto res = desc.get_resolutions();

        std::sort(res.begin(),
                  res.end(),
                  [](const tcam::tcam_resolution_description& lhs,
                     const tcam::tcam_resolution_description& rhs)
                  {
                      if (lhs.scaling == rhs.scaling)
                      {
                          return is_totally_ordered_less(lhs.max_size,rhs.max_size);
                      }
                      if (lhs.scaling.is_default())
                          return true;
                      return is_totally_ordered_less(lhs.scaling, rhs.scaling);
                  });

        if (res.empty())
        {
            SPDLOG_INFO("Format has empty resolution list. Ignoring {}",
                        img::fcc_to_string(desc.get_fourcc()));
            continue;
        }

        for (const auto& r : res)
        {
            if (r.type == tcam::TCAM_RESOLUTION_TYPE_RANGE)
            {
                append_gst_structs_for_res_type_range(device, caps, desc, caps_string.c_str(), r);
            }
            else
            {
                GstStructure* structure = gst_structure_from_string(caps_string.c_str(), NULL);

                fill_structure_fixed_resolution(device, structure, desc, r.max_size, r.scaling);
                gst_caps_append_structure(caps, structure);
            }
        }
    }

    // never use gst_caps_simplify
    // it will remove all generated resolutions 
    // in favor of ranges
    // binnning/skipping will also be lost
    // caps = gst_caps_simplify(caps);

    return gst_helper::make_ptr(caps);
}
