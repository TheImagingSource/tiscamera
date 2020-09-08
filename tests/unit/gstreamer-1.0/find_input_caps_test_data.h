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

#ifndef TCAM_TEST_FIND_INPUT_CAPS_DATA_H
#define TCAM_TEST_FIND_INPUT_CAPS_DATA_H

#include <string>
#include <vector>

#include "caps/caps.h"

struct fic_test_result
{
    const char *output_caps;
    bool requires_bayer;
    bool requires_videoconvert;
    bool requires_jpegdec;
    bool requires_dutils;
};


struct fic_test_data_container
{
    const char *name;
    const char *input_caps;
    const char *sink_caps;
    bool use_dutils;
    fic_test_result result;

    fic_test_data_container (  const char *name_,
                               const char *input_caps_,
                               const char *sink_caps_,
                               bool use_dutils_,
                               fic_test_result result_)
        : name(name_),
          input_caps(input_caps_),
          sink_caps(sink_caps_),
          use_dutils(use_dutils_),
          result(result_)
    {}
};



std::vector<fic_test_data_container> fic_test_data = {
    // {
    //     .name = "Empty input",
    //     .input_caps = nullptr,
    //     .sink_caps = "iamge/jpeg",
    //     .use_dutils = false,
    //     { nullptr,
    //       false, false, false, false, false
    //     }
    // },
    // video/x-raw, format=GRAY8, width=744, height=480,
    // framerate={ 20000/263, 60/1, 30/1, 25/1, 15/1, 15/2, 5/1 };
    // video/x-raw, format=GRAY8, width=640, height=480,
    // framerate={ 87/1, 60/1, 30/1, 25/1, 15/1, 15/2, 5/1 };
    // video/x-raw, format=GRAY8, width=320, height=240,
    // framerate={ 5000/31, 120/1, 80/1, 60/1, 30/1, 25/1, 15/1 };
    {
        "pass-trough bayer",
        caps::DFK72_CAPS,
        "video/x-bayer",
        false,
        {
            "video/x-bayer,format=grbg,width=2592,height=1944,framerate={15/2,7/1,5/1,4/1};"
            "video/x-bayer,format=grbg,width=1920,height=1080,framerate={15/1,10/1,15/2,5/1};"
            "video/x-bayer,format=grbg,width=1456,height=1944,framerate={15/1,10/1,15/2,5/1};"
            "video/x-bayer,format=grbg,width=1120,height=1496,framerate={15/1,10/1,15/2,5/1};"
            "video/x-bayer,format=grbg,width=1280,height=960,framerate={30/1,25/1,15/1,10/1};"
            "video/x-bayer,format=grbg,width=1280,height=720,framerate={30/1,25/1,15/1,10/1};"
            "video/x-bayer,format=grbg,width=640,height=480,framerate={60/1,30/1,25/1,15/1}",
            false,
            false,
            false,
            false
        }
    },

    {
        "Use jpeg only",
        caps::ECU010_CAPS,
        "image/jpeg",
        false,
        {
            "image/jpeg,width=640,height=480,framerate=30/1; "
            "image/jpeg,width=1280,height=720,framerate=30/1; "
            "image/jpeg,width=800,height=600,framerate=30/1; "
            "image/jpeg,width=352,height=288,framerate=30/1; "
            "image/jpeg,width=320,height=240,framerate=30/1",
            false,
            false,
            false,
            false
        }
    },
    {
        "Empty sink caps",
        caps::DFK72_CAPS,
        "",
        false,
        {
            caps::DFK72_CAPS,
            false,
            false,
            false,
            false,
        }
    },

    {
        "Mono and mono8 wanted",
        "video/x-raw,format=GRAY8,width=1600,height=1200,framerate={201, 15/1, 15/2, 15/4};"
        "video/x-raw,format=GRAY16_LE,width=1600,height=1200,framerate={20/1, 15/1, 15/2, 15/4};"
        "ANY",
        "video/x-raw,format=GRAY8,width=1600,height=1200,framerate=15/1",
        false,
        {
            "video/x-raw,format=GRAY8,width=1600,height=1200,framerate=15/1",
            false,
            false,
            false,
            false,
        }
    },

    {
        "Mono and mono8 wanted 2",
        "video/x-raw,format={GRAY8, GRAY16_LE},width=1600,height=1200,framerate={20/1, 15/1, 15/2, 15/4};"
        "ANY",
        "video/x-raw,format=GRAY8,width=1600,height=1200,framerate=15/1",
        false,
        {
            "video/x-raw,format=GRAY8,width=1600,height=1200,framerate=15/1",
            false,
            false,
            false,
            false,
        }
    },

    {
        "Conversion 1",
        caps::DFK72_CAPS,
        "video/x-raw,format=BGRx",
        false,
        {
            "video/x-bayer,format=grbg,width=2592,height=1944,framerate={15/2,7/1,5/1,4/1};"
            "video/x-bayer,format=grbg,width=1920,height=1080,framerate={15/1,10/1,15/2,5/1};"
            "video/x-bayer,format=grbg,width=1456,height=1944,framerate={15/1,10/1,15/2,5/1};"
            "video/x-bayer,format=grbg,width=1120,height=1496,framerate={15/1,10/1,15/2,5/1};"
            "video/x-bayer,format=grbg,width=1280,height=960,framerate={30/1,25/1,15/1,10/1};"
            "video/x-bayer,format=grbg,width=1280,height=720,framerate={30/1,25/1,15/1,10/1};"
            "video/x-bayer,format=grbg,width=640,height=480,framerate={60/1,30/1,25/1,15/1}",
            true,
            false,
            false,
            false,
        }
    },

    {
        "Conversion 2",
        "video/x-bayer, format=rggb, width=256, height=16, "
        "framerate={5/1,6/1,7/1,8/1,9/1, 10/1, 11/1, 12/1, "
        "13/1,14/1,15/1,16/1,17/1,18/1,19/1,20/1,30/1,40/1,50/1, "
        "60/1, 70/1, 80/1, 90/1, 100/1, 150/1, 200/1, 250/1, 300/1, 350/1, "
        "400/1, 450/1, 500/1, 550/1, 600/1, 650/1, 700/1, 750/1, 800/1, "
        "850/1, 900/1, 950/1, 1000/1, 1100/1, 1200/1, 1300/1, 1400/1, 1500/1, "
        "1600/1, 1700/1, 1800/1, 1900/1, 1168750/609 }; "
        "video/x-bayer, format=rggb, width=320, height=240, "
        "framerate={ 5/1, 6/1, 7/1, 8/1, 9/1, 10/1, 11/1, 12/1, "
        "13/1, 14/1, 15/1, 16/1, 17/1, 18/1, 19/1, 20/1, 30/1, 40/1, 50/1, "
        "60/1, 70/1, 80/1, 90/1, 100/1, 150/1, 200/1, 250/1, 300/1, 350/1, "
        "400/1, 450/1, 500/1, 550/1, 600/1, 650/1, 1168750/1723 }; "
        "video/x-bayer, format=rggb, width=360, height=280, "
        "framerate={ 5/1, 6/1, 7/1, 8/1, 9/1, 10/1, 11/1, 12/1, "
        "13/1, 14/1, 15/1, 16/1, 17/1, 18/1, 19/1, 20/1, 30/1, 40/1, 50/1, "
        "60/1, 70/1, 80/1, 90/1, 100/1, 150/1, 200/1, 250/1, 300/1, 350/1, "
        "400/1, 450/1, 500/1, 550/1, 584375/1004 }; "
        "video/x-bayer, format=rggb, width=544, height=480, "
        "framerate={ 5/1, 6/1, 7/1, 8/1, 9/1, 10/1, 11/1, 12/1, "
        "13/1, 14/1, 15/1, 16/1, 17/1, 18/1, 19/1, 20/1, 30/1, 40/1, 50/1, "
        "60/1, 70/1, 80/1, 90/1, 100/1, 150/1, 200/1, 250/1, 300/1, "
        "653125/1874 }; "
        "video/x-bayer, format=rggb, width=640, height=480, "
        "framerate={ 5/1, 6/1, 7/1, 8/1, 9/1, 10/1, 11/1, 12/1, "
        "13/1, 14/1, 15/1, 16/1, 17/1, 18/1, 19/1, 20/1, 30/1, 40/1, 50/1, "
        "60/1, 70/1, 80/1, 90/1, 100/1, 150/1, 200/1, 250/1, 300/1, "
        "1354375/4108 }; "
        "video/x-bayer, format=rggb, width=352, height=288, "
        "framerate={ 5/1, 6/1, 7/1, 8/1, 9/1, 10/1, 11/1, 12/1, "
        "13/1, 14/1, 15/1, 16/1, 17/1, 18/1, 19/1, 20/1, 30/1, 40/1, 50/1, "
        "60/1, 70/1, 80/1, 90/1, 100/1, 150/1, 200/1, 250/1, 300/1, 350/1, "
        "400/1, 450/1, 500/1, 550/1, 584375/1016 }; "
        "video/x-bayer, format=rggb, width=576, height=480, "
        "framerate={ 5/1, 6/1, 7/1, 8/1, 9/1, 10/1, 11/1, 12/1, "
        "13/1, 14/1, 15/1, 16/1, 17/1, 18/1, 19/1, 20/1, 30/1, 40/1, 50/1, "
        "60/1, 70/1, 80/1, 90/1, 100/1, 150/1, 200/1, 250/1, 300/1, "
        "653125/1934 }; "
        "video/x-bayer, format=rggb, width=720, height=480, "
        "framerate={ 5/1, 6/1, 7/1, 8/1, 9/1, 10/1, 11/1, 12/1, "
        "13/1, 14/1, 15/1, 16/1, 17/1, 18/1, 19/1, 20/1, 30/1, 40/1, 50/1, "
        "60/1, 70/1, 80/1, 90/1, 100/1, 150/1, 200/1, 250/1, 300/1, "
        "1354375/4408 }; "
        "video/x-bayer, format=rggb, width=960, height=720, "
        "framerate={ 5/1, 6/1, 7/1, 8/1, 9/1, 10/1, 11/1, 12/1, "
        "13/1, 14/1, 15/1, 16/1, 17/1, 18/1, 19/1, 20/1, 30/1, 40/1, 50/1, "
        "60/1, 70/1, 80/1, 90/1, 100/1, 150/1, 512/3 }; "
        "video/x-bayer, format=rggb, width=1280, "
        "height=720, framerate={ 5/1, 6/1, 7/1, 8/1, 9/1, "
        "10/1, 11/1, 12/1, 13/1, 14/1, 15/1, 16/1, 17/1, 18/1, 19/1, 20/1, "
        "30/1, 40/1, 50/1, 60/1, 70/1, 80/1, 90/1, 100/1, 128/1 }; "
        "video/x-bayer, format=rggb, width=1280, "
        "height=1024, framerate={ 5/1, 6/1, 7/1, 8/1, 9/1, "
        "10/1, 11/1, 12/1, 13/1, 14/1, 15/1, 16/1, 17/1, 18/1, 19/1, 20/1, "
        "30/1, 40/1, 50/1, 60/1, 70/1, 80/1, 90/1 }; "
        "video/x-bayer, format=rggb, width=[ 256, 1280 ], "
        "height=[ 16, 1024 ], framerate=[ 5/1, 2921875/486 ]; "
        "ANY",
        "video/x-raw, format=BGRA, width=1280, "
        "height=1024, framerate=6/1",
        false,
        {
            "video/x-bayer, format=rggb, width=1280, "
            "height=1024, framerate=6/1",
            true,
            false,
            false,
            false,
        }
    },

    {
        "Prefer raw",
        caps::DFK72_CAPS,
        "video/x-raw",
        false,
        {
            "video/x-bayer,format=grbg,width=2592,height=1944,framerate={15/2,7/1,5/1,4/1};"
            "video/x-bayer,format=grbg,width=1920,height=1080,framerate={15/1,10/1,15/2,5/1};"
            "video/x-bayer,format=grbg,width=1456,height=1944,framerate={15/1,10/1,15/2,5/1};"
            "video/x-bayer,format=grbg,width=1120,height=1496,framerate={15/1,10/1,15/2,5/1};"
            "video/x-bayer,format=grbg,width=1280,height=960,framerate={30/1,25/1,15/1,10/1};"
            "video/x-bayer,format=grbg,width=1280,height=720,framerate={30/1,25/1,15/1,10/1};"
            "video/x-bayer,format=grbg,width=640,height=480,framerate={60/1,30/1,25/1,15/1}",
            true,
            false,
            false,
            false,
        }
    },

    {
        "pass through bayer",

        caps::DFK72_CAPS,
        "video/x-bayer",
        false,
        {

            "video/x-bayer,format=grbg,width=2592,height=1944,framerate={15/2,7/1,5/1,4/1};"
            "video/x-bayer,format=grbg,width=1920,height=1080,framerate={15/1,10/1,15/2,5/1};"
            "video/x-bayer,format=grbg,width=1456,height=1944,framerate={15/1,10/1,15/2,5/1};"
            "video/x-bayer,format=grbg,width=1120,height=1496,framerate={15/1,10/1,15/2,5/1};"
            "video/x-bayer,format=grbg,width=1280,height=960,framerate={30/1,25/1,15/1,10/1};"
            "video/x-bayer,format=grbg,width=1280,height=720,framerate={30/1,25/1,15/1,10/1};"
            "video/x-bayer,format=grbg,width=640,height=480,framerate={60/1,30/1,25/1,15/1}",
            false,
            false,
            false,
            false,
        }
    },

    {
        "pass through YUY2",
        caps::ECU010_CAPS,
        "video/x-raw,format=BGRx,width=1280,height=720,framerate=9/1",
        false,
        {
            "video/x-raw, format=YUY2, width=1280, height=720, framerate=9/1",
            false,
            true,
            false,
            false,
        }
    },

    {
        "Select jpeg",
        caps::ECU010_CAPS,
        "image/jpeg",
        false,
        {
            "image/jpeg,width=640,height=480,framerate=30/1; "
            "image/jpeg,width=1280,height=720,framerate=30/1; "
            "image/jpeg,width=800,height=600,framerate=30/1; "
            "image/jpeg,width=352,height=288,framerate=30/1; "
            "image/jpeg,width=320,height=240,framerate=30/1",
            false,
            false,
            false,
            false,
        }
    },

    {
        "Select yuv",
        caps::ECU010_CAPS,
        "video/x-raw",
        false,
        {
            "video/x-raw, format=YUY2, width=1280, height=720, framerate={ 9/1 }; "
            "video/x-raw, format=YUY2, width=640, height=480, framerate={ 30/1 }; "
            "video/x-raw, format=YUY2, width=800, height=600, framerate={ 20/1 }; "
            "video/x-raw, format=YUY2, width=352, height=288, framerate={ 30/1 }; "
            "video/x-raw, format=YUY2, width=320, height=240, framerate={ 30/1 }",
            false,
            false,
            false,
            false,
        }
    },

    {
        "Select specific jpeg",
        caps::AFU050_CAPS,
        "image/jpeg, width=1280, height=960, framerate=60/1",
        false,
        {
            "image/jpeg, width=1280, height=960, framerate=60/1",
            false,
            false,
            false,
            false,
        }
    },

    {
        "convert jpeg to BGRx",
        caps::AFU050_CAPS,
        "video/x-raw,format=BGRx,width=1280,height=960,framerate=60/1",
        false,
        {
            "image/jpeg, width=1280, height=960, framerate=60/1",
            false,
            true,
            true,
            false,
        }
    },
    {
        "convert jpeg to BGRx selection",
        caps::AFU050_CAPS,
        "video/x-raw,format=BGRx",
        false,
        {

            "image/jpeg, width=2592, height=1944, framerate={ 15/1 };"
            "image/jpeg, width=1920, height=1080, framerate={ 30/1 };"
            "image/jpeg, width=1280, height=960, framerate={ 60/1 };",
            false,
            true,
            true,
            false,
        }
    },

    {
        "Select yuv selection for BGRx",
        caps::ECU010_CAPS,
        "video/x-raw,format=BGRx",
        false,
        {
            "video/x-raw,format=YUY2,width=1280,height=720,framerate={ 9/1 }; "
            "video/x-raw,format=YUY2,width=640,height=480,framerate={ 30/1 }; "
            "video/x-raw,format=YUY2,width=800,height=600,framerate={ 20/1 }; "
            "video/x-raw,format=YUY2,width=352,height=288,framerate={ 30/1 }; "
            "video/x-raw,format=YUY2,width=320,height=240,framerate={ 30/1 }",
            false,
            true,
            false,
            false,
        }
    },

    // {
    //     // afu130
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
    // },
    // {
    //     .name = ,
    //     .input_caps = ,
    //     .sink_caps = ,
    //     .use_dutils = ,
    //     {
    //         .output_caps = ,
    //         .requires_bayer = ,
    //         .requires_videoconvert = ,
    //         .requires_jpegdec = ,
    //         .requires_biteater = ,
    //         .requires_dutils = ,
    //     }
    // },

};

#endif /* TCAM_TEST_FIND_INPUT_CAPS_DATA_H */
