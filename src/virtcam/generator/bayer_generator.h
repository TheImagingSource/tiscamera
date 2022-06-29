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
#include <memory>
#include <type_traits>

using namespace img::pixel_type;

namespace tcam::generator
{

struct line_values
{
    uint16_t v0, v1;
};


class BayerGenerator : public IGenerator
{

private:

    uint16_t calc_default_speed() const
    {
        // somewhat useable method to get usable default speed;
        // use width of color space to guess a valid sub step;
        // -1 as ranges are 0-255, etc not 1-256
        uint16_t ret = (1 << (img::get_bits_per_pixel(fourcc_) / 2)) -1;

        if (ret < 40)
            // we are this close to 1, just use that instead
            return 1;
        return ret;
    }

    std::unique_ptr<pattern::ColorWheel> pattern_generator_;
    img::fourcc fourcc_;

public:

    explicit BayerGenerator (img::fourcc fcc);

    line_values generate_even();

    line_values generate_odd();

    static bool is_supported_fcc(img::fourcc fcc)
    {
        if (fcc == img::fourcc::RGGB8
            || fcc == img::fourcc::GRBG8
            || fcc == img::fourcc::BGGR8
            || fcc == img::fourcc::GBRG8
            || fcc == img::fourcc::RGGB12_MIPI_PACKED
            || fcc == img::fourcc::GRBG12_MIPI_PACKED
            || fcc == img::fourcc::BGGR12_MIPI_PACKED
            || fcc == img::fourcc::GBRG12_MIPI_PACKED
            || fcc == img::fourcc::RGGB16
            || fcc == img::fourcc::GRBG16
            || fcc == img::fourcc::BGGR16
            || fcc == img::fourcc::GBRG16)
        {
            return true;
        }

        return false;
    }

    // switch to next image
    // this function iterates through all colors
    // by increasing a channel to max before increasing the next one
    // after that the first one will be decreased before beginning anew
    void step() final;

    void fill_image(img::img_descriptor& dst) final;

}; // class BayerGenerator

} // namespace tcam::generator
