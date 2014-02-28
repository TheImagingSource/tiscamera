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

#ifndef _GST_TISCOLORIZE_H_
#define _GST_TISCOLORIZE_H_

#include <gst/base/gstbasetransform.h>

G_BEGIN_DECLS

#define GST_TYPE_TISCOLORIZE   (gst_tiscolorize_get_type())
#define GST_TISCOLORIZE(obj)   (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_TISCOLORIZE,GstTisColorize))
#define GST_TISCOLORIZE_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_TISCOLORIZE,GstTisColorizeClass))
#define GST_IS_TISCOLORIZE(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_TISCOLORIZE))
#define GST_IS_TISCOLORIZE_CLASS(obj)   (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_TISCOLORIZE))

typedef struct _GstTisColorize GstTisColorize;
typedef struct _GstTisColorizeClass GstTisColorizeClass;

struct _GstTisColorize
{
	GstBaseTransform base_tiscolorize;

	int i;
	GstPad *sinkpad;
	GstPad *srcpad;
};

struct _GstTisColorizeClass
{
  GstBaseTransformClass base_tiscolorize_class;
};

GType gst_tiscolorize_get_type (void);

G_END_DECLS

#endif
