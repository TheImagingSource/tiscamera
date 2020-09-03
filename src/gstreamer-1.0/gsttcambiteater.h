/*
 * Copyright 2017 The Imaging Source Europe GmbH
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

#ifndef TCAM_GSTTCAMBITEATER_H
#define TCAM_GSTTCAMBITEATER_H

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>

#include "biteater.h"

#ifdef __cplusplus
extern "C"
{
#endif


G_BEGIN_DECLS

#define GST_TYPE_TCAMBITEATER            (gst_tcambiteater_get_type())
#define GST_TCAMBITEATER(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_TCAMBITEATER, GstTcamBitEater))
#define GST_TCAMBITEATER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_TCAMBITEATER, GstTcamBitEaterClass))
#define GST_IS_TCAMBITEATER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_TCAMBITEATER))
#define GST_IS_TCAMBITEATER_CLASS(obj)   (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_TCAMBITEATER))

typedef unsigned char byte;

typedef struct GstTcamBitEater
{
    GstBaseTransform base_tcambiteater;

    struct tcam::biteater::biteater_meta be_meta;

    tcam_image_buffer buffer_template_in;
    tcam_image_buffer buffer_template_out;

} GstTcamBitEater;

typedef struct GstTcamBitEaterClass
{
    GstBaseTransformClass base_tcambiteater_class;
} GstTcamBitEaterClass;

GType gst_tcambiteater_get_type (void);

G_END_DECLS

#ifdef __cplusplus
}
#endif

#endif /* TCAM_GSTTCAMBITEATER_H */
