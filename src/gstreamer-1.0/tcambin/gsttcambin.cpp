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

#include "../../../libs/tcam-property/src/tcam-property-1.0.h"
#include "../../version.h"
#include "../tcamgstbase/version_check.h"
#include "../tcamgstbase/tcamgstbase.h"
#include "../tcamgstbase/tcamgstjson.h"
#include "gst/gstchildproxy.h"
#include "tcambin_data.h"
#include "tcambin_tcamprop_impl.h"

#include <cassert>
#include <cstring>
#include <gst-helper/gst_gvalue_helper.h>
#include <gst-helper/helper_functions.h>
#include <string>
#include <tcamprop1.0_gobject/tcam_property_serialize.h>
#include <unistd.h>

#define gst_tcambin_parent_class parent_class

#define GST_CAT_DEFAULT gst_tcambin_debug

G_DEFINE_TYPE_WITH_CODE(GstTcamBin,
                        gst_tcambin,
                        GST_TYPE_BIN,
                        G_IMPLEMENT_INTERFACE(TCAM_TYPE_PROPERTY_PROVIDER,
                                              tcam::gst::bin::gst_tcambin_tcamprop_init)
                        G_IMPLEMENT_INTERFACE(GST_TYPE_CHILD_PROXY, nullptr))

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


static const char* name_source = "tcambin-source";
static const char* name_capsfilter = "tcambin-src_caps";
static const char* name_converter = "tcambin-converter";
static const char* name_jpeg = "tcambin-jpegdec";
static const char* name_videoconvert = "tcambin-videoconvert";


static GstStaticPadTemplate src_template =
    GST_STATIC_PAD_TEMPLATE("src", GST_PAD_SRC, GST_PAD_ALWAYS, GST_STATIC_CAPS_ANY);


static gst_helper::gst_ptr<GstCaps> tcambin_filter_unsupported_caps(GstTcamBin* self, const GstCaps& source_caps)
{

    bool nvmm_flag = true;

    if (self->data->conversion_info.selected_conversion == TCAM_BIN_CONVERSION_CUDA)
    {
        nvmm_flag = false;
    }

    auto filter_func = [] (GstCapsFeatures* features,
                           GstStructure* structure,
                           gpointer user_data) -> gboolean
    {
        bool filter_nvmm = *(bool*)(user_data);

        if (features)
        {
            if (gst_caps_features_contains(features, "memory:NVMM") && filter_nvmm)
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

    gst_caps_filter_and_map_in_place(available_caps.get(), filter_func, (void*)&nvmm_flag);

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

static bool tcambin_create_source(GstTcamBin* self)
{
    gst_tcambin_clear_source(self);

    auto& data = get_tcambin_data(self);

    gst_helper::gst_ptr<GstElement> src_element;
    if (data.prop_tcam_device)
    {
        src_element = gst_helper::make_ptr(
            gst_device_create_element(data.prop_tcam_device.get(), name_source));
    }
    else
    {
        src_element = gst_helper::make_ptr(gst_element_factory_make("tcamsrc", name_source));
        assert(src_element);

        if (!data.device_type.empty())
        {
            GST_INFO_OBJECT(self, "Setting source type to %s", data.device_type.c_str());
            g_object_set(G_OBJECT(src_element.get()), "type", data.device_type.c_str(), NULL);
        }

        if (!data.device_serial.empty())
        {
            GST_INFO_OBJECT(self, "Setting source serial to %s", data.device_serial.c_str());
            g_object_set(G_OBJECT(src_element.get()), "serial", data.device_serial.c_str(), NULL);
        }
    }

    if (src_element == nullptr)
    {
        // The actual object should have told us what went wrong
        return false;
    }

    gst_bin_add(GST_BIN(self), src_element.get());

    GstChildProxy* proxy = GST_CHILD_PROXY(self);
    gst_child_proxy_child_added(proxy, (GObject*)src_element.get(), name_source);

    // set to READY so that caps are always readable
    auto res = gst_element_set_state(src_element.get(), GST_STATE_READY);
    if (res == GST_STATE_CHANGE_FAILURE)
    {
        return false;
    }

    data.src_element = src_element;

    GST_INFO_OBJECT(self,
                    "Opened device has serial: '%s' type: '%s'",
                    gst_helper::gobject_get_string_opt(data.src_element.get(), "serial")
                        .value_or(std::string {})
                        .c_str(),
                    gst_helper::gobject_get_string_opt(data.src_element.get(), "type")
                        .value_or(std::string {})
                        .c_str());

    // reset 'serial' and 'type' property
    data.device_serial = {};
    data.device_type = {};
    data.prop_tcam_device = {};

    auto src_caps = gst_helper::query_caps(*gst_helper::get_static_pad(*data.src_element, "src"));

    data.available_caps = tcambin_filter_unsupported_caps(self, *src_caps.get());

    return true;
}


static void gst_tcambin_clear_source(GstTcamBin* self)
{
    auto& data = get_tcambin_data(self);
    if (data.src_element)
    {
        gst_element_set_state(data.src_element.get(), GST_STATE_NULL);

        GstChildProxy* proxy = GST_CHILD_PROXY(self);
        gst_child_proxy_child_removed(proxy, G_OBJECT(data.src_element.get()), name_source);
        gst_bin_remove(GST_BIN(self), data.src_element.get());
        data.src_element = nullptr;
    }
}

static void gst_tcambin_clear_elements(GstTcamBin* self)
{
    auto& data = get_tcambin_data(self);

    auto remove_element = [self](GstElement* element, const char* name)
    {
        assert(GST_IS_ELEMENT(element));

        gst_element_set_state(element, GST_STATE_NULL);

        GstChildProxy* proxy = GST_CHILD_PROXY(self);
        gst_child_proxy_child_removed(proxy, (GObject*)element, name);

        gst_bin_remove(GST_BIN(self), element);
        // not needed bin_remove automatically does that
        // gst_object_unref(element);
    };

    if (data.pipeline_caps)
    {
        gst_element_set_state(data.pipeline_caps, GST_STATE_NULL);
        if (data.src_element)
        {
            gst_element_unlink_pads(data.src_element.get(), "src", data.pipeline_caps, "sink");
        }
        gst_bin_remove(GST_BIN(self), data.pipeline_caps);
        data.pipeline_caps = nullptr;
    }

    if (data.tcam_converter)
    {
        remove_element(data.tcam_converter, name_converter);
        data.tcam_converter = nullptr;
    }

    if (data.jpegdec)
    {
        remove_element(data.jpegdec, name_jpeg);
        data.jpegdec = nullptr;

        if (data.videoconvert)
        {
            remove_element(data.videoconvert, name_videoconvert);
            data.videoconvert = nullptr;
        }
    }

    data.elements_created = false;
}


// check element existence and compatibility
// assuming all transform elements exist the order is:
// tcamdutils -> tcamdutils-cuda -> tcamconvert
// manual overwrite disregards all checks and forces an element!
// this can cause errors and non functioning pipelines!
static void select_transform_element(TcamBinConversionElement& user_selector,
                                     TcamBinConversionElement& internal_selector)
{
    static bool compatibility_checked;

    static bool dutils_cuda_exists;
    static bool dutils_cuda_is_compatible;

    static bool dutils_exists;
    static bool dutils_is_compatible;

    // one time check
    if (!compatibility_checked)
    {
        compatibility_checked = true;
        auto factory_cuda = gst_element_factory_find("tcamdutils-cuda");
        if (factory_cuda != nullptr)
        {
            dutils_cuda_exists = true;
            if (tcam::gst::is_version_compatible_with_tiscamera("tcamdutils-cuda"))
            {
                //data.conversion_info.selected_conversion = TCAM_BIN_CONVERSION_CUDA;
                dutils_cuda_is_compatible = true;
            }
            else
            {
                GST_WARNING("tcamdutils-cuda detected. "
                            "Versions are not compatible. "
                            "Used only if forced.");
            }
            gst_object_unref(factory_cuda);
        }

        auto factory = gst_element_factory_find("tcamdutils");
        if (factory != nullptr)
        {
            dutils_exists = true;
            if (tcam::gst::is_version_compatible_with_tiscamera("tcamdutils"))
            {
                dutils_is_compatible = true;
            }
            else
            {
                GST_WARNING("tcamdutils detected. "
                            "Versions are not compatible. "
                            "Used only if forced.");
            }
            gst_object_unref(factory);
        }
    }

    if (user_selector == TCAM_BIN_CONVERSION_CUDA)
    {
        if (dutils_cuda_exists)
        {
            internal_selector = TCAM_BIN_CONVERSION_CUDA;
            GST_INFO("User selected transform element tcamdutils-cuda");
            if (!dutils_cuda_is_compatible)
            {
                GST_WARNING("tcamdutils-cuda is marked as incompatible. Error may occur.");
            }
        }
        else
        {
            GST_ERROR("Unable to use tcamconvert. Element does not seem to exist. Falling "
                      "back to auto");
            internal_selector = TCAM_BIN_CONVERSION_AUTO;
        }
    }
    else if (user_selector == TCAM_BIN_CONVERSION_DUTILS)
    {
        if (dutils_exists)
        {
            internal_selector = TCAM_BIN_CONVERSION_DUTILS;
            GST_INFO("User selected transform element tcamdutils");
        }
        else
        {
            GST_ERROR("Unable to use tcamdutils. Element does not seem to exist. Falling "
                      "back to auto");
            internal_selector = TCAM_BIN_CONVERSION_AUTO;
        }
    }
    else if (user_selector == TCAM_BIN_CONVERSION_CONVERT)
    {
        // tcamconvert always exists and is always compatible
        internal_selector = TCAM_BIN_CONVERSION_CONVERT;
        GST_INFO("User selected transform element tcamconvert");
    }
    else // user_selector == TCAM_BIN_CONVERSION_AUTO
    {
        std::string selected_element = "tcamconvert";
        if (dutils_exists && dutils_is_compatible)
        {
            internal_selector = TCAM_BIN_CONVERSION_DUTILS;
            user_selector = TCAM_BIN_CONVERSION_DUTILS;
            selected_element = "tcamdutils";
        }
        else if (dutils_cuda_exists && dutils_cuda_is_compatible)
        {
            internal_selector = TCAM_BIN_CONVERSION_CUDA;
            user_selector = TCAM_BIN_CONVERSION_CUDA;
            selected_element = "tcamdutils-cuda";
        }
        else
        {
            internal_selector = TCAM_BIN_CONVERSION_CONVERT;
            user_selector = TCAM_BIN_CONVERSION_CONVERT;
        }
        GST_INFO("Auto selected transform element: %s", selected_element.c_str());
    }
}


static gboolean create_and_add_element(GstElement** element,
                                       const char* factory_name,
                                       const char* element_name,
                                       GstBin* bin)
{
    *element = gst_element_factory_make(factory_name, element_name);
    if (!*element)
    {
        return FALSE;
    }
    GST_DEBUG_OBJECT(GST_ELEMENT(bin), "Adding %s(%p) to pipeline", factory_name, (void*)*element);
    gst_bin_add(bin, *element);

    GstChildProxy* proxy = GST_CHILD_PROXY(bin);
    gst_child_proxy_child_added(proxy, (GObject*)*element, element_name);

    return TRUE;
}

// helper function to link elements
static bool link_elements(GstElement* previous_element,
                          GstElement* element,
                          std::string& pipeline_description,
                          const std::string& name)
{
    if (!element)
    {
        return false;
    }

    gboolean link_ret = gst_element_link(previous_element, element);
    if (!link_ret)
    {
        return false;
    }
    pipeline_description += " ! ";
    pipeline_description += name;
    return true;
}

static bool tcambin_create_elements(GstTcamBin* self)
{
    auto& data = get_tcambin_data(self);

    assert(!data.is_open()); // already open?

    GST_INFO_OBJECT(self, "creating all tcambin pipeline elements");

    data.pipeline_caps = gst_element_factory_make("capsfilter", name_capsfilter);
    if (data.pipeline_caps == nullptr)
    {
        GST_ERROR_OBJECT(self, "Could not create internal pipeline caps. Aborting");
        return false;
    }

    gst_bin_add(GST_BIN(self), data.pipeline_caps);

    std::string pipeline_string = "tcamsrc";

    if (gst_element_link(data.src_element.get(), data.pipeline_caps))
    {
        pipeline_string += " ! capsfilter";
    }
    else
    {
        GST_ELEMENT_ERROR(
            self, CORE, MISSING_PLUGIN, ("Could not create/link element 'capsfilter'."), (NULL));
        return false;
    }

    if (tcam::gst::contains_jpeg(data.available_caps.get()))
    {
        if (!create_and_add_element(&data.jpegdec, "jpegdec", name_jpeg, GST_BIN(self)))
        {
            GST_ELEMENT_ERROR(
                self, CORE, MISSING_PLUGIN, ("Could not create element 'jpegdec'."), (NULL));
            return false;
        }


        std::string bin_name = "tcambin-jpegdec";
        if (!link_elements(data.pipeline_caps, data.jpegdec, pipeline_string, bin_name))
        {
            GST_ELEMENT_ERROR(
                self, CORE, NEGOTIATION, ("Could not link element '%s'.", "jpegdec"), (NULL));
            return false;
        }

        // always add videoconvert
        // this is necessaryto ensure output BGRx as output works
        if (!create_and_add_element(
                &data.videoconvert, "videoconvert", "tcambin-videoconvert", GST_BIN(self)))
        {
            GST_ELEMENT_ERROR(
                self, CORE, MISSING_PLUGIN, ("Could not create element 'videoconvert'."), (NULL));
            return false;
        }

        if (!link_elements(data.jpegdec, data.videoconvert, pipeline_string, name_videoconvert))
        {
            GST_ELEMENT_ERROR(
                self, CORE, NEGOTIATION, ("Could not link element '%s'.", "videoconvert"), (NULL));
            return false;
        }

        data.target_pad = gst_helper::get_static_pad(*self->data->videoconvert, "src");
    }
    else
    {
        std::string element_name;
        if (data.conversion_info.selected_conversion == TCAM_BIN_CONVERSION_DUTILS)
        {
            if (!create_and_add_element(
                    &data.tcam_converter, "tcamdutils", name_converter, GST_BIN(self)))
            {
                GST_ELEMENT_ERROR(
                    self, CORE, MISSING_PLUGIN, ("Could not create element 'tcamdutils'."), (NULL));
                return false;
            }
            element_name = "tcamdutils";
        }
        else if (data.conversion_info.selected_conversion == TCAM_BIN_CONVERSION_CUDA)
        {
            if (!create_and_add_element(&data.tcam_converter,
                                        "tcamdutils-cuda",
                                        name_converter,
                                        GST_BIN(self)))
            {
                GST_ELEMENT_ERROR(self,
                                  CORE,
                                  MISSING_PLUGIN,
                                  ("Could not create element 'tcamdutils-cuda'."),
                                  (NULL));
                return false;
            }
            element_name = "tcamdutils-cuda";
        }
        else // default selection
        {
            if (!create_and_add_element(
                    &data.tcam_converter, "tcamconvert", name_converter, GST_BIN(self)))
            {
                GST_ELEMENT_ERROR(self,
                                  CORE,
                                  MISSING_PLUGIN,
                                  ("Could not create element 'tcamconvert'."),
                                  (NULL));

                return false;
            }
            element_name = "tcamconvert";
        }

        if (!link_elements(data.pipeline_caps, data.tcam_converter, pipeline_string, element_name))
        {
            GST_ELEMENT_ERROR(self,
                              CORE,
                              NEGOTIATION,
                              ("Could not link element '%s'.", element_name.c_str()),
                              (NULL));
            return false;
        }
        data.target_pad = gst_helper::get_static_pad(*self->data->tcam_converter, "src");
    }

    GST_DEBUG_OBJECT(self, "Internal pipeline: %s", pipeline_string.c_str());

    data.elements_created = true;

    return true;
}


static void set_target_pad(GstTcamBin* self)
{
    auto& data = get_tcambin_data(self);

    gst_ghost_pad_set_target(GST_GHOST_PAD(data.src_ghost_pad), NULL);

    if (data.target_set == false)
    {
        if (data.target_pad == nullptr)
        {
            GST_ERROR_OBJECT(self, "target_pad not defined");
        }
        else
        {
            if (!gst_ghost_pad_set_target(GST_GHOST_PAD(data.src_ghost_pad), data.target_pad.get()))
            {
                GST_ERROR_OBJECT(self, "Could not set target for ghostpad.");
            }
        }
        data.target_set = true;
    }
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
            // select transform element here
            // this checks compatibilities
            // informs the user about them
            // and gives us the infos we need for create_elements
            select_transform_element(self->data->conversion_info.user_selector,
                                     self->data->conversion_info.selected_conversion);

            if (!tcambin_create_source(self))
            {
                return GST_STATE_CHANGE_FAILURE;
            }

            if (!tcambin_create_elements(self))
            {
                gst_tcambin_clear_source(self);
                return GST_STATE_CHANGE_FAILURE;
            }
            set_target_pad(self);

            // all elements should now be here, so we can load the settings
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

            break;
        }
        case GST_STATE_CHANGE_READY_TO_PAUSED:
        {
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

                if (gst_caps_is_any(data.target_caps.get()))
                {
                    // when offered 'ANY' change to 'EMPTY'
                    // prefer a single 'generic' caps definition over two
                    data.target_caps = gst_helper::make_ptr(gst_caps_new_empty());
                }
            }

            auto src_caps =
                gst_helper::query_caps(*gst_helper::get_static_pad(*data.src_element, "src"));

            if (data.user_caps)
            {
                GstCaps* tmp =
                    gst_caps_intersect(data.user_caps.get(), self->data->available_caps.get());
                if (tmp == nullptr || gst_caps_is_empty(tmp))
                {
                    GST_ELEMENT_ERROR(self,
                                      STREAM,
                                      WRONG_TYPE,
                                      ("The user defined device caps are not supported by the "
                                       "device. User caps are: %s",
                                       gst_helper::to_string(*data.user_caps).c_str()),
                                      (NULL));
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

            ret = GST_STATE_CHANGE_NO_PREROLL;
            break;
        }
        default:
        {
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

static bool is_state_null(GstTcamBin* self) noexcept
{
    return tcam::gst::is_gst_state_equal_or_less(GST_ELEMENT(self), GST_STATE_NULL);
}

static bool is_state_ready_or_higher(GstTcamBin* self) noexcept
{
    return tcam::gst::is_gst_state_equal_or_greater(GST_ELEMENT(self), GST_STATE_READY);
}

static bool is_state_ready_or_lower(GstTcamBin* self) noexcept
{
    return tcam::gst::is_gst_state_equal_or_less(GST_ELEMENT(self), GST_STATE_READY);
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
            if (state.src_element)
            {
                g_object_get_property(G_OBJECT(state.src_element.get()), "serial", value);
            }
            else
            {
                g_value_set_string(value, state.device_serial.c_str());
            }
            break;
        }
        case PROP_DEVICE_TYPE:
        {
            if (state.src_element)
            {
                g_object_get_property(G_OBJECT(state.src_element.get()), "type", value);
            }
            else
            {
                g_value_set_string(value, state.device_type.c_str());
            }
            break;
        }
        case PROP_DEVICE_CAPS:
        {
            if (state.user_caps)
            {
                g_value_set_string(value, gst_helper::to_string(*state.user_caps).c_str());
            }
            else
            {
                g_value_set_string(value, "");
            }
            break;
        }
        case PROP_AVAILABLE_CAPS:
        {
            if (!is_state_ready_or_higher(self))
            {
                GST_ERROR_OBJECT(
                    self, "GObject property 'device-caps' is only readable >= GST_STATE_READY.");
                return;
            }

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
            if (!is_state_null(self))
            {
                GST_ERROR_OBJECT(
                    self, "GObject property 'serial' is not writable in state >= GST_STATE_READY.");
                return;
            }

            state.device_serial =
                g_value_get_string(value) != nullptr ? g_value_get_string(value) : std::string();
            break;
        }
        case PROP_DEVICE_TYPE:
        {
            if (!is_state_null(self))
            {
                GST_ERROR_OBJECT(
                    self, "GObject property 'type' is not writable in state >= GST_STATE_READY.");
                return;
            }

            state.device_type =
                g_value_get_string(value) != nullptr ? g_value_get_string(value) : std::string();
            break;
        }
        case PROP_DEVICE_CAPS:
        {
            if (!is_state_ready_or_lower(self))
            {
                GST_ERROR_OBJECT(
                    self,
                    "GObject property 'device-caps' is not writable in state >= GST_STATE_PAUSED.");
                return;
            }

            state.user_caps = gst_helper::make_ptr(gst_caps_from_string(g_value_get_string(value)));
            break;
        }
        case PROP_CONVERSION_ELEMENT:
        {
            if (!is_state_null(self))
            {
                GST_ERROR_OBJECT(self,
                                 "GObject property 'conversion-element' is not writable in state "
                                 ">= GST_STATE_READY.");
                return;
            }

            self->data->conversion_info.user_selector =
                (TcamBinConversionElement)g_value_get_enum(value);

            // select_transform_element(self->data->conversion_info.user_selector,
            //                          self->data->conversion_info.selected_conversion);

            break;
        }
        case PROP_TCAMDEVICE:
        {
            if (!is_state_null(self))
            {
                GST_ERROR_OBJECT(
                    self,
                    "GObject property 'tcam-device' is not writable in state >= GST_STATE_READY.");
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
    self->data = new tcambin_data;

    auto& data = *self->data;

    data.conversion_info.user_selector = TCAM_BIN_CONVERSION_AUTO;
    data.conversion_info.selected_conversion = TCAM_BIN_CONVERSION_CONVERT;

    // NOTE: the result of gst_ghost_pad_new_no_target is a floating reference that is consumed by gst_element_add_pad
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
