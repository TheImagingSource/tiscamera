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

#include "../../base_types.h"

#include <gst/gst.h>
#include <string>

namespace tcamsrc
{
struct tcamsrc_state;
}

G_BEGIN_DECLS

#define GST_TYPE_TCAM_SRC          (gst_tcam_src_get_type())
#define GST_TCAM_SRC(obj)          (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_TCAM_SRC, GstTcamSrc))
#define GST_TCAM_SRC_CLASS(klass)  (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_TCAM_SRC, GstTcamSrc))
#define GST_IS_TCAM_SRC(obj)       (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_TCAM_SRC))
#define GST_IS_TCAM_SRC_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_TCAM_SRC))

typedef struct _GstTcamSrc GstTcamSrc;
typedef struct _GstTcamSrcClass GstTcamSrcClass;

struct _GstTcamSrc
{
    GstBin parent;

    tcamsrc::tcamsrc_state* state;
};


struct _GstTcamSrcClass
{
    GstBinClass parent_class;
};

GType gst_tcam_src_get_type(void);

G_END_DECLS

namespace tcamsrc
{
GstElement* get_active_source(GstTcamSrc*);
}


#endif /* TCAM_GSTTCAMSRC_H */
