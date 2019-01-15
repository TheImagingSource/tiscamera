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

#ifndef TCAM_TEST_FIND_LARGEST_CAPS_DATA_H
#define TCAM_TEST_FIND_LARGEST_CAPS_DATA_H

#include <string>
#include <vector>

struct flc_test_data_container
{
    const char *name;
    const char *input_caps;
    const char *expected_output;

    flc_test_data_container (const char* name_,
                             const char* input_caps_,
                             const char* expected_output_)
        : name(name_), input_caps(input_caps_), expected_output(expected_output_)
    {}
};

std::vector<struct flc_test_data_container> flc_test_data = {
    {
        "DFK23 camera",
        "video/x-bayer, format=rggb, width=256, height=16, framerate={ "
        "5/1, 10/1, 15/1, 20/1, 30/1, 50/1, 100/1, 150/1, 200/1, 250/1, "
        "300/1, 350/1, 400/1, 450/1, 500/1, 550/1, 600/1, 650/1, 700/1, "
        "1000/1, 1100/1, 1200/1, 1300/1, 1400/1, 1500/1, 1600/1, 1700/1, "
        "1800/1, 1900/1, 1168750/609 }; "
        "video/x-bayer, format=rggb, width=320, height=240, framerate={ "
        "5/1, 10/1, 15/1, 20/1, 30/1, 50/1, 60/1, 70/1, 80/1, 90/1, 100/1, "
        "150/1, 200/1, 250/1, 300/1, 350/1, 400/1, 450/1, 500/1, 550/1, "
        "600/1, 650/1, 1168750/1723 }; "
        "video/x-bayer, format=rggb, width=360, height=280, framerate={ "
        "5/1, 10/1, 15/1, 20/1, 30/1, 50/1, 60/1, 70/1, 80/1, 90/1, 100/1, "
        "150/1, 200/1, 250/1, 300/1, 350/1, 400/1, 450/1, 500/1, 550/1, "
        "584375/1004 }; "
        "video/x-bayer, format=rggb, width=544, height=480, framerate={ "
        "5/1, 10/1, 15/1, 20/1, 30/1, 50/1, 60/1, 70/1, 80/1, 90/1, 100/1, "
        "150/1, 200/1, 250/1, 300/1, 653125/1874 }; "
        "video/x-bayer, format=rggb, width=640, height=480, framerate={ "
        "5/1, 10/1, 15/1, 20/1, 30/1, 50/1, 90/1, 100/1, 150/1, 200/1, "
        "250/1, 300/1, 1354375/4108 }; "
        "video/x-bayer,format=rggb,width=352,height=288,framerate={ 5/1, "
        "10/1, 15/1, 20/1, 30/1, 50/1, 100/1, 150/1, 200/1, 250/1, 300/1, "
        "350/1, 400/1, 450/1, 500/1, 550/1, 584375/1016 }; "
        "video/"
        "x-bayer,format=rggb,width=576,height=480,framerate=(fraction){ "
        "5/1, 200/1, 250/1, 300/1, 653125/1934 }; "
        "video/x-bayer,format=rggb,width=720,height=480,framerate={ 5/1, "
        "6/1, 7/1, 8/1, 9/1, 10/1, 11/1, 12/1, 13/1, 14/1, 15/1, 16/1, "
        "17/1, 18/1, 19/1, 20/1, 30/1, 40/1, 50/1, 60/1, 70/1, 80/1, 90/1, "
        "100/1, 150/1, 200/1, 250/1, 300/1, 1354375/4408 }; "
        "video/x-bayer,format=rggb,width=960,height=720,framerate={ 5/1, "
        "6/1, 7/1, 8/1, 9/1, 10/1, 11/1, 12/1, 13/1, 14/1, 15/1, 16/1, "
        "17/1, 18/1, 19/1, 20/1, 30/1, 40/1, 50/1, 60/1, 70/1, 80/1, 90/1, "
        "100/1, 150/1, 512/3 }; "
        "video/x-bayer,format=rggb,width=1280,height=720,framerate={ 5/1, "
        "6/1, 7/1, 8/1, 9/1, 10/1, 11/1, 12/1, 13/1, 14/1, 15/1, 16/1, "
        "17/1, 18/1, 19/1, 20/1, 30/1, 40/1, 50/1, 60/1, 70/1, 80/1, 90/1, "
        "100/1, 128/1 }; "
        "video/x-bayer,format=rggb,width=1280,height=1024,framerate={ 5/1, "
        "10/1, 15/1, 20/1, 30/1, 50/1, 70/1, 80/1, 90/1 }; "
        "video/x-bayer, format=(string)rggb, width=(int)[ 256, 1280 ], "
        "height=(int)[ 16, 1024 ], framerate=(fraction)[ 5/1, 2921875/486 "
        "]",
        "video/x-bayer,format=rggb,width=1280,height=1024,framerate=90/1",
    },

    {
        "DFK37UX173",
        "video/x-bayer, format=(string)rggb, width=(int)1440, "
        "height=(int)1080, framerate=(fraction){ 2500000/10593, "
        "120/1, 60/1, 30/1, 15/1, 5/1, 1/1 };"
        "video/x-bayer, format=(string)rggb, width=(int)640, "
        "height=(int)480, framerate=(fraction){ 600/1, 480/1, "
        "5000000/20833, 120/1, 60/1, 30/1, 15/1, 5/1, 1/1 };"
        "video/x-bayer, format=(string)rggb16, width=(int)1440, "
        "height=(int)1080, framerate=(fraction){ 120/1, 60/1, "
        "30/1, 15/1, 5/1, 1/1 };"
        "video/x-bayer, format=(string)rggb16, width=(int)640, "
        "height=(int)480, framerate=(fraction){ 600/1, 480/1, "
        "5000000/20833, 120/1, 60/1, 30/1, 15/1, 5/1, 1/1 }",
        "video/x-bayer, format=(string)rggb, "
        "width=(int)1440, height=(int)1080, "
        "framerate=2500000/10593",
    },

    {
        "Multi Mono test",
        "video/x-raw,format=GRAY8,width=640,height=480,framerate=15/1"
        "video/x-raw,format=GRAY16_LE,width=640,height=480,framerate=15/1",
        "video/x-raw,format=GRAY16_LE,width=640,height=480,framerate=15/1",
    },

    // {
    //     .name = "",
    //     .input_caps = "",
    //     .expected_output = "",
    // },

};

#endif /* TCAM_TEST_FIND_LARGEST_CAPS_DATA_H */
