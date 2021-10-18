
#pragma once

#include "gsttcambin.h"
#include "tcamgstbase/tcambinconversion.h"
#include "tcamgstbase/tcamgstbase.h"

#include <gst-helper/helper_functions.h>
#include <gst/gst.h>
#include <string>


struct tcambin_conversion
{
    bool have_tcamconvert = true;
    bool have_tcamdutils = false;
    bool have_tcamdutils_cuda = false;

    TcamBinConversionElement user_selector = TCAM_BIN_CONVERSION_AUTO;
    TcamBinConversionElement selected_conversion = TCAM_BIN_CONVERSION_AUTO;
};


struct tcambin_data
{
    std::string device_serial;
    std::string device_type;
    gst_helper::gst_ptr<GstDevice> prop_tcam_device;

    std::string state;

    gst_helper::gst_ptr<GstPad> target_pad;
    gst_helper::gst_ptr<GstCaps> user_caps;

    // NOTE: we don't have a reference to this,
    // so this is a observer that will be made invalid in dispose
    GstPad* src_ghost_pad = nullptr;

    gst_helper::gst_ptr<GstElement> out_caps_filter_;

    gst_helper::gst_ptr<GstCaps> src_caps;
    gst_helper::gst_ptr<GstCaps> target_caps;

    // #TODO the lifetime of these is somewhat unclear to me, maybe look through this again
    GstElement* src = nullptr;
    GstElement* pipeline_caps = nullptr;
    GstElement* jpegdec = nullptr;
    GstElement* convert = nullptr;

    tcambin_conversion conversion_info = {};
    GstElement* tcam_converter = nullptr;

    gboolean elements_created = FALSE;
    gboolean elements_linked = FALSE;
    gboolean target_set = FALSE;
    gboolean must_apply_state = FALSE;

    tcam::gst::input_caps_required_modules modules;
};


inline tcambin_data& get_tcambin_data(gpointer ptr)
{
    GstTcamBin* self = GST_TCAMBIN(ptr);

    return *self->data;
}
