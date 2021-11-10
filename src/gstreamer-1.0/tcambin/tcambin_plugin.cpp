
/*
 * Copyright 2016 The Imaging Source Europe GmbH
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

#include "../../libtcam_base.h"
#include "../../version.h"
#include "gsttcambin.h"

GST_DEBUG_CATEGORY(gst_tcambin_debug);

static void init_libtcam_spdlog_binding()
{
    libtcam::setup_default_logger(); // setup logging if not yet done
    spdlog::set_default_logger(
        libtcam::get_spdlog_logger()); // attach our spdlog instance to the one in libtcam

    //Note: We don't do the gst sink dance here, because that is done in tcamsrc
}

static gboolean plugin_init(GstPlugin* plugin)
{
    GST_DEBUG_CATEGORY_INIT(gst_tcambin_debug, "tcambin", 0, "TcamBin");

    auto res = gst_element_register(plugin, "tcambin", GST_RANK_NONE, GST_TYPE_TCAMBIN);
    if (res)
    {
        init_libtcam_spdlog_binding();
    }
    return res;
}

#ifndef PACKAGE
#define PACKAGE "tcam"
#endif

GST_PLUGIN_DEFINE(GST_VERSION_MAJOR,
                  GST_VERSION_MINOR,
                  tcambin,
                  "Tcam Video Bin",
                  plugin_init,
                  get_version(),
                  "Proprietary",
                  "tcambin",
                  "theimagingsource.com")