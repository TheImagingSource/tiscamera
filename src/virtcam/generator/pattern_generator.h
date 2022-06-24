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

#include "dutils_img/pixel_structs.h"

#include <cstdint>
#include <random>

namespace tcam::generator::pattern
{

//
// generates colors around the color wheel
//
class ColorWheel
{
private:

    uint16_t r_ = 0;
    uint16_t g_ = 0;
    uint16_t b_ = 0;

    uint16_t* rising_ = &r_;
    uint16_t* descending_ = nullptr;

    uint16_t CLR_MAX = 255;
    uint16_t min_ = 0;

    // speed with which colors will change;
    // due to size of color space different values should be used
    uint16_t speed_ = 1;

    inline uint16_t get_color_min() const
    {
        return min_;
    }

    inline uint16_t get_color_max() const
    {
        return CLR_MAX;
    }

public:

    ColorWheel(uint16_t max, uint16_t speed)
        : CLR_MAX(max), speed_(speed)
    {}

    // switch to next image
    // this function iterates through all colors
    // by increasing a channel to max before increasing the next one
    // after that the first one will be decreased before beginning anew
    void step()
    {
        if (rising_)
        {
            *rising_ += speed_;
        }

        if (descending_)
        {
            *descending_ -= speed_;
        }

        uint16_t min = get_color_min();
        uint16_t max = get_color_max();

        if ((r_ >= max && g_ == min && b_ == min))
        {
            rising_ = &g_;
            descending_ = nullptr;
        }
        else if (r_ >= max && g_ >= max && b_ == min)
        {
            rising_ = nullptr;
            descending_ = &r_;
        }
        else if (g_ >= max && r_ == min && b_ == min)
        {
            rising_ = &b_;
            descending_ = nullptr;
        }
        else if (r_ == min && g_ >= max && b_ >= max)
        {
            rising_ = nullptr;
            descending_ = &g_;
        }
        else if (r_ == min && b_ >= max && g_ == min)
        {
            rising_ = &r_;
            descending_ = nullptr;
        }
        else if (g_ == min && b_ >= max && r_ >= max)
        {
            rising_ = nullptr;
            descending_ = &b_;
        }
    }

    inline img::pixel_type::BGRA64 get_pixel() const
    {
        return { b_, g_, r_, 0 };
    }


}; // class ColorWheel


//
// Random white noise
//
class WhiteNoise
{
private:
    std::default_random_engine rng_;
    std::uniform_int_distribution<uint16_t> value_gen_; // guaranteed unbiased

public:

    WhiteNoise(uint16_t max)
    {
        std::random_device rd; // only used once to initialise (seed) engine
        rng_ = std::default_random_engine(
            rd()); // random-number engine used (Mersenne-Twister in this case)
        value_gen_ = std::uniform_int_distribution<uint16_t>(0, max); // guaranteed unbiased
    }

    uint16_t get_pixel()
    {
        return value_gen_(rng_);
    }

}; // class WhiteNoise


} // namespace tcam::generator::pattern
