/*
 * Copyright 2013 The Imaging Source Europe GmbH
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

#include "bayer.h"


tBY8Pattern next_pixel (tBY8Pattern pattern)
{
    switch (pattern)
    {
        case GB:    return BG;
        case GR:    return RG;
        case RG:    return GR;
        case BG:
        default:    return GB;
    };
    return BG;
}


tBY8Pattern next_line (tBY8Pattern pattern)
{
    switch (pattern)
    {
        case GB:    return RG;
        case GR:    return BG;
        case RG:    return GB;
        case BG:
        default:    return BG;
    };
}


const char* bayer_to_string (tBY8Pattern pattern)
{
    switch (pattern)
    {
        case GB:    return "gbrg";
        case GR:    return "grbg";
        case RG:    return "rggb";
        case BG:
        default:    return "bggr";
    };
}


unsigned int initial_offset (tBY8Pattern pattern, unsigned int line_width, unsigned int bytes_per_pixel)
{
    unsigned int first_line_offset = 0;

    /* bayer8; aravis currently does not support 16 and other */
    unsigned int bytes_per_line = bytes_per_pixel * line_width / 8;

    switch (pattern)
    {
        case BG:
            first_line_offset += 1;
            break;
        case GB:
            first_line_offset += 0;
            break;
        case GR:
            first_line_offset += bytes_per_line + 1;
            break;
        case RG:
            first_line_offset += bytes_per_line;
            break;
        default:
            break;
    }
    return first_line_offset;
}
