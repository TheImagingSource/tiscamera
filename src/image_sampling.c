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
void get_sampling_points (GstBuffer* buf, auto_sample_points* points, tBY8Pattern pattern)
{
    guint8 *data = (guint8*)GST_BUFFER_DATA (buf);
    GstCaps *caps = GST_BUFFER_CAPS (buf);
    GstStructure *structure = gst_caps_get_structure (caps, 0);
    gint width, height;
    g_return_if_fail (gst_structure_get_int (structure, "width", &width));
    g_return_if_fail (gst_structure_get_int (structure, "height", &height));

    static const unsigned int bypp = 1;

    guint first_line_offset = initial_offset(pattern, width, 8);

    /* bayer8; aravis currently does not support 16 and other */
    gint bytes_per_line = 8 * width / 8;
    
    guint cnt = 0;
    guint sampling_line_step = height / (SAMPLING_LINES + 1);
    
    guint y;
    for (y = sampling_line_step; y < (height - sampling_line_step); y += sampling_line_step)
    {
        guint samplingColStep = ((width) / (SAMPLING_COLUMNS+1));

        byte* pLine = data + first_line_offset + y * bytes_per_line;
        byte* pNextLine = pLine + bytes_per_line;

        guint col;
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


guint image_brightness_bayer (GstBuffer* buf, tBY8Pattern pattern)
{
    auto_sample_points points = {};

    get_sampling_points (buf, &points, pattern);

    unsigned int r = 0;
    unsigned int g = 0;
    unsigned int b = 0;
    
    guint x;
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

guint buffer_brightness_gray (GstBuffer* buf)
{
    guint brightness = 0;
    guint8 *data = (guint8*) GST_BUFFER_DATA (buf);
    GstCaps *caps = GST_BUFFER_CAPS(buf);
    GstStructure *structure = gst_caps_get_structure (caps, 0);
    
    gint width, height;
    g_return_if_fail (gst_structure_get_int (structure, "width", &width));
    g_return_if_fail (gst_structure_get_int (structure, "height", &height));

    // currently only 8bit formats are supported
    guint byte_per_pixel = 1;
    gint pitch = width * byte_per_pixel;
    guint cnt = 0;
    guint y_accu = 0;
    guint sampling_line_step = width / (SAMPLING_LINES + 1);
    
    guint y;
    for (y = sampling_line_step; y < width; y += sampling_line_step )
    {
        guint samplingColStep = ((height) / (SAMPLING_COLUMNS+1));
        byte* pLine = data + y * pitch;

        guint col;
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
