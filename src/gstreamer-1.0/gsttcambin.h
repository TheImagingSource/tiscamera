/*
 * Copyright 2016 The Imaging Source Europe GmbH
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

#ifndef TCAM_GSTTCAMBIN_H
#define TCAM_GSTTCAMBIN_H

#include "tcamgstbase/tcamgstbase.h"

#include <girepository.h>
#include <gst/gst.h>

G_BEGIN_DECLS


#define GST_TYPE_TCAMBIN          (gst_tcambin_get_type())
#define GST_TCAMBIN(obj)          (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_TCAMBIN, GstTcamBin))
#define GST_TCAMBIN_CLASS(klass)  (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_TCAMBIN, GstTcamBin))
#define GST_IS_TCAMBIN(obj)       (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_TCAMBIN))
#define GST_IS_TCAMBIN_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_TCAMBIN))


typedef struct _GstTcamBin GstTcamBin;
typedef struct _GstTcamBinClass GstTcamBinClass;

struct tcambin_data;

struct _GstTcamBin
{
    GstBin parent;

    tcambin_data* data;

    // #TODO the lifetime of these is somewhat unclear to me, maybe look through this again
    GstElement* src;
    GstElement* pipeline_caps;
    GstElement* dutils;
    GstElement* bayer_transform;
    GstElement* debayer;
    GstElement* jpegdec;
    GstElement* convert;
    GstElement* tcamconvert;

    gboolean elements_created;
    gboolean elements_linked;
    gboolean target_set;
    gboolean must_apply_state;

    gboolean has_dutils;

    struct tcam::gst::input_caps_required_modules modules;
    struct tcam::gst::input_caps_toggles toggles;
};


struct _GstTcamBinClass
{
    GstBinClass parent_class;
};

GType gst_tcambin_get_type(void);

G_END_DECLS

#endif /* TCAM_GSTTCAMBIN_H */
