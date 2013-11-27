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

#ifndef _GST_TIS_AUTO_EXPOSURE_H_
#define _GST_TIS_AUTO_EXPOSURE_H_

#include <gst/base/gstbasetransform.h>
#include "bayer.h"

G_BEGIN_DECLS

#define GST_TYPE_TIS_AUTO_EXPOSURE            (gst_tis_auto_exposure_get_type())
#define GST_TIS_AUTO_EXPOSURE(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_TIS_AUTO_EXPOSURE, GstTis_Auto_Exposure))
#define GST_TIS_AUTO_EXPOSURE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_TIS_AUTO_EXPOSURE, GstTis_Auto_ExposureClass))
#define GST_IS_TIS_AUTO_EXPOSURE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_TIS_AUTO_EXPOSURE))
#define GST_IS_TIS_AUTO_EXPOSURE_CLASS(obj)   (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_TIS_AUTO_EXPOSURE))

/* helper structs */

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

typedef enum
{
    NETWORK,
    USB,
    UNKNOWN
} CameraType;


typedef unsigned char byte;

static const guint dist_mid = 100;
/* reference value for "optimal" brightness */
static const guint ref_val = 128;
static const gdouble steps_to_double_brightness = 3.0;

/* names of gstreamer elements used for camera interaction */
static const char* CAMERASRC_NETWORK = "GstAravis";
static const char* CAMERASRC_USB = "GstV4l2Src";


typedef struct GstTis_Auto_Exposure
{
    GstBaseTransform base_tis_auto_exposure;

    GstPad *sinkpad;
    GstPad *srcpad;

    gboolean auto_exposure;

    Gain gain;
    Exposure exposure;

    CameraType source_type;
    GstElement* camera_src;
    tBY8Pattern pattern;
    format color_format;

    gint frame_counter;

} GstTis_Auto_Exposure;

typedef struct GstTis_Auto_ExposureClass
{
    GstBaseTransformClass base_tis_auto_exposure_class;
} GstTis_Auto_ExposureClass;

GType gst_tis_auto_exposure_get_type (void);

G_END_DECLS

#endif
