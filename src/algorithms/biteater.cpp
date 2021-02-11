/*
 * Copyright 2017 The Imaging Source Europe GmbH
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

#include "biteater.h"

#include "img/image_transform_base.h"


void tcam::biteater::para_callback::call(const tcam_image_buffer& image_in,
                                         const tcam_image_buffer& image_out)
{
    int channel_in_red = 0;
    int channel_in_green = 1;
    int channel_in_blue = 2;
    int channel_in_alpha = 3;

    int channel_out_red = 0;
    int channel_out_green = 1;
    int channel_out_blue = 2;
    int channel_out_alpha = 3;

    unsigned char* Image8bit = image_out.pData;
    unsigned short int* Image16bit = (unsigned short*)image_in.pData;
    int width = image_in.format.width;
    int height = image_in.format.height;
    /*--< Find maximum value in the image >--*/
    int pixelmax = 0;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int red = Image16bit[(x + y * width) * 4 + 2];
            int green = Image16bit[(x + y * width) * 4 + 1];
            int blue = Image16bit[(x + y * width) * 4];
            int alpha = Image16bit[(x + y * width) * 4 + 3];

            // Exhange upper 8bit and lower 8bit for Intel x86
            red = ((red & 0x00ff) << 8) | ((red & 0xff00) >> 8);
            green = ((green & 0x00ff) << 8) | ((green & 0xff00) >> 8);
            blue = ((blue & 0x00ff) << 8) | ((blue & 0xff00) >> 8);
            alpha = ((alpha & 0x00ff) << 8) | ((alpha & 0xff00) >> 8);

            if (pixelmax < red)
                pixelmax = red;
            if (pixelmax < green)
                pixelmax = green;
            if (pixelmax < blue)
                pixelmax = blue;
            if (pixelmax < alpha)
                pixelmax = alpha;
        }
    }

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int red = Image16bit[(x + y * width) * 4 + channel_in_red];
            int green = Image16bit[(x + y * width) * 4 + channel_in_green];
            int blue = Image16bit[(x + y * width) * 4 + channel_in_blue];
            int alpha = Image16bit[(x + y * width) * 4 + channel_in_alpha];

            // // Exhange upper 8bit and lower 8bit for Intel x86
            // red   = ((red   & 0x00ff) << 8) | ((red   & 0xff00) >> 8);
            // green = ((green & 0x00ff) << 8) | ((green & 0xff00) >> 8);
            // blue  = ((blue  & 0x00ff) << 8) | ((blue  & 0xff00) >> 8);
            // alpha = ((alpha & 0x00ff) << 8) | ((alpha & 0xff00) >> 8);

            Image8bit[(x + y * width) * 4 + channel_out_red] = red * 255 / pixelmax;
            Image8bit[(x + y * width) * 4 + channel_out_green] = green * 255 / pixelmax;
            Image8bit[(x + y * width) * 4 + channel_out_blue] = blue * 255 / pixelmax;
            Image8bit[(x + y * width) * 4 + channel_out_alpha] = alpha * 255 / pixelmax;
        }
    }
}


struct tcam::biteater::offsets tcam::biteater::offsets_for_fourcc(unsigned int fourcc)
{
    struct offsets ret = {};
    // int channel_in_red   = 0;
    // int channel_in_green = 1;
    // int channel_in_blue  = 2;
    // int channel_in_alpha = 3;

    // int channel_out_red   = 0;
    // int channel_out_green = 1;
    // int channel_out_blue  = 2;
    // int channel_out_alpha = 3;

    switch (fourcc)
    {
        case FOURCC_BGRA32:
        {
            ret = { 2, 1, 0, 3 };
            break;
        }
        case FOURCC_BGRA64: // BGRX 16Bit per Channel
        {
            // ret = {4, 2, 0, 6};
            ret = { 4, 2, 0, 6 };
            break;
        }
        default:
        {
            break;
        }
    }

    return ret;
}


bool tcam::biteater::init_meta(struct biteater_meta& meta,
                               const tcam_video_format& in,
                               const tcam_video_format& out)
{

    meta.offset_in = offsets_for_fourcc(in.fourcc);
    meta.offset_out = offsets_for_fourcc(out.fourcc);

    if (meta.offset_in.empty() || meta.offset_out.empty())
    {
        return false;
    }

    meta.para = std::make_shared<parallel::parallel_state>();

    return true;
}


bool tcam::biteater::transform(const struct tcam_image_buffer* image_in,
                               struct tcam_image_buffer* image_out,
                               const struct biteater_meta& meta)
{
    if ((image_in->format.width != image_out->format.width)
        || (image_in->format.height != image_out->format.height))
    {
        return false;
    }

    para_callback cb;

    meta.para->queue_and_wait(&cb, *image_in, *image_out, 0);

    return true;
}
