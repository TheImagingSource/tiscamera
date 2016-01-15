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

#ifndef TCAM_GSTTCAMSRC_H
#define TCAM_GSTTCAMSRC_H

#include <gst/gst.h>
#include <gst/base/gstpushsrc.h>

#include <girepository.h>

#ifdef __cplusplus
extern "C"
{
#endif


G_BEGIN_DECLS


#define GST_TYPE_TCAM           (gst_tcam_get_type())
#define GST_TCAM(obj)           (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_TCAM, GstTcam))
#define GST_TCAM_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_TCAM, GstTcam))
#define GST_IS_TCAM(obj)        (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_TCAM))
#define GST_IS_TCAM_CLASS(obj)  (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_TCAM))

typedef struct _GstTcam GstTcam;
typedef struct _GstTcamClass GstTcamClass;

struct _GstTcam
{
    GstPushSrc element;

    char* device_serial;

    void* device;

    int n_buffers;
    const struct tcam_image_buffer* ptr;
    gboolean new_buffer;
    gboolean is_running;
    int payload;

    int buffer_timeout_us;

    int run;

    GstCaps *all_caps;
    GstCaps *fixed_caps;

    guint64 timestamp_offset;
    guint64 last_timestamp;
};


struct _GstTcamClass
{
    GstPushSrcClass parent_class;
};

GType gst_tcam_get_type (void);

G_END_DECLS


#ifdef __cplusplus
}
#endif


#endif /* TCAM_GSTTCAMSRC_H */
