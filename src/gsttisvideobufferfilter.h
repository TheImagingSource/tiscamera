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

#ifndef _GST_TISVIDEOBUFFERFILTER_H_
#define _GST_TISVIDEOBUFFERFILTER_H_

#include <gst/gstelement.h>

G_BEGIN_DECLS

#define GST_TYPE_TISVIDEOBUFFERFILTER   (gst_tisvideobufferfilter_get_type())
#define GST_TISVIDEOBUFFERFILTER(obj)   (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_TISVIDEOBUFFERFILTER,GstTisVideoBufferFilter))
#define GST_TISVIDEOBUFFERFILTER_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_TISVIDEOBUFFERFILTER,GstTisVideoBufferFilterClass))
#define GST_IS_TISVIDEOBUFFERFILTER(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_TISVIDEOBUFFERFILTER))
#define GST_IS_TISVIDEOBUFFERFILTER_CLASS(obj)   (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_TISVIDEOBUFFERFILTER))

typedef struct _GstTisVideoBufferFilter GstTisVideoBufferFilter;
typedef struct _GstTisVideoBufferFilterClass GstTisVideoBufferFilterClass;

struct _GstTisVideoBufferFilter
{
  GstElement element;

  GstPad *sinkpad;
  GstPad *srcpad;
};

struct _GstTisVideoBufferFilterClass
{
  GstElementClass element_class;
};

GType gst_tisvideobufferfilter_get_type (void);

G_END_DECLS

#endif
