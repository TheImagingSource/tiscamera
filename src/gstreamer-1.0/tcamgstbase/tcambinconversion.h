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

#pragma once

#include "tcamgstbase.h"

#include <gst-helper/gst_ptr.h>
#include <gst/gst.h>
#include <vector>

namespace tcam::gst
{
enum class CAPS_TYPE
{
    BAYER_8,
    BAYER_10,
    BAYER_12,
    BAYER_16,
    RGB_24,
    RGB_32,
    RGB_64,
    MONO_8,
    MONO_10,
    MONO_12,
    MONO_16,
    JPEG,
    YUV,
    TIS_POLARIZED,
    FLOATING,
};

class TcamBinConversion
{
private:
    struct caps_map
    {
        CAPS_TYPE type;
        gst_helper::gst_ptr<GstCaps> caps;
    };

    std::vector<caps_map> m_caps_table;
public:
    explicit TcamBinConversion();

    gst_helper::gst_ptr<GstCaps> get_caps(CAPS_TYPE type) const;

    bool is_compatible(GstCaps* to_check, CAPS_TYPE compatible_with) const;

    input_caps_required_modules get_modules(GstCaps* caps,
                                            GstCaps* wanted_output,
                                            input_caps_toggles toggles) const;
};

} // namespace tcam::gst
