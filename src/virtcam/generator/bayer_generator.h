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

#pragma once

#include "dutils_img/image_fourcc_enum.h"
#include "dutils_img/image_fourcc_func.h"
#include "dutils_img/image_transform_base.h"
#include "pattern_generator.h"
#include "generator_base.h"

#include <cmath>
#include <cstdint>
#include <dutils_img/image_bayer_pattern.h>
#include <dutils_img/pixel_structs.h>

using namespace img::pixel_type;

namespace tcam::generator
{

struct line_values
{
    uint16_t v0, v1;
};

struct bayer8
{
    uint8_t v0, v1;
};

struct bayer16
{
    uint16_t v0, v1;
};


template<typename T> T generate_struct(line_values vals) = delete;


template<> inline bayer8 generate_struct<bayer8>(line_values vals)
{
    return bayer8 { static_cast<uint8_t>(vals.v0), static_cast<uint8_t>(vals.v1) };
}

template<> inline bayer16 generate_struct<bayer16>(line_values vals)
{
    return bayer16 { vals.v0, vals.v1 };
}


template<> inline RAW12_MIPI_PACKED generate_struct<RAW12_MIPI_PACKED>(line_values vals)
{
    return RAW12_MIPI_PACKED { static_cast<uint8_t>(vals.v0 >> 8),
                               static_cast<uint8_t>(vals.v1 >> 8),
                               static_cast<uint8_t>((vals.v0 >> 4 & 0x0F) | (vals.v1 >> 4 & 0x0F) << 4) };
}


//
// actual fill image function
//
template<class TStructType, int struct_step_size, int x_step_size>
void fill_image_base(img::img_descriptor dst, line_values even, line_values odd)
{
    for (int y = 0; y < dst.dim.cy; y += 2)
    {
        auto line_start0 = img::get_line_start<TStructType>(dst, y + 0);
        auto line_start1 = img::get_line_start<TStructType>(dst, y + 1);

        int struct_index = 0;
        for (int x = 0; x < dst.dim.cx; x += x_step_size)
        {
            line_start0[struct_index] = generate_struct<TStructType>(even);
            line_start1[struct_index] = generate_struct<TStructType>(odd);

            struct_index += struct_step_size;
        }
    }
}


template<enum img::fourcc T> class BayerGenerator : public IGenerator
{

    static_assert(img::is_bayer_fcc(T), "Must be bayer!");

private:
    // maximum possible value for a single pixel
    // e.g. 255 for bayer8
    static constexpr uint16_t CLR_MAX = pow(2, img::get_bits_per_pixel(T)) - 1;


    constexpr uint16_t calc_default_speed()
    {
        // somewhat useable method to get usable default speed;
        // use width of color space to guess a valid sub step;
        // -1 as ranges are 0-255, etc not 1-256
        uint16_t ret = (1 << (img::get_bits_per_pixel(T) / 2)) - 1;

        if (ret < 20)
            // we are this close to 1, just use that instead
            return 1;
        return ret;
    }

    pattern::ColorWheel pattern_generator_;


public:
    BayerGenerator<T>() : pattern_generator_(CLR_MAX, calc_default_speed()) {}


    line_values generate_even()
    {
        auto pix = pattern_generator_.get_pixel();

        if (img::by_transform::convert_bayer_fcc_to_pattern(T) == img::by_transform::by_pattern::RG)
        {
            return { pix.r, pix.g };
        }
        else if (img::by_transform::convert_bayer_fcc_to_pattern(T)
                 == img::by_transform::by_pattern::GR)
        {
            return { pix.g, pix.r };
        }
        else if (img::by_transform::convert_bayer_fcc_to_pattern(T)
                 == img::by_transform::by_pattern::GB)
        {
            return { pix.g, pix.b };
        }
        else
        {
            return { pix.b, pix.g };
        }
    }

    line_values generate_odd()
    {
        auto pix = pattern_generator_.get_pixel();

        if (img::by_transform::convert_bayer_fcc_to_pattern(T) == img::by_transform::by_pattern::RG)
        {
            return { pix.g, pix.b };
        }
        else if (img::by_transform::convert_bayer_fcc_to_pattern(T)
                 == img::by_transform::by_pattern::GR)
        {
            return { pix.b, pix.g };
        }
        else if (img::by_transform::convert_bayer_fcc_to_pattern(T)
                 == img::by_transform::by_pattern::GB)
        {
            return { pix.r, pix.g };
        }
        else
        {
            return { pix.g, pix.r };
        }
    }

    // switch to next image
    // this function iterates through all colors
    // by increasing a channel to max before increasing the next one
    // after that the first one will be decreased before beginning anew
    void step() final
    {
        pattern_generator_.step();
    }

    void fill_image(img::img_descriptor& dst) final
    {

        if constexpr (T == img::fourcc::RGGB8
                      || T == img::fourcc::GRBG8
                      || T == img::fourcc::BGGR8
                      || T == img::fourcc::GBRG8)
        {
            fill_image_base<bayer8, 1, 2>(dst, generate_even(), generate_odd());
        }
        else if constexpr (T == img::fourcc::RGGB12_MIPI_PACKED
                           || T == img::fourcc::GRBG12_MIPI_PACKED
                           || T == img::fourcc::BGGR12_MIPI_PACKED
                           || T == img::fourcc::GBRG12_MIPI_PACKED)
        {
            fill_image_base<RAW12_MIPI_PACKED, 1, 2>(dst, generate_even(), generate_odd());
        }
        else if constexpr (T == img::fourcc::RGGB16
                           || T == img::fourcc::GRBG16
                           || T == img::fourcc::BGGR16
                           || T == img::fourcc::GBRG16)
        {
            fill_image_base<bayer16, 1, 2>(dst, generate_even(), generate_odd());
        }
        else
        {
            //static_assert(false, "Fourcc not implemented!");
        }
    }

}; // class BayerGenerator

} // namespace tcam::generator
