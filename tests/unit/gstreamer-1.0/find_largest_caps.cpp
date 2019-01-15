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

#include <catch.hpp>

#include <gst/gst.h>

#include "tcamgstbase.h"

#include "find_largest_caps_test_data.h"

TEST_CASE("find_largest_caps") {
  for (unsigned int x = 0; x < flc_test_data.size(); x++) {
    auto &entry = flc_test_data.at(x);

    GstCaps *in = nullptr;
    GstCaps *out = nullptr;

    if (entry.input_caps)
      in = gst_caps_from_string(entry.input_caps);

    if (entry.expected_output)
      out = gst_caps_from_string(entry.expected_output);
    else {
    }

    GstCaps *result = tcam_gst_find_largest_caps(in);

    INFO("in " << gst_caps_to_string(in));
    INFO("result " << gst_caps_to_string(result));
    INFO("expected " << gst_caps_to_string(out));

    REQUIRE(gst_caps_is_equal(result, out));

    gst_caps_unref(in);
    gst_caps_unref(result);
    gst_caps_unref(out);
  }
}
