
#pragma once

#include "../tcamgstbase/tcambinconversion.h"
#include "../tcamgstbase/tcamgstbase.h"
#include "gsttcambin.h"

#include <gst-helper/helper_functions.h>
#include <gst/gst.h>
#include <string>


struct tcambin_conversion
{
    TcamBinConversionElement user_selector = TCAM_BIN_CONVERSION_AUTO;
    TcamBinConversionElement selected_conversion = TCAM_BIN_CONVERSION_AUTO;
};


struct tcambin_data
{
    // source init variables, used only in tcambin_create_source
    // if tcambin_create_source terminated successfully, these are reset
    std::string device_serial;
    std::string device_type;
    gst_helper::gst_ptr<GstDevice> prop_tcam_device;

    gst_helper::gst_ptr<GstStructure> prop_init_;
    std::string prop_init_json_;
    //

    gst_helper::gst_ptr<GstPad> target_pad;
    gst_helper::gst_ptr<GstCaps> user_caps;

    // NOTE: we don't have a reference to this,
    // so this is a observer that will be made invalid in dispose
    GstPad* src_ghost_pad = nullptr;

    gst_helper::gst_ptr<GstElement> out_caps_filter_;

    gst_helper::gst_ptr<GstCaps> src_caps;
    gst_helper::gst_ptr<GstCaps> available_caps;
    gst_helper::gst_ptr<GstCaps> target_caps;

    // #TODO the lifetime of these is somewhat unclear to me, maybe look through this again
    gst_helper::gst_ptr<GstElement> src_element;
    GstElement* pipeline_caps = nullptr;
    GstElement* jpegdec = nullptr;
    GstElement* videoconvert = nullptr;
    GstElement* tcam_converter = nullptr;

    tcambin_conversion conversion_info = {};

    bool elements_created = false;
    bool target_set = false;

    bool is_open() const noexcept
    {
        return elements_created;
    }

    tcam::gst::input_caps_required_modules modules;
};


inline tcambin_data& get_tcambin_data(gpointer ptr)
{
    GstTcamBin* self = GST_TCAMBIN(ptr);

    return *self->data;
}
