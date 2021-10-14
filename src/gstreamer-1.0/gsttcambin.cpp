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

#include "gsttcambin.h"

#include "../../libs/tcamprop/src/tcam-property-1.0.h"
#include "../version.h"
#include "tcambin_tcamprop_impl.h"
#include "tcambin_data.h"
#include "tcamgstbase/tcamgstjson.h"
#include "tcamgstbase/tcamgstbase.h"

#include <cstring>
#include <gst-helper/helper_functions.h>
#include <string>
#include <unistd.h>

using namespace tcam::gst;

#define gst_tcambin_parent_class parent_class

GST_DEBUG_CATEGORY_STATIC(gst_tcambin_debug);
#define GST_CAT_DEFAULT gst_tcambin_debug


static gboolean gst_tcambin_create_source(GstTcamBin* self);
static gboolean gst_tcambin_create_elements(GstTcamBin* self);
static void gst_tcambin_clear_source(GstTcamBin* self);


G_DEFINE_TYPE_WITH_CODE(GstTcamBin,
                        gst_tcambin,
                        GST_TYPE_BIN,
                        G_IMPLEMENT_INTERFACE(TCAM_TYPE_PROPERTY_PROVIDER, tcam::gst::bin::gst_tcambin_tcamprop_init))



static void gst_tcambin_class_init(GstTcamBinClass* klass);
static void gst_tcambin_init(GstTcamBin* klass);

enum
{
    PROP_0,
    PROP_SERIAL,
    PROP_DEVICE_TYPE,
    PROP_DEVICE_CAPS,
    PROP_CONVERSION_ELEMENT,
    PROP_STATE
};

static GstStaticPadTemplate src_template =
    GST_STATIC_PAD_TEMPLATE("src", GST_PAD_SRC, GST_PAD_ALWAYS, GST_STATIC_CAPS_ANY);




static bool has_version_parity_with_tiscamera(const std::string& element_name)
{
    std::string version = get_plugin_version(element_name.c_str());

    std::string tcam_version = get_version_major();
    tcam_version += ".";
    tcam_version += get_version_minor();

    // Only check major.minor
    // everything else is potential bugfix
    if (version.find(tcam_version) == std::string::npos)
    {
        // do not post any messages here
        // element is not yet fully initialized
        // messages will _not_ be correctly propagated

        return false;
    }

    return true;
}



static gboolean gst_tcambin_create_source(GstTcamBin* self)
{
    gst_tcambin_clear_source(self);

    auto& data = get_tcambin_data(self);

    GST_DEBUG("Creating source...");

    data.src = gst_element_factory_make("tcamsrc", "tcambin-source");
    gst_bin_add(GST_BIN(self), data.src);

    if (!data.device_type.empty())
    {
        GST_INFO("Setting source type to %s", data.device_type.c_str());
        g_object_set(G_OBJECT(data.src), "type", data.device_type.c_str(), NULL);
    }

    if (!data.device_serial.empty())
    {
        GST_INFO("Setting source serial to %s", data.device_serial.c_str());
        g_object_set(G_OBJECT(data.src), "serial", data.device_serial.c_str(), NULL);
    }


    // set to READY so that caps are always readable
    gst_element_set_state(data.src, GST_STATE_READY);

    char* serial_from_src = nullptr;
    char* type_from_src = nullptr;
    // query these as late as possible
    // src needs some time as things can happen async
    g_object_get(G_OBJECT(data.src), "serial", &serial_from_src, "type", &type_from_src, NULL);
    data.device_serial = serial_from_src != nullptr ? serial_from_src : std::string();
    data.device_type = type_from_src != nullptr ? type_from_src : std::string();

    if (serial_from_src)
    {
        g_free(serial_from_src);
    }
    if (type_from_src)
    {
        g_free(type_from_src);
    }

    GST_INFO_OBJECT(self,
                    "Opened device has serial: '%s' type: '%s'",
                    data.device_serial.c_str(),
                    data.device_type.c_str());

    data.src_caps = gst_helper::query_caps(*gst_helper::get_static_pad(*data.src, "src"));
    // GST_INFO_OBJECT(
    //     self, "caps of src: %" GST_PTR_FORMAT, static_cast<void*>(data.src_caps.get()));

    return TRUE;
}


static void gst_tcambin_clear_source(GstTcamBin* self)
{
    auto& data = get_tcambin_data(self);

    if (data.src)
    {
        gst_element_set_state(data.src, GST_STATE_NULL);
        gst_bin_remove(GST_BIN(self), data.src);
        data.src = nullptr;
    }
}


static void gst_tcambin_clear_elements(GstTcamBin* self)
{
    auto& data = get_tcambin_data(self);

    auto remove_element = [=](GstElement** element) {
        if (!GST_IS_ELEMENT(*element))
        {
            return;
        }

        gst_element_set_state(*element, GST_STATE_NULL);
        gst_bin_remove(GST_BIN(self), *element);
        // not needed bin_remove automatically does that
        // gst_object_unref(element);
        *element = nullptr;
    };

    if (data.pipeline_caps)
    {
        gst_element_set_state(data.pipeline_caps, GST_STATE_NULL);
        if (data.src)
        {
            gst_element_unlink_pads(data.src, "src", data.pipeline_caps, "sink");
        }
        gst_bin_remove(GST_BIN(self), data.pipeline_caps);
        data.pipeline_caps = nullptr;
    }

    if (data.tcam_converter)
    {
        remove_element(&data.tcam_converter);
    }

    if (data.jpegdec)
    {
        remove_element(&data.jpegdec);
    }
    if (data.convert)
    {
        remove_element(&data.convert);
    }

    data.elements_created = false;
}


static gboolean create_and_add_element(GstElement** element,
                                       const char* factory_name,
                                       const char* element_name,
                                       GstBin* bin)
{
    *element = gst_element_factory_make(factory_name, element_name);
    if (*element)
    {
        GST_DEBUG("Adding %s(%p) to pipeline", factory_name, (void*)*element);
        gst_bin_add(bin, *element);
    }
    else
    {
        return FALSE;
    }
    return TRUE;
}


// helper function to post error messages to GstBus
static void send_linking_element_msg (GstTcamBin* self, const std::string& element_name)
{
    std::string msg_string = "Could not link element '" + element_name + "'.";
    GError* err =
        g_error_new(GST_CORE_ERROR, GST_CORE_ERROR_MISSING_PLUGIN, "%s", msg_string.c_str());
    GstMessage* msg = gst_message_new_error(GST_OBJECT(self), err, msg_string.c_str());
    gst_element_post_message(GST_ELEMENT(self), msg);
    g_error_free(err);
    GST_ERROR_OBJECT(self, "%s", msg_string.c_str());
}



// helper function to link elements
static bool link_elements (bool condition,
                           GstElement** previous_element,
                           GstElement** element,
                           std::string& pipeline_description,
                           const std::string& name)
{
    if (condition)
    {
        if (!*element)
        {
            return false;
        }

        gboolean link_ret = gst_element_link(*previous_element, *element);
        if (!link_ret)
        {
            return false;
        }

        pipeline_description += " ! ";
        pipeline_description += name;

        *previous_element = *element;
    }

    return true;
}

static gboolean gst_tcambin_create_elements(GstTcamBin* self)
{
    auto& data = get_tcambin_data(self);

    if (data.elements_created)
    {
        return TRUE;
    }

    gst_tcambin_clear_elements(self);

    GST_INFO("creating all potentially needed elements");

    if (data.src == nullptr)
    {
        gst_tcambin_create_source(self);
    }

    data.pipeline_caps = gst_element_factory_make("capsfilter", "tcambin-src_caps");

    if (data.pipeline_caps == nullptr)
    {
        GST_ERROR("Could not create internal pipeline caps. Aborting");
        return FALSE;
    }

    gst_bin_add(GST_BIN(self), data.pipeline_caps);

    auto send_missing_element_msg = [self](const std::string& element_name) {
        std::string msg_string = "Could not create element '" + element_name + "'.";
        GError* err =
            g_error_new_literal(GST_CORE_ERROR, GST_CORE_ERROR_MISSING_PLUGIN, msg_string.c_str());
        GstMessage* msg = gst_message_new_error(GST_OBJECT(self), err, msg_string.c_str());
        gst_element_post_message(GST_ELEMENT(self), msg);
        g_error_free(err);
        GST_ERROR("%s", msg_string.c_str());
    };

    if (data.conversion_info.selected_conversion == TCAM_BIN_CONVERSION_DUTILS)
    {
        if (!create_and_add_element(&data.tcam_converter, "tcamdutils", "tcambin-tcamdutils", GST_BIN(self)))
        {
            send_missing_element_msg("tcamdutils");
            return FALSE;
        }


    }
    else if (data.conversion_info.selected_conversion == TCAM_BIN_CONVERSION_CUDA)
    {
        if (!create_and_add_element(&data.tcam_converter, "tcamdutils", "tcambin-tcamdutils-cuda", GST_BIN(self)))
        {
            send_missing_element_msg("tcamdutils-cuda");
            return FALSE;
        }

    }
    else // default selection
    {
        if (!create_and_add_element(&data.tcam_converter, "tcamconvert", "tcambin-tcamconvert", GST_BIN(self)))
        {
            send_missing_element_msg("tcamconcert");
            return FALSE;
        }


    }



    if (contains_jpeg(data.src_caps.get()))
    {
        if (!create_and_add_element(&data.jpegdec, "jpegdec", "tcambin-jpegdec", GST_BIN(self)))
        {
            send_missing_element_msg("jpegdec");
            return FALSE;
        }
    }

    // this is needed to allow for conversions such as
    // GRAY8 to BGRx that can exist when device-caps are set
    if (!create_and_add_element(&data.convert, "videoconvert", "tcambin-videoconvert", GST_BIN(self)))
    {
        send_missing_element_msg("videoconvert");
        return FALSE;
    }

    data.elements_created = TRUE;

    return TRUE;
}


/**
 * Creating the required elements is responsibility of the caller
 */
static gboolean gst_tcambin_link_elements(GstTcamBin* self)
{
    GST_INFO("Linking elements");
    auto& data = get_tcambin_data(self);

    if (data.elements_linked)
    {
        GST_INFO("Already linked");
        return TRUE;
    }

    if (data.target_caps == nullptr)
    {
        GST_ERROR("Unknown target caps. Aborting.");
        return FALSE;
    }

    if (data.pipeline_caps == nullptr)
    {
        GST_ERROR("Could not create internal pipeline caps. Aborting");
        return FALSE;
    }

    if (data.src_caps == nullptr)
    {
        GST_ERROR("Could not find valid caps. Aborting pipeline creation.");
        return FALSE;
    }

    if (!gst_caps_is_fixed(data.src_caps.get()))
    {
        std::string sc = gst_helper::to_string(*data.src_caps);
        GstCaps* tmp = tcam_gst_find_largest_caps(data.src_caps.get());
        GST_INFO("Caps were not fixed. Reduced '%s' to: '%s'",
                 sc.c_str(),
                 gst_helper::to_string(*tmp).c_str());

        if (tmp)
        {
            data.src_caps = gst_helper::make_wrap_ptr( tmp );
        }
        else
        {
            GST_WARNING("Unable to find largest caps. Continuing with unfixated caps.");
        }
    }
    else
    {
        GST_INFO("Caps are fixed. Using caps for src: %s",
                 gst_helper::to_string(*data.src_caps).c_str());
    }

    // explicitly destroy the pipeline caps
    // when having a start/stop cycle that goes PLAYING-READY-PLAYING, etc
    // the capsfilter sometimes refuses to correctly link again
    // by simply destroying it and creating a new one we work around this issue

    if (data.pipeline_caps)
    {
        gst_element_set_state(data.pipeline_caps, GST_STATE_NULL);
        gst_bin_remove(GST_BIN(self), data.pipeline_caps);
    }

    data.pipeline_caps = gst_element_factory_make("capsfilter", "tcambin-src_caps");

    if (data.pipeline_caps == nullptr)
    {
        GST_ERROR("Could not create internal pipeline caps. Aborting");
        return FALSE;
    }

    gst_bin_add(GST_BIN(self), data.pipeline_caps);

    // back to normal linking

    auto filter_caps = gst_caps_copy(data.src_caps.get());
    g_object_set(data.pipeline_caps, "caps", filter_caps, NULL);

    gboolean ret = gst_element_link(data.src, data.pipeline_caps);

    if (!ret)
    {
        GST_ERROR("Unable to link src and capsfilter.");
        return ret;
    }
    else
    {
        // gst_element_set_state(data.pipeline_caps, GST_STATE_READY);

        GstCaps* testcaps;
        g_object_get(data.pipeline_caps, "caps", &testcaps, NULL);

        GST_ERROR("!!!!!!!!!!!!!!!!!!!!!!!!! filter: %s", gst_caps_to_string(testcaps));
    }


    std::string pipeline_description = "tcamsrc ! ";
    pipeline_description += gst_helper::to_string(*data.src_caps);

    GstElement* previous_element = data.pipeline_caps;


    if (!link_elements(data.modules.dutils,
                       &previous_element,
                       &data.tcam_converter,
                       pipeline_description,
                       gst_element_get_name(data.tcam_converter)))
    {
        send_linking_element_msg(self, gst_element_get_name(data.tcam_converter));
        return FALSE;
    }


    if (!link_elements(data.modules.jpegdec,
                       &previous_element,
                       &data.jpegdec,
                       pipeline_description,
                       "jpegdec"))
    {
        send_linking_element_msg(self, "jpegdec");
        return FALSE;
    }

    // this is needed to allow for conversions such as
    // GRAY8 to BGRx that can exist when device-caps are set
    if (!link_elements(data.modules.videoconvert,
                       &previous_element,
                       &data.convert,
                       pipeline_description,
                       "videoconvert"))
    {
        send_linking_element_msg(self, "videoconvert");
        return FALSE;
    }

    // finished_element_linking:

    GST_INFO_OBJECT(self, "Internal pipeline of tcambin: %s", pipeline_description.c_str());
    data.target_pad = gst_helper::get_static_pad(*previous_element, "src");

    if (gst_helper::caps_empty_or_any(*data.target_caps))
    {
        data.target_caps = gst_helper::make_ptr(gst_caps_copy(data.src_caps.get()));
    }

    GST_INFO_OBJECT(
        self, "Working with exit caps: %s", gst_helper::to_string(*data.target_caps).c_str());
    data.elements_linked = TRUE;

    return TRUE;
}


/**
 * Generate GstCaps that contains all possible caps from src, bayer2rgb and videoconvert
 * in case of an error: return nullptr
 */
static GstCaps* generate_all_caps(GstTcamBin* self)
{
    auto& data = get_tcambin_data(self);

    auto in_pad = gst_helper::get_static_pad(*data.src, "src");
    GstCaps* incoming_caps = gst_pad_query_caps(in_pad.get(), NULL);


    GstCaps* all_caps = gst_caps_new_empty();

    for (guint i = 0; i < gst_caps_get_size(incoming_caps); ++i)
    {
        // nvmm is described not in the gst_caps name
        // but as a GstCapsFeatures object
        // iterate over them, if existent to skip memory types we do not handle.
        GstCapsFeatures* features = gst_caps_get_features(incoming_caps, i);

        if (features)
        {
            if (gst_caps_features_contains(features, "memory:NVMM"))
            {
                //GST_INFO("Contains NVMM. Skipping");
                continue;
            }
        }

        GstCaps* tmp = gst_caps_copy_nth(incoming_caps, i);

        // append does not copy but transferres
        gst_caps_append(all_caps, tmp);
    }

    // we have four scenarios:
    // 1. camera has video/x-raw,format=GRAY8 = passed through
    // 2. camera has video/x-bayer => bayer may be passed through or converted => bayer2rgb
    // 3. camera has video/x-raw,format=SOMETHING => passed through
    // 4. camera has video/x-raw((memory:NVMM)) => filter as we cannot handle the memory type

    // this boils down to:
    // if camera is mono,jpeg,yuv => pass through and be done
    // if camera has bayer => add video/x-raw,format{EVERYTHING} with the correct settings

    // TODO: rework outgoing available caps

    // GstCaps* to_remove = gst_caps_new_empty();

    // for (guint i = 0; i < gst_caps_get_size(all_caps); ++i)
    // {
    //     GstStructure* struc = gst_caps_get_structure(all_caps, i);

    //     if (gst_structure_get_field_type(struc, "format") == G_TYPE_STRING)
    //     {
    //         const char* string = gst_structure_get_string(struc, "format");

    //         if (tcam_gst_is_bayer8_string(string))
    //         {
    //             const GValue* width = gst_structure_get_value(struc, "width");
    //             const GValue* height = gst_structure_get_value(struc, "height");
    //             const GValue* framerate = gst_structure_get_value(struc, "framerate");

    //             GstStructure* s = gst_structure_new_empty("video/x-raw");

    //             GstCaps* tmp = get_caps_from_element_name("bayer2rgb", "src");

    //             GstStructure* tmp_struc = gst_structure_copy(gst_caps_get_structure(tmp, 0));
    //             gst_structure_set_value(s, "format", gst_structure_get_value(tmp_struc, "format"));

    //             gst_structure_set_value(s, "width", width);
    //             gst_structure_set_value(s, "height", height);
    //             gst_structure_set_value(s, "framerate", framerate);

    //             gst_caps_append_structure(all_caps, s);

    //             gst_caps_unref(tmp);
    //         }
    //         else if (!data.has_dutils
    //                  && (tcam_gst_is_bayer12_string(string)
    //                      || tcam_gst_is_bayer12_packed_string(string)
    //                      || tcam_gst_is_bayer16_string(string)))
    //         {
    //             gst_caps_append_structure(to_remove, gst_structure_copy(struc));
    //         }
    //     }
    // }

    gst_caps_unref(incoming_caps);

    // TODO: find alternative
    // caps_substract implicitly calls gst_caps_simplify
    // this causes 'weird' caps like video/x-raw,format=rggb,width={2448, 2048},height=2048
    // these are hard to parse and should be avoided.
    // all_caps = gst_caps_subtract(all_caps, to_remove);

    // gst_caps_unref(to_remove);


    GST_DEBUG("All caps: %s", gst_helper::to_string(*all_caps).c_str());

    return all_caps;
}


static void set_target_pad(GstTcamBin* self)
{
    auto& data = get_tcambin_data(self);

    gst_ghost_pad_set_target(GST_GHOST_PAD(data.src_ghost_pad), NULL);

    if (data.target_set == FALSE)
    {
        if (data.target_pad == nullptr)
        {
            GST_ERROR("target_pad not defined");
        }
        else
        {
            if (!gst_ghost_pad_set_target(GST_GHOST_PAD(data.src_ghost_pad), data.target_pad.get()))
            {
                GST_ERROR("Could not set target for ghostpad.");
            }
        }
        data.target_set = TRUE;
    }
}


static gboolean apply_state(GstTcamBin* self, const std::string& /*state*/)
{
    auto& data = get_tcambin_data(self);

    bool ret = true;
    if (data.device_serial.empty())
    {
//        ret = tcam::gst::load_device_settings(TCAM_PROPPROVIDER(self), "", state);
    }
    else
    {
        //      ret = tcam::gst::load_device_settings(TCAM_PROPPROVIDER(self), data.device_serial, state);
    }

    if (!ret)
    {
        GST_WARNING_OBJECT(self, "Device may be in an undefined state.");
    }

    return ret;
}


static void check_and_notify_version_missmatch (GstTcamBin* self)
{
    if (!self->data->tcam_converter)
    {
        // this means we either have a pipeline that does not
        // need one of our conversion elements
        // or that something is very, very wrong
        // either way, do not spam the user
        return;
    }

    if (self->data->conversion_info.user_selector != TCAM_BIN_CONVERSION_AUTO)
    {
        // additional anti spam check
        // != AUTO means the user has manually selected a conversion element
        // assume they know what they are doing
        return;
    }

    std::string element_name;

    if (self->data->conversion_info.selected_conversion == TCAM_BIN_CONVERSION_DUTILS)
    {
        if (has_version_parity_with_tiscamera("tcamdutils"))
        {
            return;
        }
        element_name = "tcamdutils";
    }
    else if (self->data->conversion_info.selected_conversion == TCAM_BIN_CONVERSION_CUDA)
    {
        if (has_version_parity_with_tiscamera("tcamdutils"))
        {
            return;
        }
        element_name = "tcamdutils";
    }
    else
    {
        // only other option is tcamconvert
        // which is part of tiscamera
        // version parity is thus guaranteed
        return;
    }

    std::string warning = element_name + " version mismatch! "
        + element_name + " and tiscamera require identical major.minor version. "
        "Overwrite at own risk by explicitly setting 'tcambin conversion-element=" + element_name + "'. "
        "Found '"
        + get_plugin_version(element_name.c_str()) + "' Required: '" + get_version_major() + "."
        + get_version_minor() + "'";

    GST_WARNING("%s", warning.c_str());

    std::string quark_str = element_name + " version mismatch";
    GError* err = g_error_new(g_quark_from_string(quark_str.c_str()),
                              1,
                              "%s",
                              GST_ELEMENT_NAME(self));

    GstMessage* msg = gst_message_new_warning(GST_OBJECT(self),
                                              err,
                                              warning.c_str());
    g_clear_error(&err);
    gst_element_post_message(GST_ELEMENT(self), msg);

}



static GstStateChangeReturn gst_tcam_bin_change_state(GstElement* element, GstStateChange trans)
{
    GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;

    GstTcamBin* self = GST_TCAMBIN(element);
    auto& data = get_tcambin_data(self);

    switch (trans)
    {
        case GST_STATE_CHANGE_NULL_TO_READY:
        {
            GST_DEBUG("NULL_TO_READY");

            // post message about dutils(-cuda) version missmatch
            // this only happens when we have found tcamdutils(-cuda)
            // but the previous version check failed
            // user can manually set use_dutils to true
            // which overwrites this message
            check_and_notify_version_missmatch(self);

            if (data.src == nullptr)
            {
                gst_tcambin_create_source(self);
            }

            gst_element_set_state(data.src, GST_STATE_READY);

            data.out_caps_filter_ = gst_helper::make_consume_ptr( gst_element_factory_make("capsfilter", "tcambin-out_caps") );

            gst_ghost_pad_set_target(
                GST_GHOST_PAD(data.src_ghost_pad),
                gst_helper::get_static_pad(*data.out_caps_filter_, "src").get());

            GstCaps* all_caps = generate_all_caps(self);
            g_object_set(data.out_caps_filter_.get(), "caps", all_caps, NULL);
            gst_caps_unref(all_caps);

            if (!data.elements_created)
            {
                if (!gst_tcambin_create_elements(self))
                {
                    GST_ERROR("Error while creating elements");
                }
            }
            break;
        }
        case GST_STATE_CHANGE_READY_TO_PAUSED:
        {
            GST_INFO("READY_TO_PAUSED");

            auto sinkpad = gst_helper::get_peer_pad(*data.src_ghost_pad);

            if (sinkpad == nullptr)
            {
                data.target_caps = gst_helper::make_ptr(gst_caps_new_empty());
            }
            else
            {
                GstElement* par = gst_pad_get_parent_element(sinkpad.get());

                if (strcmp(g_type_name(
                               gst_element_factory_get_element_type(gst_element_get_factory(par))),
                           "GstCapsFilter")
                    == 0)
                {
                    GstCaps* ptr = nullptr;
                    g_object_get(par, "caps", &ptr, NULL);

                    data.target_caps = gst_helper::make_ptr(ptr);
                }
                else
                {
                    data.target_caps = gst_helper::query_caps(*sinkpad);
                }
                gst_object_unref(par);
                GST_INFO_OBJECT(self,
                                "caps of sink: %" GST_PTR_FORMAT,
                                static_cast<void*>(data.target_caps.get()));
            }

            auto src_caps = gst_helper::query_caps(*gst_helper::get_static_pad(*data.src, "src"));

            if (data.user_caps)
            {
                GstCaps* tmp = gst_caps_intersect(data.user_caps.get(), src_caps.get());
                if (tmp == nullptr)
                {
                    GST_ERROR_OBJECT(self,
                                     "The user defined device caps are not supported by the "
                                     "device. User caps are: %s",
                                     gst_helper::to_string(*data.user_caps).c_str());
                    return GST_STATE_CHANGE_FAILURE;
                }

                data.user_caps = gst_helper::make_ptr(tmp);


                // Use the intersected caps instead of the user defined ones.
                // This allows us to work with valid device caps even when
                // the user defines caps like 'video/x-raw,format=BGR' because
                // they want to define the device format but not the resolution etc.
                GST_INFO_OBJECT(self,
                                "Using user defined caps for tcamsrc. User caps are: %s",
                                gst_helper::to_string(*data.user_caps).c_str());

                data.src_caps = gst_helper::make_ptr(find_input_caps(data.user_caps.get(),
                                                                     data.target_caps.get(),
                                                                     data.modules,
                                                                     data.conversion_info.selected_conversion));
            }
            else
            {
                data.src_caps = gst_helper::make_ptr(find_input_caps(src_caps.get(),
                                                                     data.target_caps.get(),
                                                                     data.modules,
                                                                     data.conversion_info.selected_conversion));
            }

            if (!data.src_caps || gst_caps_is_empty(data.src_caps.get()))
            {
                GST_ERROR_OBJECT(self,
                                 "Unable to work with given caps: %s",
                                 gst_helper::to_string(*data.target_caps).c_str());
                return GST_STATE_CHANGE_FAILURE;
            }

            if (!gst_tcambin_link_elements(self))
            {
                GST_ERROR("Unable to link elements");
                return GST_STATE_CHANGE_FAILURE;
            }
            if (data.pipeline_caps && data.src_caps)
            {

                // data.src_caps.reset(tcam_gst_find_largest_caps(data.src_caps.get()));
                // GST_ERROR("HERE");
                // g_object_set(data.pipeline_caps, "caps", data.src_caps.get(), NULL);
            }
            set_target_pad(self);

            /*
             * We send this message as a means of always notifying
             * applications of the output caps we use.
             * This is done for the case where no output caps are given
             * and the bin selected the caps that shall go out.
             * With this message applications have a way of displaying
             * the selected caps, no matter what.
             */

            gchar* caps_info_string = g_strdup_printf("Working with src caps: %s",
                                                      gst_helper::to_string(*data.src_caps).c_str());

            GstMessage* msg = gst_message_new_info(GST_OBJECT(element), nullptr, caps_info_string);
            g_free(caps_info_string);
            gst_element_post_message(element, msg);

            if (data.must_apply_state)
            {
                apply_state(self, data.state);
                data.must_apply_state = FALSE;
            }

            ret = GST_STATE_CHANGE_NO_PREROLL;
            break;
        }
        default:
        {
#if GST_VERSION_MINOR >= 14
            GST_INFO("Changing state: %s", gst_state_change_get_name(trans));
#endif
            break;
        }
    }

    gst_element_set_locked_state(element, TRUE);
    auto tmp_ret = GST_ELEMENT_CLASS(parent_class)->change_state(element, trans);
    gst_element_set_locked_state(element, FALSE);

    if (tmp_ret == GST_STATE_CHANGE_FAILURE)
    {
        return tmp_ret;
    }

    switch (trans)
    {
        case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
        {
            ret = GST_STATE_CHANGE_NO_PREROLL;
            break;
        }
        case GST_STATE_CHANGE_PAUSED_TO_READY:
        {
            data.target_set = FALSE;
            data.elements_linked = FALSE;

            data.src_caps.reset();

            break;
        }
        case GST_STATE_CHANGE_READY_TO_NULL:
        {
            gst_tcambin_clear_source(self);
            gst_tcambin_clear_elements(self);
            gst_ghost_pad_set_target(GST_GHOST_PAD(data.src_ghost_pad), NULL);
            break;
        }
        default:
        {
            break;
        }
    }

    return ret;
}


static void gst_tcambin_get_property(GObject* object,
                                     guint prop_id,
                                     GValue* value,
                                     GParamSpec* pspec)
{
    GstTcamBin* self = GST_TCAMBIN(object);
    auto& data = get_tcambin_data(self);

    switch (prop_id)
    {
        case PROP_SERIAL:
        {
            if (data.src)
            {
                g_object_get_property(G_OBJECT(data.src), "serial", value);
            }
            else
            {
                g_value_set_string(value, data.device_serial.c_str());
            }
            break;
        }
        case PROP_DEVICE_TYPE:
        {

            if (data.src)
            {
                g_object_get_property(G_OBJECT(data.src), "type", value);
            }
            else
            {
                g_value_set_string(value, data.device_type.c_str());
            }
            break;
        }
        case PROP_DEVICE_CAPS:
        {
            g_value_set_string(value, gst_helper::to_string(*data.user_caps).c_str());
            break;
        }
        case PROP_CONVERSION_ELEMENT:
        {
            g_value_set_enum(value, self->data->conversion_info.user_selector);
            break;
        }
        case PROP_STATE:
        {
            if (!data.elements_created)
            {
                //gst_tcambin_create_elements(GST_TCAMBIN(self));
            }
            if (!data.device_serial.empty())
            {
                // std::string bla =
                //     tcam::gst::create_device_settings(data.device_serial, TCAM_PROPPROVIDER(self)).c_str();
                // g_value_set_string(value, bla.c_str());
            }
            else
            {
                g_value_set_string(value, "");
            }
            break;
        }
        default:
        {
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
        }
    }
}


static void gst_tcambin_set_property(GObject* object,
                                     guint prop_id,
                                     const GValue* value,
                                     GParamSpec* pspec)
{
    GstTcamBin* self = GST_TCAMBIN(object);
    auto& data = get_tcambin_data(self);

    switch (prop_id)
    {
        case PROP_SERIAL:
        {
            data.device_serial =
                g_value_get_string(value) != nullptr ? g_value_get_string(value) : std::string();
            if (data.src != nullptr)
            {
                GST_INFO("Setting source serial to %s", data.device_serial.c_str());
                g_object_set_property(G_OBJECT(data.src), "serial", value);
            }

            break;
        }
        case PROP_DEVICE_TYPE:
        {
            data.device_type =
                g_value_get_string(value) != nullptr ? g_value_get_string(value) : std::string();
            if (data.src != nullptr)
            {
                GST_INFO("Setting source device type to %s", g_value_get_string(value));
                g_object_set_property(G_OBJECT(data.src), "type", value);
            }
            break;
        }
        case PROP_DEVICE_CAPS:
        {
            GST_WARNING("Setting DEVICE_CAPS %s", g_value_get_string(value));
            data.user_caps = gst_helper::make_ptr(gst_caps_from_string(g_value_get_string(value)));
            GST_INFO("%s", gst_caps_to_string(data.user_caps.get()));
            break;
        }
        case PROP_CONVERSION_ELEMENT:
        {
            self->data->conversion_info.user_selector = (TcamBinConversionElement)g_value_get_enum(value);

            if (self->data->conversion_info.user_selector == TCAM_BIN_CONVERSION_CONVERT)
            {
                if (self->data->conversion_info.have_tcamconvert)
                {
                    self->data->conversion_info.selected_conversion = TCAM_BIN_CONVERSION_CONVERT;
                }
                else
                {
                    GST_ERROR("Unable to use tcamconvert. Element does not seem to exist. Falling back to auto");
                    self->data->conversion_info.selected_conversion = TCAM_BIN_CONVERSION_AUTO;
                }
            }
            else if (self->data->conversion_info.user_selector == TCAM_BIN_CONVERSION_DUTILS)
            {
                if (self->data->conversion_info.have_tcamdutils)
                {
                    self->data->conversion_info.selected_conversion = TCAM_BIN_CONVERSION_DUTILS;
                }
                else
                {
                    GST_ERROR("Unable to use tcamdutils. Element does not seem to exist. Falling back to auto");
                    self->data->conversion_info.selected_conversion = TCAM_BIN_CONVERSION_AUTO;
                }
            }
            else if (self->data->conversion_info.user_selector == TCAM_BIN_CONVERSION_CUDA)
            {
                if (self->data->conversion_info.have_tcamdutils_cuda)
                {
                    self->data->conversion_info.selected_conversion = TCAM_BIN_CONVERSION_CUDA;
                }
                else
                {
                    GST_ERROR("Unable to use tcamconvert. Element does not seem to exist. Falling back to auto");
                    self->data->conversion_info.selected_conversion = TCAM_BIN_CONVERSION_AUTO;
                }
            }
            else
            {
                self->data->conversion_info.selected_conversion = TCAM_BIN_CONVERSION_AUTO;
            }

            break;
        }
        case PROP_STATE:
        {
            GstState gstate;

            gst_element_get_state(GST_ELEMENT(self), &gstate, nullptr, 1000000);

            if (gstate == GST_STATE_VOID_PENDING || gstate == GST_STATE_NULL
                || gstate == GST_STATE_READY || gstate == GST_STATE_PAUSED)
            {
                GST_INFO(
                    "tcambin not ready. State will be applied once GST_STATE_READY is reached.");
                data.must_apply_state = TRUE;
                data.state = g_value_get_string(value) != nullptr ? g_value_get_string(value) :
                                                                    std::string();
            }
            else
            {
                data.must_apply_state = FALSE;

                bool ret = apply_state(self, g_value_get_string(value));
                if (!ret)
                {
                    GST_WARNING("Device may be in an undefined state.");
                }
            }
            break;
        }
        default:
        {
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
        }
    }
}


static void gst_tcambin_init(GstTcamBin* self)
{
    GST_DEBUG_OBJECT(self, "init");

    self->data = new tcambin_data;

    auto& data = *self->data;

    data.elements_linked = FALSE;

    data.conversion_info.selected_conversion = TCAM_BIN_CONVERSION_CONVERT;

    auto factory = gst_element_factory_find("tcamdutils");

    if (factory != nullptr)
    {
        data.conversion_info.have_tcamdutils = true;
        gst_object_unref(factory);
        if (has_version_parity_with_tiscamera("tcamdutils"))
        {
            data.conversion_info.selected_conversion = TCAM_BIN_CONVERSION_DUTILS;
        }
    }

    auto factory_cuda = gst_element_factory_find("tcamdutils-cuda");

    if (factory_cuda != nullptr)
    {
        data.conversion_info.have_tcamdutils_cuda = true;
        gst_object_unref(factory_cuda);
        if (has_version_parity_with_tiscamera("tcamdutils-cuda"))
        {
            data.conversion_info.selected_conversion = TCAM_BIN_CONVERSION_CUDA;
        }
    }

    data.src = nullptr;
    data.pipeline_caps = nullptr;

    data.tcam_converter = nullptr;

    data.jpegdec = nullptr;
    data.convert = nullptr;

    // NOTE: the result of gst_ghost_pad_new_no_target is a floating reference that is consumer by gst_element_add_pad
    data.src_ghost_pad = gst_ghost_pad_new_no_target("src", GST_PAD_SRC);
    gst_element_add_pad(GST_ELEMENT(self), data.src_ghost_pad);

    GST_OBJECT_FLAG_SET(self, GST_ELEMENT_FLAG_SOURCE);
}

static void gst_tcambin_finalize(GObject* object)
{
    GstTcamBin* self = GST_TCAMBIN(object);

    delete self->data;

    G_OBJECT_CLASS(parent_class)->finalize(object);
}

static void gst_tcambin_dispose(GstTcamBin* self)
{
    GST_DEBUG("dispose");

    gst_tcambin_clear_source(self);
    gst_tcambin_clear_elements(self);

    if (self->data->src_ghost_pad)
    {
        gst_element_remove_pad(GST_ELEMENT(self),
                               self->data->src_ghost_pad); // this actually releases the ghost pad
        self->data->src_ghost_pad = nullptr;
    }

    G_OBJECT_CLASS(parent_class)->dispose((GObject*)self);
}


static void gst_tcambin_class_init(GstTcamBinClass* klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);
    GstElementClass* element_class = GST_ELEMENT_CLASS(klass);

    // enum is not a registered GType for ?reasons?
    // call explicitly to be sure
    tcam_bin_conversion_element_get_type();

    object_class->dispose = (GObjectFinalizeFunc)gst_tcambin_dispose;
    object_class->finalize = gst_tcambin_finalize;
    object_class->set_property = gst_tcambin_set_property;
    object_class->get_property = gst_tcambin_get_property;

    element_class->change_state = GST_DEBUG_FUNCPTR(gst_tcam_bin_change_state);

    g_object_class_install_property(
        object_class,
        PROP_SERIAL,
        g_param_spec_string("serial",
                            "Camera serial",
                            "Serial of the camera that shall be used",
                            "",
                            static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));


    g_object_class_install_property(
        object_class,
        PROP_DEVICE_TYPE,
        g_param_spec_string("type",
                            "Camera type",
                            "type/backend of the camera",
                            "auto",
                            static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));


    g_object_class_install_property(
        object_class,
        PROP_DEVICE_CAPS,
        g_param_spec_string("device-caps",
                            "Device Caps",
                            "GstCaps the tcamsrc shall use",
                            "",
                            static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property(
        object_class,
        PROP_CONVERSION_ELEMENT,
        g_param_spec_enum("conversion-element",
                          "conversion",
                          "Select used transformation element",
                          g_type_from_name("TcamBinConversionElement"),
                          TCAM_BIN_CONVERSION_AUTO,
                          static_cast<GParamFlags>(G_PARAM_READWRITE)));

    g_object_class_install_property(
        object_class,
        PROP_STATE,
        g_param_spec_string("state",
                            "Property State",
                            "Property values the internal elements shall use",
                            "",
                            static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));


    gst_element_class_add_pad_template(element_class, gst_static_pad_template_get(&src_template));

    gst_element_class_set_details_simple(element_class,
                                         "Tcam Video Bin",
                                         "Source/Video",
                                         "Tcam based bin",
                                         "The Imaging Source <support@theimagingsource.com>");
}


static gboolean plugin_init(GstPlugin* plugin)
{
    GST_DEBUG_CATEGORY_INIT(gst_tcambin_debug, "tcambin", 0, "TcamBin");

    return gst_element_register(plugin, "tcambin", GST_RANK_NONE, GST_TYPE_TCAMBIN);
}

#ifndef TCAMBIN_VERSION
#define TCAMBIN_VERSION "1.0.0"
#endif


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
