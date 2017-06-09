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

#ifndef TCAM_ALGORITHM_IMAGE_SAMPLING_H
#define TCAM_ALGORITHM_IMAGE_SAMPLING_H

#include "bayer.h"

#ifdef __cplusplus
extern "C"
{
#endif


/* number of sample points */
#define SAMPLING_LINES      30
#define SAMPLING_COLUMNS    40
#define SAMPLING_MIN_WIDTH  8
#define SAMPLING_MIN_HEIGHT 8

typedef unsigned char byte;

typedef struct auto_sample_points
{
    struct pixel
    {
        byte r;
        byte g;
        byte b;
    } samples[1500];

    unsigned int cnt;
} auto_sample_points;


typedef struct gst_tcam_image_size
{
    unsigned int width;
    unsigned int height;
} gst_tcam_image_size;


typedef struct
{
    byte* image;
    unsigned int width;
    unsigned int height;
    unsigned int rowstride;
    format color_format;
    tBY8Pattern pattern;
} image_buffer;


#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))


/**
 * @name get_sampling_points
 * @param buf - image buffer that shall be analyzed
 * @param points - sample points of the image
 * @param bayer pattern of image
 * @brief analyzes given buffer and fills sample points
*/
void get_sampling_points (unsigned char* buf,
                          auto_sample_points* points,
                          tBY8Pattern pattern,
                          gst_tcam_image_size size);

void get_sampling_points_from_buffer (image_buffer* buf,
                                      auto_sample_points* points);

void get_sampling_points_from_buffer (image_buffer* buf,
                                      auto_sample_points* points);
/**
 * @name image_brightness
 * @param buf - image buffer that shall be analyzed
 * @return guint containing the image brightness
 */
unsigned int image_brightness_bayer (image_buffer* buf);

/**
 * @name
 * @param
 * @return
 */
unsigned int buffer_brightness_gray (image_buffer* buf);

#ifdef __cplusplus
}
#endif

#endif /* TCAM_ALGORITHM_IMAGE_SAMPLING_H */
