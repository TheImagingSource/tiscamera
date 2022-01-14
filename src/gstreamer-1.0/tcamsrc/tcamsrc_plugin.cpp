
#include "../../libtcam_base.h"
#include "../../version.h"
#include "../tcamgstbase/spdlog_gst_sink.h"
#include "gsttcammainsrc.h"
#include "gsttcamsrc.h"
#include "mainsrc_gst_device_provider.h"

GST_DEBUG_CATEGORY_STATIC(libtcam_category);

static void init_libtcam_spdlog_binding()
{
    libtcam::setup_default_logger();

    auto default_sink = tcam::gst::log::create_gst_sink(libtcam_category);

    auto logger = libtcam::get_spdlog_logger();
    logger->sinks().push_back(default_sink);

    // synchronize gst_debug_level and spdlog
    spdlog::default_logger()->set_level(tcam::gst::log::level_from_gst_debug_min());

    spdlog::set_default_logger(logger);

    libtcam::print_version_info_once();
}

static gboolean plugin_init(GstPlugin* plugin)
{
    gst_device_provider_register(
        plugin, "tcammainsrcdeviceprovider", GST_RANK_PRIMARY, TCAM_TYPE_MAINSRC_DEVICE_PROVIDER);
    gst_element_register(plugin, "tcamsrc", GST_RANK_PRIMARY, GST_TYPE_TCAM_SRC);
    gst_element_register(plugin, "tcammainsrc", GST_RANK_PRIMARY, GST_TYPE_TCAM_MAINSRC);


    GST_DEBUG_CATEGORY_INIT(
        libtcam_category, "tcam-libtcam", GST_DEBUG_BG_CYAN, "libtcam internals");

    init_libtcam_spdlog_binding();

    return TRUE;
}

#ifndef PACKAGE
#define PACKAGE "tcam"
#endif


GST_PLUGIN_DEFINE(GST_VERSION_MAJOR,
                  GST_VERSION_MINOR,
                  tcamsrc,
                  "TCam Video Source",
                  plugin_init,
                  get_version(),
                  "Proprietary",
                  "tcamsrc",
                  "theimagingsource.com")
