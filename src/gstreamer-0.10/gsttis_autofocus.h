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

#ifndef _GST_TIS_AUTOFOCUS_H_
#define _GST_TIS_AUTOFOCUS_H_

#include <gst/base/gstbasetransform.h>
#include "AutoFocus.h"

G_BEGIN_DECLS

#define GST_TYPE_TIS_AUTOFOCUS            (gst_tis_autofocus_get_type())
#define GST_TIS_AUTOFOCUS(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_TIS_AUTOFOCUS, GstTis_AutoFocus))
#define GST_TIS_AUTOFOCUS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_TIS_AUTOFOCUS, GstTis_AutoFocusClass))
#define GST_IS_TIS_AUTOFOCUS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_TIS_AUTOFOCUS))
#define GST_IS_TIS_AUTOFOCUS_CLASS(obj)   (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_TIS_AUTOFOCUS))


typedef unsigned char byte;

enum CAMERA_TYPE
{
    CAMERA_TYPE_UNKNOWN = 0,
    CAMERA_TYPE_ARAVIS,
    CAMERA_TYPE_USB,
};


/* names of gstreamer elements used for camera interaction */
/* static const char* CAMERASRC_NETWORK = "GstAravis"; */

typedef struct GstTis_AutoFocus
{
    GstBaseTransform base_tis_auto_exposure;

    GstPad *sinkpad;
    GstPad *srcpad;

    enum CAMERA_TYPE camera_type;

    unsigned int width;
    unsigned int height;

    gboolean focus_active;

    GstElement* camera_src;
    AutoFocus* focus;

    guint cur_focus;
    guint x;
    guint y;
    guint size;


} GstTis_AutoFocus;

typedef struct GstTis_AutoFocusClass
{
    GstBaseTransformClass base_tis_autofocus_class;
} GstTis_AutoFocusClass;

GType gst_tis_autofocus_get_type (void);

G_END_DECLS

#endif /* _GST_TIS_AUTOFOCUS_H_ */
