/*
 * Copyright 2022 The Imaging Source Europe GmbH
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

#pragma once

#include "gsttcammainsrc.h"
#include <gst/gst.h>

#include "mainsrc_device_state.h"

G_BEGIN_DECLS

typedef struct _GstTcamBufferPool GstTcamBufferPool;
typedef struct _GstTcamBufferPoolClass GstTcamBufferPoolClass;


#define GST_TYPE_TCAM_BUFFER_POOL       (gst_tcam_buffer_pool_get_type())
#define GST_IS_TCAM_BUFFER_POOL(obj)    (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_TCAM_BUFFER_POOL))
#define GST_TCAM_BUFFER_POOL(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_TCAM_BUFFER_POOL, GstTcamBufferPool))
#define GST_TCAM_BUFFER_POOL_CAST(obj)  ((GstTcamBufferPool*)(obj))

struct tcam_pool_state;

struct _GstTcamBufferPool
{
    GstBufferPool parent;

    GstBufferPool* other_pool_ = nullptr;

    GstElement* src_element = nullptr;

    tcam_pool_state* state_;

};

struct _GstTcamBufferPoolClass
{
    GstBufferPoolClass parent_class;
};

GType gst_tcam_buffer_pool_get_type();

GstBufferPool* gst_tcam_buffer_pool_new(GstElement* src, GstCaps* caps);

void gst_tcam_buffer_pool_set_other_pool(GstTcamBufferPool* pool, GstBufferPool* other_pool);

void gst_tcam_buffer_pool_delete_buffer(GstTcamBufferPool* self);

G_END_DECLS
