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

#ifndef _GST_TCAMAUTOFOCUS_H_
#define _GST_TCAMAUTOFOCUS_H_

#include "algorithms/tcam-algorithm.h"

#include <gst/base/gstbasetransform.h>

#ifdef __cplusplus
extern "C"
{
#endif

    G_BEGIN_DECLS

#define GST_TYPE_TCAMAUTOFOCUS (gst_tcamautofocus_get_type())
#define GST_TCAMAUTOFOCUS(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_TCAMAUTOFOCUS, GstTcamAutoFocus))
#define GST_TCAMAUTOFOCUS_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_TCAMAUTOFOCUS, GstTcamAutoFocusClass))
#define GST_IS_TCAMAUTOFOCUS(obj)       (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_TCAMAUTOFOCUS))
#define GST_IS_TCAMAUTOFOCUS_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_TCAMAUTOFOCUS))


    typedef unsigned char byte;


    /* names of gstreamer elements used for camera interaction */
    /* static const char* CAMERASRC_NETWORK = "GstAravis"; */

    typedef struct GstTcamAutoFocus
    {
        GstBaseTransform base_tis_auto_exposure;

        gint image_width;
        gint image_height;

        gboolean focus_active;

        GstElement* camera_src;
        tcam::algorithms::focus::AutoFocus* focus;

        gint cur_focus;

        gint roi_left;
        gint roi_top;
        gint roi_width;
        gint roi_height;

        tcam::algorithms::roi::ROI* roi;

        tcam_video_format fmt;

        gboolean init_focus;
        auto_alg::auto_focus_params params;
    } GstTcamAutoFocus;

    typedef struct GstTcamAutoFocusClass
    {
        GstBaseTransformClass base_tcamautofocus_class;
    } GstTcamAutoFocusClass;

    GType gst_tcamautofocus_get_type(void);

    G_END_DECLS

#ifdef __cplusplus
}
#endif

#endif /* _GST_TCAMAUTOFOCUS_H_ */
