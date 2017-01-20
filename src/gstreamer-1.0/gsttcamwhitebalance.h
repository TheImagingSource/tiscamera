/*
 * Copyright 2014 The Imaging Source Europe GmbH
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

#ifndef __GST_TCAMWHITEBALANCE_H__
#define __GST_TCAMWHITEBALANCE_H__

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include <gst/gstbuffer.h>

#include "algorithms/tcam-algorithm.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define GST_TYPE_TCAMWHITEBALANCE            (gst_tcamwhitebalance_get_type())
#define GST_TCAMWHITEBALANCE(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_TCAMWHITEBALANCE,GstTcamWhitebalance))
#define GST_TCAMWHITEBALANCE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_TCAMWHITEBALANCE,GstTcamWhitebalanceClass))
#define GST_IS_TCAMWHITEBALANCE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_TCAMWHITEBALANCE))
#define GST_IS_TCAMWHITEBALANCE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_TCAMWHITEBALANCE))

typedef struct _GstTcamWhitebalance GstTcamWhitebalance;
typedef struct _GstTcamWhitebalanceClass GstTcamWhitebalanceClass;


static const guint MAX_STEPS = 20;
static guint WB_IDENTITY = 64;
static guint WB_MAX = 255;
static const guint BREAK_DIFF = 2;

const guint NEARGRAY_MIN_BRIGHTNESS      = 10;
const guint NEARGRAY_MAX_BRIGHTNESS      = 253;
const float NEARGRAY_MAX_COLOR_DEVIATION = 0.25f;
const float NEARGRAY_REQUIRED_AMOUNT     = 0.08f;

/* rgb values have to be evaluated differently. These are the according factors */
static const guint r_factor = (guint32)((1 << 8) * 0.299f);
static const guint g_factor = (guint32)((1 << 8) * 0.587f);
static const guint b_factor = (guint32)((1 << 8) * 0.114f);

typedef unsigned char byte;


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
    GstElement* source_element;
    Gain gain;
    Exposure exposure;
    gdouble framerate;

    struct device_color color;
};

struct _GstTcamWhitebalance {
    GstBaseTransform base_object;

    GstPad        *srcpad;
    GstPad        *sinkpad1;

    gst_tcam_image_size image_size;
    gdouble        framerate;
    tBY8Pattern    pattern;
    guint expected_buffer_size;


    /* user defined values */
    guint red;
    guint green;
    guint blue;

    /* persistent values */
    rgb_tripel rgb;
    gboolean auto_wb;
    gboolean auto_enabled;
    gboolean force_hardware_wb;
    struct device_resources res;
};

struct _GstTcamWhitebalanceClass {
    GstBaseTransformClass gstbasetransform_class;
};

GType gst_tcamwhitebalance_get_type (void);

#ifdef __cplusplus
}
#endif

#endif /* __GST_TCAMWHITEBALANCE_H__ */
