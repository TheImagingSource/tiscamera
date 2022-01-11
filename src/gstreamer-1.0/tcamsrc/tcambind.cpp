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
        return std::make_pair( input.substr( 0, pos ), input.substr( pos + 1 ) );
    }
    return std::make_pair(input, std::string {});
}

static void fill_structure_fixed_resolution(GstStructure* structure,
                                            const tcam::VideoFormatDescription& format,
                                            const tcam::tcam_resolution_description& res)
{
    GValue fps_list = G_VALUE_INIT;
    g_value_init(&fps_list, GST_TYPE_LIST);

    for (auto rate : format.get_frame_rates(res))
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
                      res.max_size.width,
                      "height",
                      G_TYPE_INT,
                      res.max_size.height,
                      NULL);

    if (!res.scaling.is_default())
    {
        if (res.scaling.binning_h != 1 || res.scaling.binning_v != 1)
        {
            std::string binning = std::to_string(res.scaling.binning_h) + "x" + std::to_string(res.scaling.binning_v);

            gst_structure_set(structure, "binning", G_TYPE_STRING, binning.c_str(), nullptr);
        }
        if (res.scaling.skipping_h != 1 || res.scaling.skipping_v != 1)
        {
            std::string skipping = std::to_string(res.scaling.skipping_h) + "x" + std::to_string(res.scaling.skipping_v);

            gst_structure_set(structure, "skipping", G_TYPE_STRING, skipping.c_str(), nullptr);
        }
    }

    gst_structure_take_value(structure, "framerate", &fps_list);
}


gst_helper::gst_ptr<GstCaps> tcambind::convert_videoformatsdescription_to_caps(
    const std::vector<tcam::VideoFormatDescription>& descriptions)
{
    GstCaps* caps = gst_caps_new_empty();

    for (const auto& desc : descriptions)
    {
        if (desc.get_fourcc() == 0)
        {
            SPDLOG_INFO("Format has empty fourcc. Format-desc='{}'", desc.get_struct().description);
            continue;
        }

        const char* caps_string = tcam::gst::tcam_fourcc_to_gst_1_0_caps_string(desc.get_fourcc());

        if (caps_string == nullptr)
        {
            SPDLOG_INFO("Format has empty caps string. Ignoring {}",
                        img::fcc_to_string(desc.get_fourcc()));
            continue;
        }

        auto res = desc.get_resolutions();

        for (const auto& r : res)
        {
            uint32_t min_width = r.min_size.width;
            uint32_t min_height = r.min_size.height;

            uint32_t max_width = r.max_size.width;
            uint32_t max_height = r.max_size.height;

            if (r.type == tcam::TCAM_RESOLUTION_TYPE_RANGE)
            {
                auto framesizes = tcam::get_standard_resolutions(r.min_size, r.max_size);

                // check if min/max are already in the vector.
                // some devices return std resolutions as max
                if (r.min_size != framesizes.front())
                {
                    framesizes.insert(framesizes.begin(), r.min_size);
                }

                if (r.max_size != framesizes.back())
                {
                    framesizes.push_back(r.max_size);
                }

                for (const auto& reso : framesizes)
                {
                    if (!r.scaling.is_default())
                    {
                        continue;
                    }

                    GstStructure* structure = gst_structure_from_string(caps_string, NULL);

                    std::vector<double> framerates = desc.get_framerates(reso);

                    if (framerates.empty())
                    {
                        continue;
                    }

                    GValue fps_list = G_VALUE_INIT;
                    g_value_init(&fps_list, GST_TYPE_LIST);

                    for (const auto& f : framerates)
                    {
                        int frame_rate_numerator;
                        int frame_rate_denominator;
                        gst_util_double_to_fraction(
                            f, &frame_rate_numerator, &frame_rate_denominator);

                        if ((frame_rate_denominator == 0) || (frame_rate_numerator == 0))
                        {
                            continue;
                        }

                        GValue fraction = G_VALUE_INIT;
                        g_value_init(&fraction, GST_TYPE_FRACTION);
                        gst_value_set_fraction(
                            &fraction, frame_rate_numerator, frame_rate_denominator);
                        gst_value_list_append_value(&fps_list, &fraction);
                        g_value_unset(&fraction);
                    }


                    gst_structure_set(structure,
                                      "width",
                                      G_TYPE_INT,
                                      reso.width,
                                      "height",
                                      G_TYPE_INT,
                                      reso.height,
                                      NULL);

                    gst_structure_take_value(structure, "framerate", &fps_list);
                    gst_caps_append_structure(caps, structure);
                }


                std::vector<double> fps = desc.get_frame_rates(r);

                std::vector<double> highest_fps = desc.get_framerates({ max_width, max_height });

                if (fps.empty())
                {
                    // GST_ERROR("Could not find any framerates for format");
                    continue;
                }

                // finally also add the range to allow unusual settings like 1920x96@90fps
                GstStructure* structure = gst_structure_from_string(caps_string, NULL);

                GValue w = G_VALUE_INIT;

                if (min_width < max_width)
                {
                    g_value_init(&w, GST_TYPE_INT_RANGE);
                    gst_value_set_int_range_step(&w, min_width, max_width, r.width_step_size);
                }
                else
                {
                    g_value_init(&w, G_TYPE_INT);
                    g_value_set_int(&w, min_width);
                }

                GValue h = G_VALUE_INIT;

                if (min_height < max_height)
                {
                    g_value_init(&h, GST_TYPE_INT_RANGE);
                    gst_value_set_int_range_step(&h, min_height, max_height, r.height_step_size);
                }
                else
                {
                    g_value_init(&h, G_TYPE_INT);
                    g_value_set_int(&h, min_height);
                }

                int fps_min_num;
                int fps_min_den;
                int fps_max_num;
                int fps_max_den;
                gst_util_double_to_fraction(
                    *std::min_element(fps.begin(), fps.end()), &fps_min_num, &fps_min_den);
                gst_util_double_to_fraction(
                    *std::max_element(highest_fps.begin(), highest_fps.end()),
                    &fps_max_num,
                    &fps_max_den);

                GValue f = G_VALUE_INIT;
                g_value_init(&f, GST_TYPE_FRACTION_RANGE);

                gst_value_set_fraction_range_full(
                    &f, fps_min_num, fps_min_den, fps_max_num, fps_max_den);

                if (!r.scaling.is_default())
                {
                    if (r.scaling.binning_h != 1 || r.scaling.binning_v != 1)
                    {
                        std::string binning = std::to_string(r.scaling.binning_h) + "x" + std::to_string(r.scaling.binning_v);

                        gst_structure_set(structure, "binning", G_TYPE_STRING, binning.c_str(), nullptr);
                    }
                    if (r.scaling.skipping_h != 1 || r.scaling.skipping_v != 1)
                    {
                        std::string skipping = std::to_string(r.scaling.skipping_h) + "x" + std::to_string(r.scaling.skipping_v);

                        gst_structure_set(structure, "skipping", G_TYPE_STRING, skipping.c_str(), nullptr);
                    }
                }

                gst_structure_take_value(structure, "width", &w);
                gst_structure_take_value(structure, "height", &h);
                gst_structure_take_value(structure, "framerate", &f);
                gst_caps_append_structure(caps, structure);
            }
            else
            {
                GstStructure* structure = gst_structure_from_string(caps_string, NULL);

                fill_structure_fixed_resolution(structure, desc, r);
                gst_caps_append_structure(caps, structure);
            }
        }
    }

    return gst_helper::make_ptr( caps );
}
