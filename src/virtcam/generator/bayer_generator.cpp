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

#include "bayer_generator.h"
#include "dutils_img/image_fourcc_enum.h"

namespace {
struct bayer8
{
    uint8_t v0, v1;
};

struct bayer16
{
    uint16_t v0, v1;
};


template<typename T> T generate_struct(tcam::generator::line_values vals) = delete;


template<> inline bayer8 generate_struct<bayer8>(tcam::generator::line_values vals)
{
    return bayer8 { static_cast<uint8_t>(vals.v0), static_cast<uint8_t>(vals.v1) };
}

template<> inline bayer16 generate_struct<bayer16>(tcam::generator::line_values vals)
{
    return bayer16 { vals.v0, vals.v1 };
}


template<>
inline RAW12_MIPI_PACKED generate_struct<RAW12_MIPI_PACKED>(tcam::generator::line_values vals)
{
    return RAW12_MIPI_PACKED { static_cast<uint8_t>(vals.v0 >> 4),
                               static_cast<uint8_t>(vals.v1 >> 4),
                               static_cast<uint8_t>((vals.v0 >> 4 & 0x0F)
                                                    | (vals.v1 >> 4 & 0x0F) << 4) };
}


//
// actual fill image function
//
template<class TStructType, int struct_step_size, int x_step_size>
void fill_image_base(img::img_descriptor dst,
                     tcam::generator::line_values even,
                     tcam::generator::line_values odd)
{
    TStructType struct_even = generate_struct<TStructType>(even);
    TStructType struct_odd = generate_struct<TStructType>(odd);

    //SPDLOG_INFO("filling with [ {},{} : {},{} ]", even.v0, even.v1, odd.v0, odd.v1);

    for (int y = 0; y < dst.dim.cy; y += 2)
    {
        auto line_start0 = img::get_line_start<TStructType>(dst, y + 0);
        auto line_start1 = img::get_line_start<TStructType>(dst, y + 1);

        int struct_index = 0;
        for (int x = 0; x < dst.dim.cx; x += x_step_size)
        {
            line_start0[struct_index] = struct_even;
            line_start1[struct_index] = struct_odd;

            struct_index += struct_step_size;
        }
    }
}

} // namespace



tcam::generator::BayerGenerator::BayerGenerator(img::fourcc fcc)
{
    fourcc_ = fcc;
    uint16_t max = pow(2, img::get_bits_per_pixel(fourcc_)) - 1;
    pattern_generator_ = std::make_unique<pattern::ColorWheel>(max, calc_default_speed());
}


tcam::generator::line_values tcam::generator::BayerGenerator::generate_even()
{
    auto pix = pattern_generator_->get_pixel();

    if (img::by_transform::convert_bayer_fcc_to_pattern(fourcc_)
        == img::by_transform::by_pattern::RG)
    {
        return { pix.r, pix.g };
    }
    else if (img::by_transform::convert_bayer_fcc_to_pattern(fourcc_)
             == img::by_transform::by_pattern::GR)
    {
        return { pix.g, pix.r };
    }
    else if (img::by_transform::convert_bayer_fcc_to_pattern(fourcc_)
             == img::by_transform::by_pattern::GB)
    {
        return { pix.g, pix.b };
    }
    else
    {
        return { pix.b, pix.g };
    }
}


tcam::generator::line_values tcam::generator::BayerGenerator::generate_odd()
{
    auto pix = pattern_generator_->get_pixel();

    if (img::by_transform::convert_bayer_fcc_to_pattern(fourcc_)
        == img::by_transform::by_pattern::RG)
    {
        return { pix.g, pix.b };
    }
    else if (img::by_transform::convert_bayer_fcc_to_pattern(fourcc_)
             == img::by_transform::by_pattern::GR)
    {
        return { pix.b, pix.g };
    }
    else if (img::by_transform::convert_bayer_fcc_to_pattern(fourcc_)
             == img::by_transform::by_pattern::GB)
    {
        return { pix.r, pix.g };
    }
    else
    {
        return { pix.g, pix.r };
    }
}


void tcam::generator::BayerGenerator::step()
{
    pattern_generator_->step();
}


void tcam::generator::BayerGenerator::fill_image(img::img_descriptor& dst)
{
    if (img::is_by8_fcc(fourcc_))
    {
        fill_image_base<bayer8, 1, 2>(dst, generate_even(), generate_odd());
    }
    else if (fourcc_ == img::fourcc::RGGB12_MIPI_PACKED
             || fourcc_ == img::fourcc::GRBG12_MIPI_PACKED
             || fourcc_ == img::fourcc::BGGR12_MIPI_PACKED
             || fourcc_ == img::fourcc::GBRG12_MIPI_PACKED)
    {
        fill_image_base<RAW12_MIPI_PACKED, 1, 2>(dst, generate_even(), generate_odd());
    }
    else if (img::is_by16_fcc(fourcc_))
    {
        fill_image_base<bayer16, 1, 2>(dst, generate_even(), generate_odd());
    }
}
