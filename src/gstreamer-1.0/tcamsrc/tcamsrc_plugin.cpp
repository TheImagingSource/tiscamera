
#include "../../error.h"
#include "../../libtcam_base.h"
#include "../../version.h"
#include "../tcamgstbase/spdlog_gst_sink.h"
#include "gsttcammainsrc.h"
#include "gsttcamsrc.h"
#include "mainsrc_gst_device_provider.h"

#include <tcamprop1.0_gobject/tcam_gerror.h>

GST_DEBUG_CATEGORY_STATIC(libtcam_category);

static TcamError to_TcamError(tcam::status status)
{
    switch (status)
    {
        case tcam::status::Success:
            return TCAM_ERROR_SUCCESS;
        case tcam::status::UndefinedError:
            return TCAM_ERROR_UNKNOWN;
        case tcam::status::Timeout:
            return TCAM_ERROR_TIMEOUT;
        case tcam::status::NotImplemented:
            return TCAM_ERROR_NOT_IMPLEMENTED;
        case tcam::status::InvalidParameter:
            return TCAM_ERROR_PARAMETER_INVALID;

        case tcam::status::DeviceCouldNotBeOpened:
            return TCAM_ERROR_DEVICE_NOT_OPENED;
        case tcam::status::DeviceLost:
            return TCAM_ERROR_DEVICE_LOST;
        case tcam::status::ResourceNotLockable:
            return TCAM_ERROR_DEVICE_LOST;
        case tcam::status::DeviceAccessBlocked:
            return TCAM_ERROR_DEVICE_NOT_ACCESSIBLE;

        case tcam::status::PropertyNotImplemented:
            return TCAM_ERROR_PROPERTY_NOT_IMPLEMENTED;
        case tcam::status::PropertyValueOutOfBounds:
            return TCAM_ERROR_PROPERTY_VALUE_OUT_OF_RANGE;
        case tcam::status::PropertyNotWriteable:
            return TCAM_ERROR_PROPERTY_NOT_WRITEABLE;
        case tcam::status::PropertyNoDefaultAvailable:
            return TCAM_ERROR_PROPERTY_DEFAULT_NOT_AVAILABLE;

        case tcam::status::FormatInvalid:
            break;
    }
    return TCAM_ERROR_UNKNOWN;
}

static bool error_translator(GError** err, const std::error_code& errc)
{
    if (errc.category() != tcam::error_category())
    {
        return false;
    }

    auto err_id = to_TcamError(static_cast<tcam::status>(errc.value()));
    tcamprop1_gobj::set_gerror(err, err_id, errc.message());
    return true;
}

static void init_libtcam_spdlog_binding()
{
    tcamprop1_gobj::register_translator(&error_translator);
    libtcam::setup_default_logger();

    auto default_sink = tcam::gst::log::create_gst_sink(libtcam_category);

    auto logger = libtcam::get_spdlog_logger();
    logger->sinks().push_back(default_sink);

    spdlog::set_default_logger(logger);

    // update spdlog level using the current GST level
    spdlog::default_logger()->set_level(tcam::gst::log::level_from_gst_debug_min());

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
