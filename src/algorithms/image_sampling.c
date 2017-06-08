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


#include "image_sampling.h"

/* retrieve sampling points for image analysis */
void get_sampling_points (unsigned char* data, auto_sample_points* points, tBY8Pattern pattern, gst_tcam_image_size size)
{
    unsigned int width = size.width;
    unsigned int height = size.height;

    static const unsigned int bypp = 1;

    unsigned int first_line_offset = initial_offset(pattern, width, 8);

    /* bayer8; aravis currently does not support 16 and other */
    unsigned int bytes_per_line = 8 * width / 8;

    unsigned int cnt = 0;
    unsigned int sampling_line_step = height / (SAMPLING_LINES + 1);

    unsigned int y;
    for (y = sampling_line_step; y < (height - sampling_line_step); y += sampling_line_step)
    {
        unsigned int samplingColStep = ((width) / (SAMPLING_COLUMNS+1));

        byte* pLine = (byte*)data + first_line_offset + y * bytes_per_line;
        byte* pNextLine = pLine + bytes_per_line;

        unsigned int col;
        for (col = samplingColStep; col < (width - samplingColStep); col += samplingColStep)
        {
            unsigned int r = 0, g = 0, b = 0;
            if ( y & 1 )
            {
                if (col & 1)
                {
                    r = pLine[col+bypp];
                    g = pLine[col];
                    b = pNextLine[col];
                }
                else
                {
                    r = pLine[col];
                    g = pLine[col+bypp];
                    b = pNextLine[col+bypp];
                }
            }
            else
            {
                if (col & 1)
                {
                    r = pNextLine[col+bypp];
                    g = pLine[col+bypp];
                    b = pLine[col];
                }
                else
                {
                    r = pNextLine[col];
                    g = pLine[col];
                    b = pLine[col+bypp];
                }
            }

            if (cnt < ARRAYSIZE( points->samples ))
            {
                points->samples[cnt].r = (byte)r;
                points->samples[cnt].g = (byte)g;
                points->samples[cnt].b = (byte)b;
                ++cnt;
            }
        }
    }
    points->cnt = cnt;
}


void get_sampling_points_from_buffer (image_buffer* buf,
                                      auto_sample_points* points)
{
    unsigned int* data = (unsigned int*)buf->image;

    unsigned int width = buf->width;
    unsigned int height = buf->height;

    static const unsigned int bypp = 1;

    unsigned int first_line_offset = initial_offset(buf->pattern, width, 8);

    unsigned int bytes_per_line = buf->rowstride;

    unsigned int cnt = 0;
    unsigned int sampling_line_step = height / (SAMPLING_LINES + 1);

    unsigned int y;
    for (y = sampling_line_step; y < (height - sampling_line_step); y += sampling_line_step)
    {
        unsigned int samplingColStep = ((width) / (SAMPLING_COLUMNS+1));

        byte* pLine = (byte*)data + first_line_offset + y * bytes_per_line;
        byte* pNextLine = pLine + bytes_per_line;

        unsigned int col;
        for (col = samplingColStep; col < (width - samplingColStep); col += samplingColStep)
        {
            unsigned int r = 0, g = 0, b = 0;
            if ( y & 1 )
            {
                if (col & 1)
                {
                    r = pLine[col+bypp];
                    g = pLine[col];
                    b = pNextLine[col];
                }
                else
                {
                    r = pLine[col];
                    g = pLine[col+bypp];
                    b = pNextLine[col+bypp];
                }
            }
            else
            {
                if (col & 1)
                {
                    r = pNextLine[col+bypp];
                    g = pLine[col+bypp];
                    b = pLine[col];
                }
                else
                {
                    r = pNextLine[col];
                    g = pLine[col];
                    b = pLine[col+bypp];
                }
            }

            if (cnt < ARRAYSIZE( points->samples ))
            {
                points->samples[cnt].r = (byte)r;
                points->samples[cnt].g = (byte)g;
                points->samples[cnt].b = (byte)b;
                ++cnt;
            }
        }
    }
    points->cnt = cnt;
}

static unsigned int clip( unsigned int x )
{
	if( x > 255 ) return 255;
	return x;
}


unsigned int image_brightness_bayer (image_buffer* buf)
{
    auto_sample_points points = {0};

    get_sampling_points_from_buffer(buf, &points);

    unsigned int r = 0;
    unsigned int g = 0;
    unsigned int b = 0;

    unsigned int x;
    for (x = 0; x < points.cnt; ++x)
    {
        r += clip(points.samples[x].r);
        g += clip(points.samples[x].g);
        b += clip(points.samples[x].b);
    }

    r /= points.cnt;
    g /= points.cnt;
    b /= points.cnt;

    return (r + g + b) / 3;
}

unsigned int buffer_brightness_gray (image_buffer* buf)
{
    unsigned int brightness = 0;
    unsigned char*data = (unsigned char*)buf->image;

    /* GstCaps *caps = GST_BUFFER_CAPS(buf); */
    /* GstStructure *structure = gst_caps_get_structure (caps, 0); */

    /* gint width, height; */
    /* g_return_if_fail (gst_structure_get_int (structure, "width", &width)); */
    /* g_return_if_fail (gst_structure_get_int (structure, "height", &height)); */
    /* TODO find from pad https://github.com/kkonopko/gstreamer/blob/master/docs/random/porting-to-1.0.txt#L267 */


    unsigned int width = buf->width;
    unsigned int height = buf->height;

    // currently only 8bit formats are supported
    unsigned int byte_per_pixel = 1;
    int pitch = width * byte_per_pixel;
    unsigned int cnt = 0;
    unsigned int y_accu = 0;
    unsigned int sampling_line_step = width / (SAMPLING_LINES + 1);

    unsigned int y;
    for (y = sampling_line_step; y < width; y += sampling_line_step )
    {
        unsigned int samplingColStep = ((height) / (SAMPLING_COLUMNS+1));
        byte* pLine = data + y * pitch;

        unsigned int col;
        for (col = samplingColStep; col < height; col += samplingColStep)
        {
            ++cnt;
            y_accu += pLine[col];
        }
    }

    if (cnt)
    {
        brightness = y_accu / cnt;
    }
    return brightness;
}
