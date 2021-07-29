/*
 * Copyright 2019 The Imaging Source Europe GmbH
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

#include <gst/gst.h>
#include "../../src/gstreamer-1.0/tcamgstbase/tcamgstbase.h"

/**
 * This test aimes to verify that multiple sources are correctly identified
 * when present in the same pipeline.
 */
TEST_CASE("tcam_gst_find_camera_src")
{
    GError* err = nullptr;

    const char* pipeline_str = "tcamsrc name=test-source1 "
                               "! bayer2rgb "
                               "! videoconvert name=test-conv1 "
                               "! videomixer name=mix ! videoconvert ! fakesink "
                               " tcamsrc name=test-source2 "
                               "! bayer2rgb name=test-b2r2 "
                               "! videoconvert ! mix.";

    GstElement* pipeline = gst_parse_launch(pipeline_str, &err);

    gst_element_set_state(pipeline, GST_STATE_READY);

    GstElement* conv1 = gst_bin_get_by_name(GST_BIN(pipeline), "test-conv1");
    GstElement* bayer2rgb2 = gst_bin_get_by_name(GST_BIN(pipeline), "test-b2r2");

    GstElement* source1 = gst_bin_get_by_name(GST_BIN(pipeline), "test-source1");
    GstElement* source2 = gst_bin_get_by_name(GST_BIN(pipeline), "test-source2");

    REQUIRE(conv1 != nullptr);
    REQUIRE(bayer2rgb2 != nullptr);
    REQUIRE(source1 != nullptr);
    REQUIRE(source2 != nullptr);

    GstElement* res_source1 = tcam::gst::tcam_gst_find_camera_src(conv1);
    GstElement* res_source2 = tcam::gst::tcam_gst_find_camera_src(bayer2rgb2);

    REQUIRE(source1 == res_source1);
    REQUIRE(source2 == res_source2);

    gst_element_set_state(pipeline, GST_STATE_NULL);
}
