
#pragma once

#include <string>
#include <gst-helper/helper_functions.h>

#include <gst/gst.h>

#include "gsttcambin.h"

#include "tcamgstbase/tcamgstbase.h"


struct tcambin_data
{
    std::string device_serial;
    std::string device_type;

    std::string state;

    gst_helper::gst_ptr<GstPad> target_pad;
    gst_helper::gst_ptr<GstCaps> user_caps;

    GstPad* src_ghost_pad =
        nullptr; // NOTE: we don't have a reference to this, so this is a observer that will be made invalid in dispose

    gst_helper::gst_ptr<GstElement> out_caps_filter_;

    gst_helper::gst_ptr<GstCaps> src_caps;
    gst_helper::gst_ptr<GstCaps> target_caps;


    // #TODO the lifetime of these is somewhat unclear to me, maybe look through this again
    GstElement* src = nullptr;
    GstElement* pipeline_caps = nullptr;
    GstElement* dutils = nullptr;
    GstElement* bayer_transform = nullptr;
    GstElement* jpegdec = nullptr;
    GstElement* convert = nullptr;
    GstElement* tcamconvert = nullptr;

    gboolean elements_created = FALSE;
    gboolean elements_linked = FALSE;
    gboolean target_set = FALSE;
    gboolean must_apply_state = FALSE;

    gboolean has_dutils = FALSE;

    tcam::gst::input_caps_required_modules modules;
    tcam::gst::input_caps_toggles toggles;
};


inline tcambin_data& get_tcambin_data(gpointer ptr)
{
    GstTcamBin* self = GST_TCAMBIN(ptr);

    return *self->data;
}
