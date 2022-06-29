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

#include "rgb_generator.h"
#include "dutils_img/image_fourcc_enum.h"


namespace
{

uint16_t calc_default_speed(img::fourcc fcc)
{
    // somewhat useable method to get usable default speed;
    // use width of color space to guess a valid sub step;
    // -1 as ranges are 0-255, etc not 1-256
    uint16_t ret = (1 << (img::get_bits_per_pixel(fcc) / 3 / 2)) - 1;

    if (ret < 20)
        // we are this close to 1, just use that instead
        return 1;
    return ret;
}

} // namespace

tcam::generator::RGBGenerator::RGBGenerator(img::fourcc fcc) : fourcc_(fcc)
{
    uint16_t max = pow(2, img::get_bits_per_pixel(fourcc_) / 3) - 1;
    pattern_generator_ = pattern::ColorWheel(max, calc_default_speed(fourcc_));
}


void tcam::generator::RGBGenerator::step()
{
    pattern_generator_.step();
}

void tcam::generator::RGBGenerator::fill_image(img::img_descriptor& dst)
{
    if (fourcc_ == img::fourcc::BGR24)
    {
        auto pix = pattern_generator_.get_pixel();
        img::pixel_type::B8G8R8 data { static_cast<uint8_t>(pix.b),
            static_cast<uint8_t>(pix.g),
            static_cast<uint8_t>(pix.r) };

        for (int y = 0; y < dst.dim.cy; y++)
        {
            auto line = img::get_line_start<img::pixel_type::B8G8R8>(dst, y);

            for (int x = 0; x < dst.dim.cx; x++) { line[x] = data; }
        }
    }
    else if (fourcc_ == img::fourcc::BGRA32)
    {
        auto pix = pattern_generator_.get_pixel();
        img::pixel_type::BGRA32 data { static_cast<uint8_t>(pix.b),
            static_cast<uint8_t>(pix.g),
            static_cast<uint8_t>(pix.r),
            0 };

        for (int y = 0; y < dst.dim.cy; y++)
        {
            auto line = img::get_line_start<img::pixel_type::BGRA32>(dst, y);

            for (int x = 0; x < dst.dim.cx; x++) { line[x] = data; }
        }
    }
    else if (fourcc_ == img::fourcc::BGRA64)
    {
        auto pix = pattern_generator_.get_pixel();
        for (int y = 0; y < dst.dim.cy; y++)
        {
            auto line = img::get_line_start<img::pixel_type::BGRA64>(dst, y);

            for (int x = 0; x < dst.dim.cx; x++) { line[x] = pix; }
        }
    }
}
