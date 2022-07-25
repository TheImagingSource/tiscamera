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

#ifndef TCAM_GSTTCAMMAINSRC_H
#define TCAM_GSTTCAMMAINSRC_H

#include <gst/base/gstpushsrc.h>
#include <gst/gst.h>

G_BEGIN_DECLS

#define GST_TYPE_TCAM_MAINSRC (gst_tcam_mainsrc_get_type())
#define GST_TCAM_MAINSRC(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_TCAM_MAINSRC, GstTcamMainSrc))
#define GST_TCAM_MAINSRC_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_TCAM_MAINSRC, GstTcamMainSrc))
#define GST_IS_TCAM_MAINSRC(obj)       (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_TCAM_MAINSRC))
#define GST_IS_TCAM_MAINSRC_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_TCAM_MAINSRC))

typedef struct _GstTcamMainSrc GstTcamMainSrc;
typedef struct _GstTcamMainSrcClass GstTcamMainSrcClass;

struct device_state;


#define GST_TYPE_TCAM_IO_MODE (gst_tcam_io_mode_get_type())
GType gst_tcam_io_mode_get_type(void);

typedef enum
{
    GST_TCAM_IO_AUTO = 0,
    GST_TCAM_IO_MMAP = 1,
    GST_TCAM_IO_USERPTR = 2,
    //GST_TCAM_IO_DMABUF = 3,
    //GST_TCAM_IO_DMABUF_IMPORT = 4,
} GstTcamIOMode;

struct _GstTcamMainSrc
{
    GstPushSrc element;

    GstBufferPool* pool;
    device_state* device;

    gdouble fps;
};


struct _GstTcamMainSrcClass
{
    GstPushSrcClass parent_class;
};

GType gst_tcam_mainsrc_get_type(void);

GST_DEBUG_CATEGORY_EXTERN(tcam_mainsrc_debug);

G_END_DECLS

#endif /* TCAM_GSTTCAMMAINSRC_H */
