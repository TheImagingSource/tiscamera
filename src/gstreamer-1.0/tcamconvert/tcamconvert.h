/*
 * Copyright 2021 The Imaging Source Europe GmbH
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

#ifndef TCAMCONVERT_H_INC_
#define TCAMCONVERT_H_INC_

#include <gst/base/gstbasetransform.h>
#include <gst/gst.h>

namespace tcamconvert
{
class tcamconvert_context_base;
}

G_BEGIN_DECLS

#define GST_TYPE_TCAMCONVERT (gst_tcamconvert_get_type())
#define GST_TCAMCONVERT(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_TCAMCONVERT, GstTCamConvert))
#define GST_TCAMCONVERT_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_TCAMCONVERT, GstTCamConvertClass))
#define GST_IS_TCAMCONVERT(obj)       (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_TCAMCONVERT))
#define GST_IS_TCAMCONVERT_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_TCAMCONVERT))

typedef struct GstTCamConvert
{
    GstBaseTransform base;

    tcamconvert::tcamconvert_context_base* context_;

} GstTCamConvert;

typedef struct GstTCamConvertClass
{
    GstBaseTransformClass base_class;
} GstTCamConvertClass;

GType gst_tcamconvert_get_type(void);

G_END_DECLS

#endif /* TCAMCONVERT_H_INC_ */
