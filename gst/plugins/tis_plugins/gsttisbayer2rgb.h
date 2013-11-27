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

#ifndef _GST_TISBAYER2RGB_H_
#define _GST_TISBAYER2RGB_H_

#include <gst/base/gstbasetransform.h>

G_BEGIN_DECLS

#define GST_TYPE_TISBAYER2RGB   (gst_tisbayer2rgb_get_type())
#define GST_TISBAYER2RGB(obj)   (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_TISBAYER2RGB,GstTisBayer2RGB))
#define GST_TISBAYER2RGB_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_TISBAYER2RGB,GstTisBayer2RGBClass))
#define GST_IS_TISBAYER2RGB(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_TISBAYER2RGB))
#define GST_IS_TISBAYER2RGB_CLASS(obj)   (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_TISBAYER2RGB))

typedef struct _GstTisBayer2RGB GstTisBayer2RGB;
typedef struct _GstTisBayer2RGBClass GstTisBayer2RGBClass;

struct _GstTisBayer2RGB
{
	GstBaseTransform base_tisbayer2rgb;

	GstPad *sinkpad;
	GstPad *srcpad;

	gint rgain;
	gint ggain;
	gint bgain;
	gboolean auto_gain;
};

struct _GstTisBayer2RGBClass
{
  GstBaseTransformClass base_tisbayer2rgb_class;
};

GType gst_tisbayer2rgb_get_type (void);

G_END_DECLS

#endif
