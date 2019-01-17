/*
 * Copyright 2018 The Imaging Source Europe GmbH
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

#include "catch.hpp"
#include "tcamgstbase.h"
#include <regex>

#include <gst/gst.h>

#include "find_input_caps_test_data.h"

/**
 * This file contains all tests for the function 'find_input_caps'
 * in src/gstreamer-1.0/tcamgstbase.cpp
 */

TEST_CASE("find_input_caps")
{

    bool has_dutils;
    if (gst_element_factory_find("tcamdutils") != nullptr)
    {
        has_dutils = true;
    }
    else
    {
        WARN("No tcamdutils. Not all tests will be run");
        has_dutils = false;
    }

    for (unsigned int x = 0; x < fic_test_data.size(); x++)
    {
        auto &entry = fic_test_data.at(x);

        if (entry.use_dutils && !has_dutils)
        {
            continue;
        }

        SECTION(entry.name)
        {

            GstCaps* src_caps = nullptr;
            GstCaps* sink_caps = nullptr;
            GstCaps* expected_output = nullptr;

            if (entry.input_caps)
            {
                src_caps = gst_caps_from_string(entry.input_caps);
            }

            if (entry.sink_caps)
            {
                sink_caps = gst_caps_from_string(entry.sink_caps);
            }

            if (entry.result.output_caps)
            {
                expected_output = gst_caps_from_string(entry.result.output_caps);
            }

            bool requires_bayer;
            bool requires_videoconvert;
            bool requires_jpegdec;
            bool requires_dutils;
            bool requires_biteater;
            bool use_dutils = entry.use_dutils;
            // bool use_dutils = false;

            GstCaps* result_caps = find_input_caps(src_caps,
                                                   sink_caps,
                                                   requires_bayer,
                                                   requires_videoconvert,
                                                   requires_jpegdec, requires_dutils,
                                                   requires_biteater, use_dutils);

            if (result_caps)
            {
                std::regex e("; ");
                INFO("Result caps: "
                     << std::regex_replace(gst_caps_to_string(result_caps), e, ";\n")
                     << "\n\n");
                INFO("Expected caps: "
                     << std::regex_replace(gst_caps_to_string(expected_output), e, ";\n"));

                REQUIRE(gst_caps_is_equal(result_caps, expected_output));
            }
            else
            {
                REQUIRE(result_caps == expected_output);
            }

            REQUIRE(requires_bayer == entry.result.requires_bayer);
            REQUIRE(requires_videoconvert == entry.result.requires_videoconvert);
            REQUIRE(requires_jpegdec == entry.result.requires_jpegdec);
            REQUIRE(requires_dutils == entry.result.requires_dutils);
            REQUIRE(requires_biteater == entry.result.requires_biteater);

            if (src_caps)
            {
                gst_caps_unref(src_caps);
            }
            if (sink_caps)
            {
                gst_caps_unref(sink_caps);
            }
            if (expected_output)
            {
                gst_caps_unref(expected_output);
            }
            if (result_caps)
            {
                gst_caps_unref(result_caps);
            }
        }
    }
}
