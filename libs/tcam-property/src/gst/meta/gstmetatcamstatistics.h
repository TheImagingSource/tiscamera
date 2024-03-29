/*
 * Copyright 2018 The Imaging Source Europe GmbH
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


#ifndef GST_META_TCAM_STATISTICS_H
#define GST_META_TCAM_STATISTICS_H


#include <glib/gstdio.h>
#include <gst/gst.h>
#include <gst/video/video.h>

_Pragma("GCC visibility push (default)")

#if __cplusplus
extern "C" {
#endif

G_BEGIN_DECLS

typedef struct _GstMetaTcamStatistics TcamStatisticsMeta;

struct _GstMetaTcamStatistics
{
    GstMeta meta;

    GstStructure* structure;
};

// registering out metadata API definition
GType tcam_statistics_meta_api_get_type(void);
#define TCAM_STATISTICS_META_API_TYPE (tcam_statistics_meta_api_get_type())

// finds and returns the metadata
#define gst_buffer_get_tcam_statistics_meta(b) \
    ((TcamStatisticsMeta*)gst_buffer_get_meta((b), TCAM_STATISTICS_META_API_TYPE))


const GstMetaInfo* tcam_statistics_meta_get_info(void);
#define TCAM_STATISTICS_META_INFO (tcam_statistics_meta_get_info())

TcamStatisticsMeta* gst_buffer_add_tcam_statistics_meta(GstBuffer* buffer,
                                                        GstStructure* statistics);

gboolean tcam_statistics_get_structure(TcamStatisticsMeta*, char* out_buffer, size_t out_buffer_size);

G_END_DECLS

#if __cplusplus
} // extern "C"
#endif

_Pragma("GCC visibility pop")

#endif /* GST_META_TCAM_STATISTICS_H */
