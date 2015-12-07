/*
 * Copyright 2015 The Imaging Source Europe GmbH
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

#ifndef TIS_GSTREAMER010_BASE_H
#define TIS_GSTREAMER010_BASE_H

#include <gst/gst.h>
#include "bayer.h"


static const unsigned int V4L2_CID_EUVC_GAIN_R_OLD = 0x980921;
static const unsigned int V4L2_CID_EUVC_GAIN_G_OLD = 0x980922;
static const unsigned int V4L2_CID_EUVC_GAIN_B_OLD = 0x980923;
static const unsigned int V4L2_CID_EUVC_GAIN_R = 0x0199e921;
static const unsigned int V4L2_CID_EUVC_GAIN_G = 0x0199e922;
static const unsigned int V4L2_CID_EUVC_GAIN_B = 0x0199e923;


typedef struct
{
    gdouble min;
    gdouble max;
    gdouble value;

} Gain;


typedef struct
{
    gdouble min;
    gdouble max;
    gdouble value;

} Exposure;


enum GST_SOURCE_TYPE
{
    UNKNOWN = 0,
    V4L2,
    ARAVIS,
};


struct device_color
{

    gboolean has_whitebalance;
    gboolean has_auto_whitebalance;
    rgb_tripel rgb;
    gint max; // highest possible value. usually 255
    gint default_value; // Availalbe for V4l2. Aravis may not.
};


struct device_resources
{
    enum GST_SOURCE_TYPE source_type;
    GstElement* source_element;
    Gain gain;
    Exposure exposure;
    gdouble framerate;

    struct device_color color;
};


struct device_resources find_source (GstElement* self);

void update_device_resources (struct device_resources* res);

gboolean device_set_rgb (struct device_resources* res);


#endif /* TIS_GSTREAMER010_BASE_H */
