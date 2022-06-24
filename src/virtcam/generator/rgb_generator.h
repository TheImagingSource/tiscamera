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
#include "generator_base.h"
#include "pattern_generator.h"

#include <cmath>
#include <cstdint>
#include <dutils_img/image_bayer_pattern.h>
#include <dutils_img/pixel_structs.h>


namespace tcam::generator
{

static constexpr bool is_rgb(img::fourcc fcc)
{
    if (fcc == img::fourcc::BGR24
        || fcc == img::fourcc::BGRA32
        || fcc == img::fourcc::BGRA64
        || fcc == img::fourcc::BGRFloat)
    {
        return true;
    }

    return false;
}


template<enum img::fourcc T> class RGBGenerator : public IGenerator
{
    static_assert(is_rgb(T), "Must be RGB!");

private:
    // maximum possible value for a single pixel
    // e.g. 255 for bayer8
    static constexpr uint16_t CLR_MAX = pow(2, img::get_bits_per_pixel(T) / 3) - 1;

    pattern::ColorWheel pattern_generator_;

    uint16_t calc_default_speed()
    {
        // somewhat useable method to get usable default speed;
        // use width of color space to guess a valid sub step;
        // -1 as ranges are 0-255, etc not 1-256
        uint16_t ret = (1 << (img::get_bits_per_pixel(T) / 3 / 2)) - 1;

        if (ret < 20)
            // we are this close to 1, just use that instead
            return 1;
        return ret;
    }


public:
    RGBGenerator<T>() : pattern_generator_(CLR_MAX, calc_default_speed()) {}

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
        if constexpr (T == img::fourcc::BGR24)
        {
            auto pix = pattern_generator_.get_pixel();
            img::pixel_type::B8G8R8 data {
                static_cast<uint8_t>(pix.b),
                static_cast<uint8_t>(pix.g),
                static_cast<uint8_t>(pix.r)
            };

            for (unsigned int offset = 0; offset < dst.size(); offset += sizeof(data))
            {
                auto ptr = reinterpret_cast<img::pixel_type::B8G8R8*>(dst.data() + offset);

                *ptr = data;
            }
        }
        else if constexpr (T == img::fourcc::BGRA64)
        {
            auto pix = pattern_generator_.get_pixel();

            for (unsigned int offset = 0; offset < dst.size(); offset += sizeof(pix))
            {
                auto ptr = reinterpret_cast<img::pixel_type::BGRA64*>(dst.data() + offset);

                *ptr = pix;
            }
        }
        else
        {
            //static_assert(false, "Fourcc not implemented!");
        }
    }

}; // class RGBGenerator


} // namespace tcam::generator
