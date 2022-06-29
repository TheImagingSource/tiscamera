/*
 * Copyright 2022 The Imaging Source Europe GmbH
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

#include "mono_generator.h"

using namespace img::pixel_type;

tcam::generator::MonoGenerator::MonoGenerator(img::fourcc fcc)
    : fourcc_(fcc)
{
    uint16_t max = pow(2, img::get_bits_per_pixel(fourcc_)) - 1;
    pattern_gen_ = pattern::WhiteNoise(max);
}


void tcam::generator::MonoGenerator::fill_image(img::img_descriptor& dst)
{

    if (fourcc_ == img::fourcc::MONO8)
    {
        for (int y = 0; y < dst.dim.cy; y++)
        {
            auto line = img::get_line_start<Y8>(dst, y);

            for (int x = 0; x < dst.dim.cx; x++)
            {
                line[x] = { static_cast<uint8_t>(pattern_gen_.get_pixel()) };
            }
        }
    }
    else if (fourcc_ == img::fourcc::MONO12_MIPI_PACKED)
    {

        for (int y = 0; y < dst.dim.cy; y++)
        {
            auto line = img::get_line_start<RAW12_MIPI_PACKED>(dst, y);

            for (int x = 0; x < dst.dim.cx; x++)
            {
                auto v0 = pattern_gen_.get_pixel();
                auto v1 = pattern_gen_.get_pixel();

                RAW12_MIPI_PACKED pix = { static_cast<uint8_t>(v0 >> 4),
                                          static_cast<uint8_t>(v1 >> 4),
                                          static_cast<uint8_t>((v0 >> 4 & 0x0F)
                                                               | (v1 >> 4 & 0x0F) << 4) };

                line[x] = { pix };
            }
        }
    }
    else if (fourcc_ == img::fourcc::MONO16)
    {
        for (int y = 0; y < dst.dim.cy; y++)
        {
            auto line = img::get_line_start<Y16_LE>(dst, y);

            for (int x = 0; x < dst.dim.cx; x++) { line[x] = { pattern_gen_.get_pixel() }; }
        }
    }
}
