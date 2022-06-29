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

#include <algorithm>
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

    uint16_t max_ = 255;
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
        return max_;
    }

public:

    ColorWheel() {};

    ColorWheel(uint16_t max, uint16_t speed)
        : max_(max), speed_(speed)
    {

        r_ = 0;
        g_ = 0;
        b_ = 0;
    }

    ColorWheel(const ColorWheel& other)
    {
        max_ = other.max_;
        min_ = other.min_;
        r_ = other.r_;
        g_ = other.g_;
        b_ = other.b_;

        if (other.rising_ == &other.r_)
        {
            rising_ = &r_;
        }
        else if (other.rising_ == &other.g_)
        {
            rising_ = &g_;
        }
        else if (other.rising_ == &other.b_)
        {
            rising_ = &b_;
        }
        if (other.descending_ == &other.r_)
        {
            descending_ = &r_;
        }
        else if (other.descending_ == &other.g_)
        {
            descending_ = &g_;
        }
        else if (other.descending_ == &other.b_)
        {
            descending_ = &b_;
        }
    }

    ColorWheel& operator=(const ColorWheel& other)
    {
        max_ = other.max_;
        min_ = other.min_;
        r_ = other.r_;
        g_ = other.g_;
        b_ = other.b_;

        if (other.rising_ == &other.r_)
        {
            rising_ = &r_;
        }
        else if (other.rising_ == &other.g_)
        {
            rising_ = &g_;
        }
        else if (other.rising_ == &other.b_)
        {
            rising_ = &b_;
        }
        if (other.descending_ == &other.r_)
        {
            descending_ = &r_;
        }
        else if (other.descending_ == &other.g_)
        {
                descending_ = &g_;
        }
        else if (other.descending_ == &other.b_)
        {
            descending_ = &b_;
        }
        return *this;
    }

    // switch to next image
    // this function iterates through all colors
    // by increasing a channel to max before increasing the next one
    // after that the first one will be decreased before beginning anew
    void step()
    {
        uint16_t min = get_color_min();
        uint16_t max = get_color_max();
        if (rising_)
        {
            *rising_ += speed_;
        }

        if (descending_)
        {
            *descending_ -= speed_;
        }

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
        //SPDLOG_INFO("{}:{}:{} - {}", r_, g_, b_, speed_);
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
    std::uniform_int_distribution<uint16_t> value_gen_;

public:

    WhiteNoise() {};

    explicit WhiteNoise(uint16_t max)
    {
        std::random_device rd; // only used once to initialise (seed) engine
        rng_ = std::default_random_engine(rd());
        value_gen_ = std::uniform_int_distribution<uint16_t>(0, max);
    }

    uint16_t get_pixel()
    {
        return value_gen_(rng_);
    }

}; // class WhiteNoise


} // namespace tcam::generator::pattern
