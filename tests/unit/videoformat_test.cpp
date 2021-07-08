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
#include "tcam.h"

#include <dutils_img/image_fourcc.h>

TEST_CASE("VideoFormat Tests")
{
    struct tcam::tcam_video_format format_struct = { FOURCC_Y800, 0, 0, 640, 480, 30.0 };

    tcam::VideoFormat format(format_struct);

    REQUIRE(format.get_fourcc() == FOURCC_Y800);

    //struct tcam_image_size size1 = {640, 480};

    // REQUIRE(tcam::are_equal(format.get_size(), size1));
    REQUIRE(format.get_framerate() == 30.0);

    REQUIRE(format.get_pitch_size() == 640);

    SECTION("Size change results in different pitch/buffer size")
    {
        format.set_size(1280, 1024);

        REQUIRE(format.get_pitch_size() == 1280);
        REQUIRE(format.get_required_buffer_size() == 1310720);
    }

    SECTION("Format change results in different pitch/buffer size")
    {
        format.set_fourcc(FOURCC_MJPG);

        REQUIRE(format.get_pitch_size() == 1920);
        REQUIRE(format.get_required_buffer_size() == 921600);
    }

    SECTION("Size AND format change results in different pitch/buffer size")
    {
        format.set_size(1280, 1024);
        format.set_fourcc(FOURCC_BGRA32);

        REQUIRE(format.get_pitch_size() == 5120);
        REQUIRE(format.get_required_buffer_size() == 5242880);
    }

    // TODO reenable from string

    // SECTION("From string", "[VideoFormat::from_string]")
    // {
    //     tcam::VideoFormat tmp_format;

    //     tmp_format.from_string("format=Y800,width=640,height=480,framerate=30.000000");

    //     REQUIRE(format == tmp_format);
    // }

    SECTION("To string", "[VideoFormat::to_string]")
    {

        REQUIRE(format.to_string() == "format=Mono8,width=640,height=480,framerate=30.000000");
    }
}
