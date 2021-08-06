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

#include "caps/caps.h"

#include <gst/gst.h>
#include "../../src/gstreamer-1.0/tcamgstbase/tcamgstbase.h"

//
//
// the intention of this program is to act as a boilerplate
// to test specific caps for find_input_caps
// and to ease debugging.
//
//

bool verify_flag(const char* name, bool flag, bool expected, bool still_good)
{
    if (flag != expected)
    {
        const char* exp_str;
        const char* got_str;

        if (flag)
        {
            got_str = "true";
        }
        else
        {
            got_str = "false";
        }

        if (expected)
        {
            exp_str = "true";
        }
        else
        {
            exp_str = "false";
        }

        printf("%s is wrong. Expected '%s' but got '%s'\n", name, exp_str, got_str);

        return false;
    }

    return still_good;
}


int main(int argc, char* argv[])
{
    gst_init(&argc, &argv);

    bool test_succeeded = true;

    struct tcam::gst::input_caps_required_modules modules;
    struct tcam::gst::input_caps_toggles toggles;
    struct tcam::gst::input_caps_required_modules expected;

    expected.tcamconvert = false;
    expected.videoconvert = false;
    expected.jpegdec = false;
    expected.dutils = false;

    const char* src_caps_str =
        "video/x-raw,format={GRAY8, GRAY16_LE},width=1600,height=1200,framerate={20/1, 15/1, 15/2, "
        "15/4};";
    const char* sink_caps_str = "video/x-raw,format=GRAY8,width=1600,height=1200,framerate=15/1";
    const char* expected_caps_str =
        "video/x-raw,format=GRAY8,width=1600,height=1200,framerate=15/1";

    // STOP EDITING

    GstCaps* src_caps = gst_caps_from_string(src_caps_str);
    GstCaps* sink_caps = gst_caps_from_string(sink_caps_str);
    GstCaps* expected_caps = gst_caps_from_string(expected_caps_str);

    GstCaps* result_caps = find_input_caps(src_caps, sink_caps, modules, toggles);

    if (!gst_caps_is_equal(result_caps, expected_caps))
    {
        printf("Caps are not equal.\n %s\n\n%s",
               (const char*)gst_caps_to_string(result_caps),
               (const char*)gst_caps_to_string(expected_caps));
        test_succeeded = false;
    }


    test_succeeded =
        verify_flag("videoconvert", modules.videoconvert, expected.videoconvert, test_succeeded);

    test_succeeded = verify_flag("jpegenc", modules.jpegdec, expected.jpegdec, test_succeeded);

    test_succeeded = verify_flag("dutils", modules.dutils, expected.dutils, test_succeeded);


    if (test_succeeded)
    {
        printf("Caps test succeeded\n");
    }

    gst_caps_unref(src_caps);
    gst_caps_unref(sink_caps);
    gst_caps_unref(expected_caps);
    gst_caps_unref(result_caps);

    gst_deinit();

    return 0;
}
