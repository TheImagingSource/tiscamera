/*
 * Copyright 2014 The Imaging Source Europe GmbH
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

#include "public_utils.h"

#include "utils.h"
#include "standard_properties.h"
#include "format.h"

using namespace tcam;

const char* tcam::fourcc_to_description (uint32_t fourcc)
{
    return fourcc2description(fourcc);
}


uint32_t tcam::description_to_fourcc (const char* description)
{
    return description2fourcc(description);
}


std::string tcam::category2string (TCAM_PROPERTY_CATEGORY category)
{
    switch (category)
    {
        case TCAM_PROPERTY_CATEGORY_COLOR: return "Color";
        case TCAM_PROPERTY_CATEGORY_EXPOSURE: return "Exposure";
        case TCAM_PROPERTY_CATEGORY_IMAGE: return "Image";
        case TCAM_PROPERTY_CATEGORY_LENS: return "Lens";
        case TCAM_PROPERTY_CATEGORY_PARTIAL_SCAN: return "Partial Scan";
        case TCAM_PROPERTY_CATEGORY_SPECIAL: return "Special";
        case TCAM_PROPERTY_CATEGORY_UNKNOWN: return "Unknown";
        case TCAM_PROPERTY_CATEGORY_AUTO_ROI: return "Auto ROI";
        case TCAM_PROPERTY_CATEGORY_WDR: return "WDR";
        default: return "";
    }
}

std::string tcam::property_id_to_string (TCAM_PROPERTY_ID id)
{
    return property_id2string(id);
}


std::string tcam::property_type_to_string (TCAM_PROPERTY_TYPE type)
{
    return propertyType2String(type);
}


uint64_t tcam::get_image_size (uint32_t fourcc,
                               unsigned int width,
                               unsigned int height)
{
    return get_buffer_length(width, height, fourcc);
}


struct tcam_image_buffer* tcam::allocate_image_buffers (const struct tcam_video_format* format,
                                                             size_t n_buffers)
{
    struct tcam_image_buffer* ptr = nullptr;

    if (format != nullptr && n_buffers > 0)
    {
        // allocate buffer array
        ptr = (struct tcam_image_buffer*)malloc(sizeof(struct tcam_image_buffer) * n_buffers);

        unsigned int length = 0;
        // allocate the actual image data fields
        for (unsigned int i = 0; i < n_buffers; ++i)
        {
            struct tcam_image_buffer* tmp = &ptr[i];

            tmp->pData = (unsigned char*)malloc(tcam_get_required_buffer_size(format));

            // fill the rest
            tmp->length = length;
            tmp->format = *format;
            tmp->pitch = get_pitch_length(format->width, format->fourcc);
        }

    }

    return ptr;
}


void tcam::free_image_buffers (struct tcam_image_buffer* ptr, size_t n_buffer)
{
    if (ptr == nullptr || n_buffer < 1)
        return;

    free (ptr);
}


bool tcam::is_image_buffer_complete (const struct tcam_image_buffer* buffer)
{
    return is_buffer_complete(buffer);
}


std::vector<struct tcam_image_size> tcam::get_standard_resolutions (const struct tcam_image_size& min,
                                                                    const struct tcam_image_size& max)
{
    static const std::vector<struct tcam_image_size> resolutions =
        {
            {128, 96},
            {320, 240},
            {360, 280},
            {544, 480},
            {640, 480},
            {352, 288},
            {576, 480},
            {720, 480},
            {960, 720},
            {1280, 720},
            {1440, 1080},
            {1920, 1080},
            {1920, 1200},
            {2048, 1152},
            {2048, 1536},
            {2560, 1440},
            {3840, 2160},
            {4096, 3072},
            {7680, 4320},
            {7680, 4800},
        };

    std::vector<struct tcam_image_size> ret;

    for (const auto& r : resolutions)
    {
        if (is_smaller(min, r) && is_smaller(r, max))
        {
            ret.push_back(r);
        }
    }

    return ret;
}
