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

#include "whitebalance.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>


static const uint MAX_STEPS = 20;
static uint WB_IDENTITY = 64;
static uint WB_MAX = 255;
static const uint BREAK_DIFF = 2;

const uint NEARGRAY_MIN_BRIGHTNESS      = 10;
const uint NEARGRAY_MAX_BRIGHTNESS      = 253;
const float NEARGRAY_MAX_COLOR_DEVIATION = 0.25f;
const float NEARGRAY_REQUIRED_AMOUNT     = 0.08f;

/* rgb values have to be evaluated differently. These are the according factors */
static const uint r_factor = (uint32_t)((1 << 8) * 0.299f);
static const uint g_factor = (uint32_t)((1 << 8) * 0.587f);
static const uint b_factor = (uint32_t)((1 << 8) * 0.114f);


/* number of sample points */
#define SAMPLING_LINES      30
#define SAMPLING_COLUMNS    40

typedef unsigned char byte;

typedef struct auto_sample_points
{
    struct pixel
    {
        byte r;
        byte g;
        byte b;
    } samples[1500];

    unsigned int	cnt;
} auto_sample_points;


#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))



/* retrieve sampling points for image analysis */
static void get_sampling_points (struct wb_settings* settings, auto_sample_points* points)
{
    unsigned int* data = (unsigned int*)settings->data;

    unsigned int width = settings->width;
    unsigned int height = settings->height;

    static const unsigned int bypp = 1;

    unsigned int first_line_offset = initial_offset(settings->pattern, width, 8);

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


static uint clip (uint x, uint max)
{
    if ( x > max )
        return max;
    return x;
}


uint calc_brightness_from_clr_avg (uint r, uint g, uint b)
{
    return (r * r_factor + g * g_factor + b * b_factor) >> 8;
}


bool is_near_gray (uint r, uint g, uint b)
{
    uint brightness = calc_brightness_from_clr_avg( r, g, b );
    if ( brightness < NEARGRAY_MIN_BRIGHTNESS ) return false;
    if ( brightness > NEARGRAY_MAX_BRIGHTNESS ) return false;

    uint deltaR = abs( (int)r - (int)brightness );
    uint deltaG = abs( (int)g - (int)brightness );
    uint deltaB = abs( (int)b - (int)brightness );

    float devR = deltaR / (float)brightness;
    float devG = deltaG / (float)brightness;
    float devB = deltaB / (float)brightness;

    return ((devR < NEARGRAY_MAX_COLOR_DEVIATION) &&
            (devG < NEARGRAY_MAX_COLOR_DEVIATION) &&
            (devB < NEARGRAY_MAX_COLOR_DEVIATION));
}


rgb_tripel simulate_whitebalance (const auto_sample_points* data, const rgb_tripel* wb, bool enable_near_gray)
{
    rgb_tripel result = { 0, 0, 0 };
    rgb_tripel result_near_gray = { 0, 0, 0 };
    unsigned int count_near_gray = 0;

    uint i;
    for (i = 0; i < data->cnt; ++i)
    {
        unsigned int r = clip( data->samples[i].r * wb->R / WB_IDENTITY, WB_MAX );
        unsigned int g = clip( data->samples[i].g * wb->G / WB_IDENTITY, WB_MAX );
        unsigned int b = clip( data->samples[i].b * wb->B / WB_IDENTITY, WB_MAX );

        result.R += r;
        result.G += g;
        result.B += b;

        if ( is_near_gray( r, g, b ) )
        {
            result_near_gray.R += r;
            result_near_gray.G += g;
            result_near_gray.B += b;
            count_near_gray += 1;
        }
    }

    float near_gray_amount = count_near_gray / (float)data->cnt;

    if ((near_gray_amount < NEARGRAY_REQUIRED_AMOUNT) || !enable_near_gray)
    {
        result.R /= data->cnt;
        result.G /= data->cnt;
        result.B /= data->cnt;
        return result;
    }
    else
    {
        result_near_gray.R /= count_near_gray;
        result_near_gray.G /= count_near_gray;
        result_near_gray.B /= count_near_gray;
        return result_near_gray;
    }
}


static rgb_tripel average_color_cam (const auto_sample_points* data )
{
    rgb_tripel result = { 0, 0, 0 };

	uint i;
    for (i = 0; i < data->cnt; ++i)
    {
        unsigned int r =  data->samples[i].r ;
        unsigned int g = data->samples[i].g ;
        unsigned int b = data->samples[i].b ;

        result.R += r;
        result.G += g;
        result.B += b;
    }

	result.R /= data->cnt;
	result.G /= data->cnt;
	result.B /= data->cnt;
	return result;
}


bool wb_auto_step (rgb_tripel* clr, rgb_tripel* wb )
{
    unsigned int avg = ((clr->R + clr->G + clr->B) / 3);
    int dr = (int)avg - clr->R;
    int dg = (int)avg - clr->G;
    int db = (int)avg - clr->B;

    if (abs(dr) < BREAK_DIFF && abs(dg) < BREAK_DIFF && abs(db) < BREAK_DIFF)
    {
        wb->R = clip( wb->R, WB_MAX );
        wb->G = clip( wb->G, WB_MAX );
        wb->B = clip( wb->B, WB_MAX );

        return true;
    }

    if ((clr->R > avg) && (wb->R > WB_IDENTITY))
    {
        wb->R -= 1;
    }

    if ((clr->G > avg) && (wb->G > WB_IDENTITY))
    {
        wb->G -= 1;
    }

    if ((clr->B > avg) && (wb->B > WB_IDENTITY))
    {
        wb->B -= 1;
    }

    if ((clr->R < avg) && (wb->R < WB_MAX))
    {
        wb->R += 1;
    }

    if ((clr->G < avg) && (wb->G < WB_MAX))
    {
        wb->G += 1;
    }

    if ((clr->B < avg) && (wb->B < WB_MAX))
    {
        wb->B += 1;
    }

    if ((wb->R > WB_IDENTITY) && (wb->G > WB_IDENTITY) && (wb->B > WB_IDENTITY))
    {
        wb->R -= 1;
        wb->G -= 1;
        wb->B -= 1;
    }

    return false;
}


bool auto_whitebalance (const auto_sample_points* data, rgb_tripel* wb, uint* resulting_brightness)
{
    rgb_tripel old_wb = *wb;
    if (wb->R < WB_IDENTITY)
        wb->R = WB_IDENTITY;
    if (wb->G < WB_IDENTITY)
        wb->G = WB_IDENTITY;
    if (wb->B < WB_IDENTITY)
        wb->B = WB_IDENTITY;
    if (old_wb.R != wb->R || old_wb.G != wb->G || old_wb.B != wb->B)
        return false;

    while ((wb->R > WB_IDENTITY) && (wb->G > WB_IDENTITY) && (wb->B > WB_IDENTITY))
    {
        wb->R -= 1;
        wb->G -= 1;
        wb->B -= 1;
    }

    unsigned int steps = 0;
    while (steps++ < MAX_STEPS)
    {
        rgb_tripel tmp = simulate_whitebalance(data, wb, true);

        // Simulate white balance once more, this time always on the whole image
        rgb_tripel tmp2 = simulate_whitebalance(data, wb, false);
        *resulting_brightness = calc_brightness_from_clr_avg( tmp2.R, tmp2.G, tmp2.B );

        if (wb_auto_step(&tmp, wb))
        {
            return true;
        }
    }
    wb->R = clip( wb->R, WB_MAX );
    wb->G = clip( wb->G, WB_MAX );
    wb->B = clip( wb->B, WB_MAX );

    return false;
}


bool auto_whitebalance_cam (const auto_sample_points* data, rgb_tripel* wb)
{
    rgb_tripel old_wb = *wb;

    if (wb->R < WB_IDENTITY)
        wb->R = WB_IDENTITY;
    if (wb->G < WB_IDENTITY)
        wb->G = WB_IDENTITY;
    if (wb->B < WB_IDENTITY)
        wb->B = WB_IDENTITY;
    if (old_wb.R != wb->R || old_wb.G != wb->G || old_wb.B != wb->B)
        return false;

    while ((wb->R > WB_IDENTITY) && (wb->G > WB_IDENTITY) && (wb->B > WB_IDENTITY))
    {
        wb->R -= 1;
        wb->G -= 1;
        wb->B -= 1;
    }

    rgb_tripel averageColor = average_color_cam( data);
    if(wb_auto_step(&averageColor, wb ) )
    {
        return true;
    }

    wb->R = clip( wb->R, WB_MAX );
    wb->G = clip( wb->G, WB_MAX );
    wb->B = clip( wb->B, WB_MAX );

    return false;
}


byte wb_pixel_c (byte pixel, byte wb_r, byte wb_g, byte wb_b, tBY8Pattern pattern)
{
    unsigned int val = pixel;
    switch (pattern)
    {
        case BG:
            val = (val * wb_b) / 64;
            break;
        case GB:
            val = (val * wb_g) / 64;
            break;
        case GR:
            val = (val * wb_g) / 64;
            break;
        case RG:
            val = (val * wb_r) / 64;
            break;
    };
    return ( val > 0xFF ? 0xFF : (byte)(val));
}


static void wb_line_c (byte* dest_line,
                       byte* src_line,
                       unsigned int dim_x,
                       byte wb_r, byte wb_g, byte wb_b,
                       tBY8Pattern pattern)
{
    const tBY8Pattern even_pattern = pattern;
    const tBY8Pattern odd_pattern = next_pixel(pattern);
    uint x;
    for (x = 0; x < dim_x; x += 2)
    {
        unsigned int v0 = wb_pixel_c( src_line[x], wb_r, wb_g, wb_b,even_pattern );
        unsigned int v1 = wb_pixel_c( src_line[x+1], wb_r, wb_g, wb_b, odd_pattern );
        *((uint16_t*)(dest_line + x)) = (uint16_t)(v1 << 8 | v0);
    }

    if (x == (dim_x - 1))
    {
        dest_line[x] = wb_pixel_c( src_line[x], wb_r, wb_g, wb_b, even_pattern );
    }
}


static void	wb_image_c (struct wb_settings* settings)
{
    unsigned int* data = (unsigned int*)settings->data;

    unsigned int dim_x = settings->width;
    unsigned int dim_y = settings->height;

    unsigned int pitch = 8 * dim_x / 8;

    tBY8Pattern odd = next_line(settings->pattern);

    unsigned int y;
    for (y = 0 ; y < (dim_y - 1); y += 2)
    {
        byte* line0 = (byte*)data + y * pitch;
        byte* line1 = (byte*)data + (y + 1) * pitch;

        wb_line_c(line0, line0, dim_x,
                  settings->rgb.R, settings->rgb.G, settings->rgb.B,
                  settings->pattern);
        wb_line_c(line1, line1, dim_x,
                  settings->rgb.R, settings->rgb.G, settings->rgb.B,
                  odd);
    }

    if (y == (dim_y - 1))
    {
        byte* line = (byte*)data + y * pitch;
        wb_line_c(line, line, dim_x,
                  settings->rgb.R, settings->rgb.G, settings->rgb.B,
                  settings->pattern);
    }

}


void apply_wb_by8_c (struct wb_settings* settings)
{
    wb_image_c(settings);
}


void whitebalance_buffer (struct wb_settings* settings)
{
    if (settings == NULL)
    {
        return;
    }

    rgb_tripel rgb = settings->rgb;

    /* we prefer to set our own values */
    if (settings->auto_whitebalance == false)
    {
        rgb = settings->user_values;
    }
    else /* update the permanent values to represent the current adjustments */
    {
        auto_sample_points points = {};

        get_sampling_points (settings, &points);

        unsigned int resulting_brightness = 0;
        auto_whitebalance(&points, &rgb, &resulting_brightness);

        settings->rgb = rgb;
    }

    apply_wb_by8_c(settings);
}
