
#include "../../version.h"
#include "gsttcammainsrc.h"
#include "gsttcamsrc.h"
#include "mainsrc_gst_device_provider.h"

static gboolean plugin_init(GstPlugin* plugin)
{
    gst_device_provider_register(
        plugin, "tcammainsrcdeviceprovider", GST_RANK_PRIMARY, TCAM_TYPE_MAINSRC_DEVICE_PROVIDER);
    gst_element_register(plugin, "tcamsrc", GST_RANK_PRIMARY, GST_TYPE_TCAM_SRC);
    gst_element_register(plugin, "tcammainsrc", GST_RANK_PRIMARY, GST_TYPE_TCAM_MAINSRC);

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
