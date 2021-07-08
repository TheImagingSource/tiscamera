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

#include "find_input_caps_test_data.h"

#include "caps/caps.h"


std::vector<fic_test_data_container> fic_test_data = {};


void add_test(const std::string& name,
              const std::string& input_caps,
              const std::string& sink_caps,
              struct tcam::gst::input_caps_toggles toggles,
              const std::string& expected_caps,
              const tcam::gst::input_caps_required_modules& expected_modules)
{
    struct fic_test_result res = {};
    res.output_caps = expected_caps;
    res.modules = expected_modules;
    fic_test_data.push_back({ name, input_caps, sink_caps, toggles, res });
}


void init_dutils_test_data()
{

    //     .name = "yuv dutils handling",
    //     .input_caps = "video/x-raw,format=YUY2,width=4128,height=3096,framerate={1/1};"
    //     "video/x-raw,format=YUY2,width=3264,height=2448,framerate={1/1};"
    //     "video/x-raw,format=YUY2,width=2592,height=1944,framerate={1/1};"
    //     "video/x-raw,format=YUY2,width=1920,height=1080,framerate={30/1,25/1,20/1,15/1,10/1,5/1};"
    //     "video/x-raw,format=YUY2,width=1600,height=1200,framerate={30/1,25/1,20/1,15/1,10/1,5/1};"
    //     "video/x-raw,format=YUY2,width=1280,height=960,framerate={30/1,25/1,20/1,15/1,10/1,5/1};"
    //     "video/x-raw,format=YUY2,width=1280,height=720,framerate={30/1,25/1,20/1,15/1,10/1,5/1};"
    //     "video/x-raw,format=YUY2,width=800,height=480,framerate={30/1,25/1,20/1,15/1,10/1,5/1};"
    //     "video/x-raw,format=YUY2,width=640,height=480,framerate={30/1,25/1,20/1,15/1,10/1,5/1}",
    //     .sink_caps = "video/x-raw,format=BGRx",
    //     .use_dutils = true,
    //     {
    //         .output_caps = ,
    //         .requires_bayer = false,
    //         .requires_videoconvert = true,
    //         .requires_jpegdec = false,
    //         .requires_biteater = false,
    //         .requires_dutils = false,
    //     }
}


void init_test_data(bool use_pimipi, bool use_dutlis)
{
    struct tcam::gst::input_caps_toggles toggles = {};
    struct tcam::gst::input_caps_required_modules modules = {};

    if (use_dutlis)
    {
        init_dutils_test_data();
    }

    if (use_pimipi)
    {
        return;
    }

    // request is for bayer only
    // input caps contain bayer & GRAY8
    add_test("pass-trough bayer",
             caps::DFK72_CAPS,
             "video/x-bayer",
             toggles,
             "video/x-bayer,format=grbg,width=2592,height=1944,framerate={15/2,7/1,5/1,4/1};"
             "video/x-bayer,format=grbg,width=1920,height=1080,framerate={15/1,10/1,15/2,5/1};"
             "video/x-bayer,format=grbg,width=1456,height=1944,framerate={15/1,10/1,15/2,5/1};"
             "video/x-bayer,format=grbg,width=1120,height=1496,framerate={15/1,10/1,15/2,5/1};"
             "video/x-bayer,format=grbg,width=1280,height=960,framerate={30/1,25/1,15/1,10/1};"
             "video/x-bayer,format=grbg,width=1280,height=720,framerate={30/1,25/1,15/1,10/1};"
             "video/x-bayer,format=grbg,width=640,height=480,framerate={60/1,30/1,25/1,15/1}",
             modules);

    // add_test("Empty sink caps",
    //          caps::DFK72_CAPS,
    //          "",
    //          toggles,
    //          caps::DFK72_CAPS,
    //          modules
    //     );

    modules.bayer2rgb = true;

    // request is any kind of raw
    // since bayer is better than mono
    // return bayer
    add_test("Prefer raw",
             caps::DFK72_CAPS,
             "video/x-raw",
             toggles,
             "video/x-bayer,format=grbg,width=2592,height=1944,framerate={15/2,7/1,5/1,4/1};"
             "video/x-bayer,format=grbg,width=1920,height=1080,framerate={15/1,10/1,15/2,5/1};"
             "video/x-bayer,format=grbg,width=1456,height=1944,framerate={15/1,10/1,15/2,5/1};"
             "video/x-bayer,format=grbg,width=1120,height=1496,framerate={15/1,10/1,15/2,5/1};"
             "video/x-bayer,format=grbg,width=1280,height=960,framerate={30/1,25/1,15/1,10/1};"
             "video/x-bayer,format=grbg,width=1280,height=720,framerate={30/1,25/1,15/1,10/1};"
             "video/x-bayer,format=grbg,width=640,height=480,framerate={60/1,30/1,25/1,15/1}",
             modules);

    reset_input_caps_modules(modules);
    modules.bayer2rgb = true;
    add_test("Conversion 1",
             caps::DFK72_CAPS,
             "video/x-raw,format=BGRx",
             toggles,
             "video/x-bayer,format=grbg,width=2592,height=1944,framerate={15/2,7/1,5/1,4/1};"
             "video/x-bayer,format=grbg,width=1920,height=1080,framerate={15/1,10/1,15/2,5/1};"
             "video/x-bayer,format=grbg,width=1456,height=1944,framerate={15/1,10/1,15/2,5/1};"
             "video/x-bayer,format=grbg,width=1120,height=1496,framerate={15/1,10/1,15/2,5/1};"
             "video/x-bayer,format=grbg,width=1280,height=960,framerate={30/1,25/1,15/1,10/1};"
             "video/x-bayer,format=grbg,width=1280,height=720,framerate={30/1,25/1,15/1,10/1};"
             "video/x-bayer,format=grbg,width=640,height=480,framerate={60/1,30/1,25/1,15/1}",
             modules);


    reset_input_caps_modules(modules);

    add_test("Pass jpeg",
             caps::ECU010_CAPS,
             "image/jpeg",
             toggles,
             "image/jpeg,width=640,height=480,framerate=30/1; "
             "image/jpeg,width=1280,height=720,framerate=30/1; "
             "image/jpeg,width=800,height=600,framerate=30/1; "
             "image/jpeg,width=352,height=288,framerate=30/1; "
             "image/jpeg,width=320,height=240,framerate=30/1",
             modules);

    reset_input_caps_modules(modules);
    modules.videoconvert = true;
    add_test("Select yuv selection for BGRx",
             caps::ECU010_CAPS,
             "video/x-raw,format=BGRx",
             toggles,
             "video/x-raw,format=YUY2,width=1280,height=720,framerate={ 9/1 }; "
             "video/x-raw,format=YUY2,width=640,height=480,framerate={ 30/1 }; "
             "video/x-raw,format=YUY2,width=800,height=600,framerate={ 20/1 }; "
             "video/x-raw,format=YUY2,width=352,height=288,framerate={ 30/1 }; "
             "video/x-raw,format=YUY2,width=320,height=240,framerate={ 30/1 }",
             modules);


    reset_input_caps_modules(modules);
    modules.videoconvert = true;
    add_test("pass through YUY2",
             caps::ECU010_CAPS,
             "video/x-raw,format=BGRx,width=1280,height=720,framerate=9/1",
             toggles,
             "video/x-raw, format=YUY2, width=1280, height=720, framerate=9/1",
             modules);
    //         }
    //     },

    reset_input_caps_modules(modules);
    modules.videoconvert = true;
    modules.jpegdec = true;
    add_test("convert jpeg to BGRx",
             caps::AFU050_CAPS,
             "video/x-raw,format=BGRx,width=1280,height=960,framerate=60/1",
             toggles,
             "image/jpeg, width=1280, height=960, framerate=60/1",
             modules);

    reset_input_caps_modules(modules);
    add_test(
        "Mono and mono8 wanted",
        "video/x-raw,format=GRAY8,width=1600,height=1200,framerate={201, 15/1, 15/2, 15/4};"
        "video/x-raw,format=GRAY16_LE,width=1600,height=1200,framerate={20/1, 15/1, 15/2, 15/4};",
        "video/x-raw,format=GRAY8,width=1600,height=1200,framerate=15/1",
        toggles,
        "video/x-raw,format=GRAY8,width=1600,height=1200,framerate=15/1",
        modules);

    add_test("GRAY8 wanted from format list",
             "video/x-raw,format={GRAY8, GRAY16_LE},width=1600,height=1200,framerate={20/1, 15/1, "
             "15/2, 15/4};",
             "video/x-raw,format=GRAY8,width=1600,height=1200,framerate=15/1",
             toggles,
             "video/x-raw,format=GRAY8,width=1600,height=1200,framerate=15/1",
             modules);

    add_test("GRAY16 wanted from format list",
             "video/x-raw,format={GRAY8, GRAY16_LE},width=1600,height=1200,framerate={20/1, 15/1, "
             "15/2, 15/4};",
             "video/x-raw,format=GRAY16_LE,width=1600,height=1200,framerate=15/1",
             toggles,
             "video/x-raw,format=GRAY16_LE,width=1600,height=1200,framerate=15/1",
             modules);

    modules.videoconvert = true;
    add_test("Mono wanted for BGRx out",
             "video/x-raw,format={GRAY8, GRAY16_LE},width=1600,height=1200,framerate={20/1, 15/1, "
             "15/2, 15/4};",
             "video/x-raw,format=BGRx",
             toggles,
             "video/x-raw,format={GRAY8, GRAY16_LE},width=1600,height=1200,framerate={20/1, 15/1, "
             "15/2, 15/4};",

             modules);

    reset_input_caps_modules(modules);
    add_test("Select yuv",
             caps::ECU010_CAPS,
             "video/x-raw",
             toggles,
             "video/x-raw, format=YUY2, width=1280, height=720, framerate={ 9/1 }; "
             "video/x-raw, format=YUY2, width=640, height=480, framerate={ 30/1 }; "
             "video/x-raw, format=YUY2, width=800, height=600, framerate={ 20/1 }; "
             "video/x-raw, format=YUY2, width=352, height=288, framerate={ 30/1 }; "
             "video/x-raw, format=YUY2, width=320, height=240, framerate={ 30/1 }",
             modules);

    reset_input_caps_modules(modules);
    modules.videoconvert = true;
    add_test("YUV input; BGRx output",
             caps::ECU010_CAPS,
             "video/x-raw, format=BGRx",
             toggles,
             "video/x-raw, format=YUY2, width=1280, height=720, framerate={ 9/1 }; "
             "video/x-raw, format=YUY2, width=640, height=480, framerate={ 30/1 }; "
             "video/x-raw, format=YUY2, width=800, height=600, framerate={ 20/1 }; "
             "video/x-raw, format=YUY2, width=352, height=288, framerate={ 30/1 }; "
             "video/x-raw, format=YUY2, width=320, height=240, framerate={ 30/1 }",
             modules);
}


std::vector<fic_test_data_container> get_test_data()
{
    return fic_test_data;
}
