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

#ifndef _GST_TISWHITEBALANCE_H_
#define _GST_TISWHITEBALANCE_H_

#include <gst/base/gstbasetransform.h>
#include "bayer.h"
#include "base.h"


G_BEGIN_DECLS

#define GST_TYPE_TISWHITEBALANCE   (gst_tiswhitebalance_get_type())
#define GST_TISWHITEBALANCE(obj)   (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_TISWHITEBALANCE,GstTisWhiteBalance))
#define GST_TISWHITEBALANCE_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_TISWHITEBALANCE,GstTisWhiteBalanceClass))
#define GST_IS_TISWHITEBALANCE(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_TISWHITEBALANCE))
#define GST_IS_TISWHITEBALANCE_CLASS(obj)   (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_TISWHITEBALANCE))


static const guint MAX_STEPS = 20;
static guint WB_IDENTITY =  64;
static guint WB_MAX = 255;
static const guint BREAK_DIFF = 2;

const guint NEARGRAY_MIN_BRIGHTNESS      = 10;
guint NEARGRAY_MAX_BRIGHTNESS            = 253;
const float NEARGRAY_MAX_COLOR_DEVIATION = 0.25f;
const float NEARGRAY_REQUIRED_AMOUNT     = 0.08f;

/* rgb values have to be evaluated differently. These are the according factors */
static const guint r_factor = (guint32)((1 << 8) * 0.299f);
static const guint g_factor = (guint32)((1 << 8) * 0.587f);
static const guint b_factor = (guint32)((1 << 8) * 0.114f);

typedef unsigned char byte;

typedef struct
{
    GstBaseTransform base_tiswhitebalance;

    GstPad *sinkpad;
    GstPad *srcpad;

    /* user defined values */
    gint red;
    gint green;
    gint blue;

    tBY8Pattern pattern;

    /* persistent values */
    rgb_tripel rgb;

    gint height;
    gint width;

    gboolean auto_wb;
    gboolean auto_enabled;
	
	gboolean hardware_wb_enabled;

    struct device_resources res;

} GstTisWhiteBalance;

typedef struct
{
    GstBaseTransformClass base_tiswhitebalance_class;
} GstTisWhiteBalanceClass;

GType gst_tiswhitebalance_get_type (void);

G_END_DECLS

#endif
