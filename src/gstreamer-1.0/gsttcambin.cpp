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

#include "../../libs/gst-helper/include/tcamprop1.0_gobject/tcam_property_serialize.h"
#include "../../libs/tcamprop/src/tcam-property-1.0.h"
#include "../version.h"
#include "tcambin_data.h"
#include "tcambin_tcamprop_impl.h"
#include "tcamgstbase/tcamgstbase.h"
#include "tcamgstbase/tcamgstjson.h"

#include <cstring>
#include <gst-helper/gst_gvalue_helper.h>
#include <gst-helper/helper_functions.h>
#include <string>
#include <unistd.h>

using namespace tcam::gst;

#define gst_tcambin_parent_class parent_class

GST_DEBUG_CATEGORY_STATIC(gst_tcambin_debug);
#define GST_CAT_DEFAULT gst_tcambin_debug

G_DEFINE_TYPE_WITH_CODE(GstTcamBin,
                        gst_tcambin,
                        GST_TYPE_BIN,
                        G_IMPLEMENT_INTERFACE(TCAM_TYPE_PROPERTY_PROVIDER,
                                              tcam::gst::bin::gst_tcambin_tcamprop_init))

static void gst_tcambin_clear_source(GstTcamBin* self);

enum
{
    PROP_0,
    PROP_SERIAL,
    PROP_DEVICE_TYPE,
    PROP_DEVICE_CAPS,
    PROP_AVAILABLE_CAPS,
    PROP_CONVERSION_ELEMENT,
    PROP_TCAM_PROPERTIES_JSON,
    PROP_TCAMDEVICE,
    PROP_TCAM_PROPERTIES_GSTSTRUCT,
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


static gst_helper::gst_ptr<GstCaps> tcambin_filter_unsupported_caps(const GstCaps& source_caps)
{

    auto filter_func =
        [](GstCapsFeatures* features, GstStructure* structure, gpointer /*user_data*/) -> gboolean
    {
        if (features)
        {
            if (gst_caps_features_contains(features, "memory:NVMM"))
            {
                return FALSE;
            }
        }

        if (gst_structure_has_field(structure, "format"))
        {
            auto format = gst_structure_get_string(structure, "format");

            if (g_strcmp0(format, "BGR") == 0
                || tcam::gst::format_is_yuv(gst_structure_get_name(structure), format))
            {
                return FALSE;
            }
        }

        return TRUE;
    };

    auto available_caps = gst_helper::gst_ptr<GstCaps>::wrap(gst_caps_copy(&source_caps));

    gst_caps_filter_and_map_in_place(available_caps.get(), filter_func, nullptr);

    return available_caps;
}

void gst_tcambin_apply_properties(GstTcamBin* self, const GstStructure& strct)
{
    tcamprop1_gobj::apply_properties(
        TCAM_PROPERTY_PROVIDER(self),
        strct,
        [self](const GError& err, const std::string& prop_name, const GValue*)
        {
            GST_WARNING_OBJECT(self,
                               "Failed to init property named '%s' due to: '%s'",
                               prop_name.c_str(),
                               err.message);
        });
}

static gboolean gst_tcambin_create_source(GstTcamBin* self)
{
    gst_tcambin_clear_source(self);

    auto& data = get_tcambin_data(self);

    GST_DEBUG("Creating source...");

    if (data.prop_tcam_device)
    {
        data.src = gst_device_create_element(data.prop_tcam_device.get(), "tcambin-source");
    }
    else
    {
        data.src = gst_element_factory_make("tcamsrc", "tcambin-source");

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
    }

    gst_bin_add(GST_BIN(self), data.src);

    // set to READY so that caps are always readable
    gst_element_set_state(data.src, GST_STATE_READY);

    data.device_serial = gst_helper::gobject_get_string(G_OBJECT(data.src), "serial");
    data.device_type =
        gst_helper::gobject_get_string_opt(G_OBJECT(data.src), "type").value_or(std::string {});

    GST_INFO_OBJECT(self,
                    "Opened device has serial: '%s' type: '%s'",
                    data.device_serial.c_str(),
                    data.device_type.c_str());

    auto src_caps = gst_helper::query_caps(*gst_helper::get_static_pad(*data.src, "src"));

    data.available_caps = tcambin_filter_unsupported_caps(*src_caps.get());

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

    auto remove_element = [=](GstElement** element)
    {
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
        data.tcam_converter = nullptr;
    }

    if (data.jpegdec)
    {
        remove_element(&data.jpegdec);
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
static void send_linking_element_msg(GstTcamBin* self, const std::string& element_name)
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
static bool link_elements(bool condition,
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
    }

    return true;
}

static gboolean gst_tcambin_create_elements(GstTcamBin* self)
{
    auto& data = get_tcambin_data(self);

    if (data.is_open())
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

    auto send_missing_element_msg = [self](const std::string& element_name)
    {
        std::string msg_string = "Could not create element '" + element_name + "'.";
        GError* err =
            g_error_new_literal(GST_CORE_ERROR, GST_CORE_ERROR_MISSING_PLUGIN, msg_string.c_str());
        GstMessage* msg = gst_message_new_error(GST_OBJECT(self), err, msg_string.c_str());
        gst_element_post_message(GST_ELEMENT(self), msg);
        g_error_free(err);
        GST_ERROR("%s", msg_string.c_str());
    };

    gst_bin_add(GST_BIN(self), data.pipeline_caps);

    std::string pipeline_string = "tcamsrc";

    if (gst_element_link(data.src, data.pipeline_caps))
    {
        pipeline_string += " ! capsfilter name=tcambin-src_caps";
    }
    else
    {
        send_missing_element_msg("capsfilter");
        return FALSE;
    }

    if (tcam::gst::contains_jpeg(self->data->available_caps.get()))
    {
        if (contains_jpeg(data.available_caps.get()))
        {
            if (!create_and_add_element(&data.jpegdec, "jpegdec", "tcambin-jpegdec", GST_BIN(self)))
            {
                send_missing_element_msg("jpegdec");
                return FALSE;
            }
        }
    }
    else
    {
        std::string element_name;
        if (data.conversion_info.selected_conversion == TCAM_BIN_CONVERSION_DUTILS)
        {
            if (!create_and_add_element(
                    &data.tcam_converter, "tcamdutils", "tcambin-tcamdutils", GST_BIN(self)))
            {
                send_missing_element_msg("tcamdutils");
                return FALSE;
            }
            element_name = "tcamdutils";
        }
        else if (data.conversion_info.selected_conversion == TCAM_BIN_CONVERSION_CUDA)
        {
            if (!create_and_add_element(
                    &data.tcam_converter, "tcamdutils", "tcambin-tcamdutils-cuda", GST_BIN(self)))
            {
                send_missing_element_msg("tcamdutils-cuda");
                return FALSE;
            }
            element_name = "tcamdutils-cuda";
        }
        else // default selection
        {
            if (!create_and_add_element(
                    &data.tcam_converter, "tcamconvert", "tcambin-tcamconvert", GST_BIN(self)))
            {
                send_missing_element_msg("tcamconvert");
                return FALSE;
            }
            element_name = "tcamconvert";
        }

        std::string bin_name = "tcambin-";
        bin_name += element_name;
        if (!link_elements(
                true, &data.pipeline_caps, &data.tcam_converter, pipeline_string, bin_name))
        {
            send_linking_element_msg(self, element_name);
            return false;
        }
    }
    data.target_pad = gst_helper::get_static_pad(*self->data->tcam_converter, "src");

    GST_DEBUG("Internal pipeline: %s", pipeline_string.c_str());

    data.elements_created = true;

    return TRUE;
}


static void set_target_pad(GstTcamBin* self)
{
    auto& data = get_tcambin_data(self);

    gst_ghost_pad_set_target(GST_GHOST_PAD(data.src_ghost_pad), NULL);

    if (data.target_set == false)
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
        data.target_set = true;
    }
}

static void check_and_notify_version_missmatch(GstTcamBin* self)
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
        if (has_version_parity_with_tiscamera("tcamdutils-cuda"))
        {
            return;
        }
        element_name = "tcamdutils-cuda";
    }
    else
    {
        // only other option is tcamconvert
        // which is part of tiscamera
        // version parity is thus guaranteed
        return;
    }

    std::string warning =
        element_name + " version mismatch! " + element_name
        + " and tiscamera require identical major.minor version. "
          "Overwrite at own risk by explicitly setting 'tcambin conversion-element="
        + element_name
        + "'. "
          "Found '"
        + get_plugin_version(element_name.c_str()) + "' Required: '" + get_version_major() + "."
        + get_version_minor() + "'";

    GST_WARNING("%s", warning.c_str());

    std::string quark_str = element_name + " version mismatch";
    GError* err =
        g_error_new(g_quark_from_string(quark_str.c_str()), 1, "%s", GST_ELEMENT_NAME(self));

    GstMessage* msg = gst_message_new_warning(GST_OBJECT(self), err, warning.c_str());
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

            gst_tcambin_create_elements(self);
            set_target_pad(self);

            break;
        }
        case GST_STATE_CHANGE_READY_TO_PAUSED:
        {
            GST_DEBUG("READY_TO_PAUSED");

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
                GstCaps* tmp =
                    gst_caps_intersect(data.user_caps.get(), self->data->available_caps.get());
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

                data.src_caps =
                    gst_helper::make_ptr(find_input_caps(data.user_caps.get(),
                                                         data.target_caps.get(),
                                                         data.modules,
                                                         data.conversion_info.selected_conversion));
            }
            else
            {
                data.src_caps =
                    gst_helper::make_ptr(find_input_caps(src_caps.get(),
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

            // apply caps to internal capsfilter
            // this applies the caps to tcamsrc
            g_object_set(self->data->pipeline_caps, "caps", self->data->src_caps.get(), NULL);

            /*
             * We send this message as a means of always notifying
             * applications of the output caps we use.
             * This is done for the case where no output caps are given
             * and the bin selected the caps that shall go out.
             * With this message applications have a way of displaying
             * the selected caps, no matter what.
             */

            gchar* caps_info_string = g_strdup_printf(
                "Working with src caps: %s", gst_helper::to_string(*data.src_caps).c_str());

            GstMessage* msg = gst_message_new_info(GST_OBJECT(element), nullptr, caps_info_string);
            g_free(caps_info_string);
            gst_element_post_message(element, msg);

            if (!data.prop_init_json_.empty())
            {
                tcam::gst::load_device_settings(TCAM_PROPERTY_PROVIDER(self), data.prop_init_json_);

                data.prop_init_json_ = {};
            }
            if (data.prop_init_)
            {
                gst_tcambin_apply_properties(self, *data.prop_init_);

                data.prop_init_.reset();
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
            data.target_set = false;

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
    auto& state = get_tcambin_data(self);

    switch (prop_id)
    {
        case PROP_SERIAL:
        {
            if (state.src)
            {
                g_object_get_property(G_OBJECT(state.src), "serial", value);
            }
            else
            {
                g_value_set_string(value, state.device_serial.c_str());
            }
            break;
        }
        case PROP_DEVICE_TYPE:
        {

            if (state.src)
            {
                g_object_get_property(G_OBJECT(state.src), "type", value);
            }
            else
            {
                g_value_set_string(value, state.device_type.c_str());
            }
            break;
        }
        case PROP_DEVICE_CAPS:
        {
            g_value_set_string(value, gst_helper::to_string(*state.user_caps).c_str());
            break;
        }
        case PROP_AVAILABLE_CAPS:
        {
            g_value_set_string(value, gst_helper::to_string(*state.available_caps).c_str());
            break;
        }
        case PROP_CONVERSION_ELEMENT:
        {
            g_value_set_enum(value, self->data->conversion_info.user_selector);
            break;
        }
        case PROP_TCAM_PROPERTIES_JSON:
        {
            if (!state.is_open())
            {
                g_value_set_string(value, "");
            }
            else
            {
                std::string tmp = tcam::gst::create_device_settings(TCAM_PROPERTY_PROVIDER(self));
                g_value_set_string(value, tmp.c_str());
            }
            break;
        }
        case PROP_TCAM_PROPERTIES_GSTSTRUCT:
        {
            gst_helper::gst_ptr<GstStructure> ptr;
            if (!state.is_open())
            {
                if (!state.prop_init_.empty())
                {
                    ptr = gst_helper::make_ptr(gst_structure_copy(state.prop_init_.get()));
                }
                else
                {
                    ptr = gst_helper::make_ptr(gst_structure_new_empty("tcam"));
                }
            }
            else
            {
                ptr = gst_helper::make_ptr(gst_structure_new_empty("tcam"));

                tcamprop1_gobj::serialize_properties(TCAM_PROPERTY_PROVIDER(self), *ptr);
            }
            gst_value_set_structure(value, ptr.get());
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
    auto& state = get_tcambin_data(self);

    switch (prop_id)
    {
        case PROP_SERIAL:
        {
            state.device_serial =
                g_value_get_string(value) != nullptr ? g_value_get_string(value) : std::string();
            if (state.src != nullptr)
            {
                GST_INFO("Setting source serial to %s", state.device_serial.c_str());
                g_object_set_property(G_OBJECT(state.src), "serial", value);
            }

            break;
        }
        case PROP_DEVICE_TYPE:
        {
            state.device_type =
                g_value_get_string(value) != nullptr ? g_value_get_string(value) : std::string();
            if (state.src != nullptr)
            {
                GST_INFO("Setting source device type to %s", g_value_get_string(value));
                g_object_set_property(G_OBJECT(state.src), "type", value);
            }
            break;
        }
        case PROP_DEVICE_CAPS:
        {
            GST_INFO("Setting device-caps to %s", g_value_get_string(value));
            state.user_caps = gst_helper::make_ptr(gst_caps_from_string(g_value_get_string(value)));
            break;
        }
        case PROP_CONVERSION_ELEMENT:
        {
            self->data->conversion_info.user_selector =
                (TcamBinConversionElement)g_value_get_enum(value);

            if (self->data->conversion_info.user_selector == TCAM_BIN_CONVERSION_CONVERT)
            {
                if (self->data->conversion_info.have_tcamconvert)
                {
                    self->data->conversion_info.selected_conversion = TCAM_BIN_CONVERSION_CONVERT;
                }
                else
                {
                    GST_ERROR("Unable to use tcamconvert. Element does not seem to exist. Falling "
                              "back to auto");
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
                    GST_ERROR("Unable to use tcamdutils. Element does not seem to exist. Falling "
                              "back to auto");
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
                    GST_ERROR("Unable to use tcamconvert. Element does not seem to exist. Falling "
                              "back to auto");
                    self->data->conversion_info.selected_conversion = TCAM_BIN_CONVERSION_AUTO;
                }
            }
            else
            {
                self->data->conversion_info.selected_conversion = TCAM_BIN_CONVERSION_AUTO;
            }

            break;
        }
        case PROP_TCAMDEVICE:
        {
            if (state.is_open())
            {
                GST_ERROR_OBJECT(
                    self, "The gobject property 'tcam-device' can only be set in GST_STATE_NULL");
                return;
            }

            auto ptr = GST_DEVICE(g_value_get_object(value));
            if (ptr == nullptr)
            {
                state.prop_tcam_device.reset();
            }
            else
            {
                state.prop_tcam_device = gst_helper::make_addref_ptr(ptr);
            }
            break;
        }
        case PROP_TCAM_PROPERTIES_JSON:
        {
            if (!state.is_open())
            {
                if (auto ptr = g_value_get_string(value); ptr != nullptr)
                {
                    state.prop_init_json_ = ptr;
                }
                else
                {
                    state.prop_init_json_ = {};
                }
            }
            else
            {
                if (auto ptr = g_value_get_string(value); ptr != nullptr)
                {
                    tcam::gst::load_device_settings(TCAM_PROPERTY_PROVIDER(self), ptr);
                }
            }
            break;
        }
        case PROP_TCAM_PROPERTIES_GSTSTRUCT:
        {
            auto strc = gst_value_get_structure(value);
            if (!state.is_open())
            {
                if (strc)
                {
                    state.prop_init_ = gst_helper::make_ptr(gst_structure_copy(strc));
                }
                else
                {
                    state.prop_init_.reset();
                }
            }
            else
            {
                if (strc)
                {
                    gst_tcambin_apply_properties(self, *strc);
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
    GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
    GstElementClass* element_class = GST_ELEMENT_CLASS(klass);

    // enum is not a registered GType for ?reasons?
    // call explicitly to be sure
    tcam_bin_conversion_element_get_type();

    gobject_class->dispose = (GObjectFinalizeFunc)gst_tcambin_dispose;
    gobject_class->finalize = gst_tcambin_finalize;
    gobject_class->set_property = gst_tcambin_set_property;
    gobject_class->get_property = gst_tcambin_get_property;

    element_class->change_state = GST_DEBUG_FUNCPTR(gst_tcam_bin_change_state);

    g_object_class_install_property(
        gobject_class,
        PROP_SERIAL,
        g_param_spec_string("serial",
                            "Camera serial",
                            "Serial of the camera that shall be used",
                            "",
                            static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property(
        gobject_class,
        PROP_DEVICE_TYPE,
        g_param_spec_string("type",
                            "Camera type",
                            "type/backend of the camera",
                            "auto",
                            static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property(
        gobject_class,
        PROP_TCAMDEVICE,
        g_param_spec_object("tcam-device",
                            "Tcam Device",
                            "Assigns the GstDevice to open when transitioning from NULL to READY.",
                            GST_TYPE_DEVICE,
                            static_cast<GParamFlags>(G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property(
        gobject_class,
        PROP_DEVICE_CAPS,
        g_param_spec_string("device-caps",
                            "Device Caps",
                            "GstCaps tcamsrc shall use",
                            "",
                            static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property(
        gobject_class,
        PROP_AVAILABLE_CAPS,
        g_param_spec_string("available-caps",
                            "GstCaps tcamsrc offers",
                            "GstCaps the tcamsrc device offers and the tcambin supports",
                            "",
                            static_cast<GParamFlags>(G_PARAM_READABLE | G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property(gobject_class,
                                    PROP_CONVERSION_ELEMENT,
                                    g_param_spec_enum("conversion-element",
                                                      "conversion",
                                                      "Select used transformation element",
                                                      g_type_from_name("TcamBinConversionElement"),
                                                      TCAM_BIN_CONVERSION_AUTO,
                                                      static_cast<GParamFlags>(G_PARAM_READWRITE)));

    g_object_class_install_property(
        gobject_class,
        PROP_TCAM_PROPERTIES_JSON,
        g_param_spec_string(
            "tcam-properties-json",
            "Reads/Writes the properties as a json string",
            "Reads/Writes the properties as a json string to/from the source/filter elements",
            "",
            static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));


    g_object_class_install_property(
        gobject_class,
        PROP_TCAM_PROPERTIES_GSTSTRUCT,
        g_param_spec_boxed(
            "tcam-properties",
            "Reads/Writes the properties in a GstStructure",
            "In GST_STATE_NULL, sets the initial values for tcam-property 1.0 properties."
            "In GST_STATE_READY, sets the current properties of the device, or reads the current "
            "state of all properties"
            "Names and types are the ones found in the tcam-property 1.0 interface."
            "(Usage e.g.: 'gst-launch-1.0 tcambin "
            "tcam-properties=tcam,ExposureAuto=Off,ExposureTime=33333 ! ...')",
            GST_TYPE_STRUCTURE,
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
