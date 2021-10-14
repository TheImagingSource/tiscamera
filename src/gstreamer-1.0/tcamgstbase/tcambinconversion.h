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


typedef enum {
    TCAM_BIN_CONVERSION_AUTO = 0,
    TCAM_BIN_CONVERSION_CONVERT = 1,
    TCAM_BIN_CONVERSION_DUTILS = 2,
    TCAM_BIN_CONVERSION_CUDA = 3,
} TcamBinConversionElement;

GType tcam_bin_conversion_element_get_type(void);

namespace tcam::gst
{



struct input_caps_required_modules
{
    bool tcamconvert = false;
    bool videoconvert = false;
    bool jpegdec = false;
    bool dutils = false;

    bool operator==(const input_caps_required_modules& other) const noexcept
    {
        if (tcamconvert == other.tcamconvert
            && videoconvert == other.videoconvert && jpegdec == other.jpegdec
            && dutils == other.dutils)
        {
            return true;
        }
        return false;
    }

    std::string to_string() const
    {
        std::string ret = "tcamconvert=";
        tcamconvert ? ret+="true," : ret+="false,";

        ret+="videoconvert=";
        videoconvert ? ret+="true," : ret+="false,";

        ret+="jpegdec=";
        jpegdec ? ret+="true," : ret+="false,";

        ret+="dutils=";
        dutils ? ret+="true" : ret+="false";

        return ret;
    }
};


/**
 * @param available_caps - caps the source offers
 * @param wanted_caps - caps the sink wants, if null available_caps will be returned
 * @param requires_bayertransform(out) - will be set to true when the tcambayertransform element is required
 * @param requires_bayer2rgb(out) - will be set to true when the bayer2rgb element is required
 * @param requires_vidoeconvert(out) - will be set to true when the videoconvert element is required
 * @param requires_jpegconvert(out) - will be set to true when the jpegdec element is required
 * @param use_dutils(in) - false when dutils shall be ignored
 *
 * @return possible caps for the source
 */
GstCaps* find_input_caps(GstCaps* available_caps,
                         GstCaps* wanted_caps,
                         input_caps_required_modules& modules,
                         TcamBinConversionElement toggle);


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
    BAYER_PWL,
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
                                            TcamBinConversionElement toggles) const;
};

} // namespace tcam::gst
