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

#ifndef _IMAGE_SAMPLING_H_
#define _IMAGE_SAMPLING_H_

#include "bayer.h"
#include <gst/gst.h>
#include <glib.h>

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

    guint	cnt;
} auto_sample_points;

#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))


/**
 * @name get_sampling_points
 * @param buf - image buffer that shall be analyzed
 * @param points - sample points of the image
 * @param bayer pattern of image
 * @brief analyzes given buffer and fills sample points
*/
void get_sampling_points (GstBuffer* buf, auto_sample_points* points, tBY8Pattern pattern);

/**
 * @name image_brightness
 * @param buf - image buffer that shall be analyzed
 * @return guint containing the image brightness
 */
guint image_brightness_bayer (GstBuffer* buf, tBY8Pattern pattern);

/**
 * @name
 * @param
 * @return
 */
guint buffer_brightness_gray (GstBuffer* buf);

#endif /* _IMAGE_SAMPLING_H_ */
