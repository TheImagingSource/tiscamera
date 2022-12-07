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

#include "gstmetatcamstatistics.h"

#include <cstring>

GType tcam_statistics_meta_api_get_type(void)
{
    static GType type;
    static const gchar* tags[] = {"id", "val", NULL};

    if (g_once_init_enter(&type))
    {
        GType _type = gst_meta_api_type_register("TcamStatisticsMetaApi", tags);
        g_once_init_leave(&type, _type);
    }
    return type;
}


static gboolean tcam_statistics_meta_init(GstMeta* meta,
                                          gpointer /* params */,
                                          GstBuffer* /* buffer */)
{
    TcamStatisticsMeta* tcam = (TcamStatisticsMeta*)meta;

    tcam->structure = nullptr;
    //gst_structure_new_empty("TcamStatistics");

    return TRUE;
}


static gboolean tcam_statistics_meta_transform(GstBuffer* trans_buffer,
                                               GstMeta* meta,
                                               GstBuffer* /* buffer */,
                                               GQuark type,
                                               gpointer /* data */)
{
    g_return_val_if_fail(GST_IS_BUFFER(trans_buffer), FALSE);
    // g_return_val_if_fail(statistics, nullptr);

    // we always copy

    TcamStatisticsMeta* tcam = (TcamStatisticsMeta*)meta;

    if (GST_META_TRANSFORM_IS_COPY(type))
    {
        TcamStatisticsMeta* trans_tcam = (TcamStatisticsMeta*)gst_buffer_add_meta(
            trans_buffer, TCAM_STATISTICS_META_INFO, nullptr);

        if (!trans_tcam)
        {
            return FALSE;
        }

        trans_tcam->structure = gst_structure_copy(tcam->structure);
    }
    return TRUE;
}


static void tcam_statistics_meta_free(GstMeta* meta, GstBuffer* /* buffer */)
{
    TcamStatisticsMeta* tcam = (TcamStatisticsMeta*)meta;

    if (tcam->structure)
    {
        gst_structure_free(tcam->structure);

        tcam->structure = nullptr;
    }
}


const GstMetaInfo* tcam_statistics_meta_get_info(void)
{
    static const GstMetaInfo* meta_info = nullptr;

    if (g_once_init_enter(&meta_info))
    {
        const GstMetaInfo* mi = gst_meta_register(TCAM_STATISTICS_META_API_TYPE,
                                                  "TcamStatisticsMeta",
                                                  sizeof(TcamStatisticsMeta),
                                                  tcam_statistics_meta_init,
                                                  tcam_statistics_meta_free,
                                                  tcam_statistics_meta_transform);
        g_once_init_leave(&meta_info, mi);
    }

    return meta_info;
}


TcamStatisticsMeta* gst_buffer_add_tcam_statistics_meta(GstBuffer* buffer, GstStructure* statistics)
{

    g_return_val_if_fail(GST_IS_BUFFER(buffer), nullptr);
    g_return_val_if_fail(statistics, nullptr);


    TcamStatisticsMeta* meta =
        (TcamStatisticsMeta*)gst_buffer_add_meta(buffer, TCAM_STATISTICS_META_INFO, nullptr);

    if (!meta)
    {
        return nullptr;
    }

    meta->structure = statistics;

    return meta;
}


gboolean tcam_statistics_get_structure(TcamStatisticsMeta* meta, char* out_buffer, size_t out_buffer_size)
{
    if (!meta || !out_buffer)
    {
        return FALSE;
    }

    char* tmp =  gst_structure_to_string(meta->structure);

    if (strlen(tmp) >= out_buffer_size)
    {
        g_free(tmp);
        return FALSE;
    }

    if (out_buffer_size > 0)
    {
        out_buffer[0] = '\0';
        strncat(out_buffer, tmp, out_buffer_size - 1);
    }

    g_free(tmp);

    return TRUE;
}
