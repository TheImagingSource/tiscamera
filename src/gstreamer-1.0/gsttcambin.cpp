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
// #include <girepository.h>

#include "gst_helper.h"
#include "tcamgstbase.h"
#include "tcamgstjson.h"
#include "tcamprop.h"

#include <cstring>
#include <glib-object.h>
#include <string>
#include <unistd.h>
#include <vector>

struct tcambin_data
{
    std::string device_serial;
    std::string device_type;

    std::string state;

    gst_helper::gst_unique_ptr<GstPad> target_pad;
    gst_helper::gst_unique_ptr<GstCaps> user_caps;

    GstPad* src_ghost_pad =
        nullptr; // NOTE: we don't have a reference to this, so this is a observer that will be made invalid in dispose

    gst_helper::gst_unique_ptr<GstElement> out_caps;

    gst_helper::gst_unique_ptr<GstCaps> src_caps;
    gst_helper::gst_unique_ptr<GstCaps> target_caps;
};


#define gst_tcambin_parent_class parent_class

GST_DEBUG_CATEGORY_STATIC(gst_tcambin_debug);
#define GST_CAT_DEFAULT gst_tcambin_debug


static gboolean gst_tcambin_create_source(GstTcamBin* self);
static gboolean gst_tcambin_create_elements(GstTcamBin* self);
static void gst_tcambin_clear_source(GstTcamBin* self);

//
// introspection interface
//

static GSList* gst_tcam_bin_get_property_names(TcamProp* self);

static gchar* gst_tcam_bin_get_property_type(TcamProp* self, const gchar* name);

static gboolean gst_tcam_bin_get_tcam_property(TcamProp* self,
                                               const gchar* name,
                                               GValue* value,
                                               GValue* min,
                                               GValue* max,
                                               GValue* def,
                                               GValue* step,
                                               GValue* type,
                                               GValue* flags,
                                               GValue* category,
                                               GValue* group);

static gboolean gst_tcam_bin_set_tcam_property(TcamProp* self,
                                               const gchar* name,
                                               const GValue* value);

static GSList* gst_tcam_bin_get_tcam_menu_entries(TcamProp* self, const gchar* name);

static GSList* gst_tcam_bin_get_device_serials(TcamProp* self);
static GSList* gst_tcam_bin_get_device_serials_backend(TcamProp* self);

static gboolean gst_tcam_bin_get_device_info(TcamProp* self,
                                             const char* serial,
                                             char** name,
                                             char** identifier,
                                             char** connection_type);

static void gst_tcam_bin_prop_init(TcamPropInterface* iface)
{
    iface->get_tcam_property_names = gst_tcam_bin_get_property_names;
    iface->get_tcam_property_type = gst_tcam_bin_get_property_type;
    iface->get_tcam_property = gst_tcam_bin_get_tcam_property;
    iface->get_tcam_menu_entries = gst_tcam_bin_get_tcam_menu_entries;
    iface->set_tcam_property = gst_tcam_bin_set_tcam_property;
    iface->get_tcam_device_serials = gst_tcam_bin_get_device_serials;
    iface->get_tcam_device_serials_backend = gst_tcam_bin_get_device_serials_backend;
    iface->get_tcam_device_info = gst_tcam_bin_get_device_info;
}


G_DEFINE_TYPE_WITH_CODE(GstTcamBin,
                        gst_tcambin,
                        GST_TYPE_BIN,
                        G_IMPLEMENT_INTERFACE(TCAM_TYPE_PROP, gst_tcam_bin_prop_init))


/**
 * gst_tcam_get_property_type:
 * @self: a #GstTcamBin
 * @name: a #char* identifying the property to query
 *
 * Return the type of a property
 *
 * Returns: (transfer full): A string describing the property type
 */
static gchar* gst_tcam_bin_get_property_type(TcamProp* iface, const gchar* name)
{
    gchar* ret = NULL;

    GstTcamBin* self = GST_TCAMBIN(iface);

    if (!self->elements_created)
    {
        gst_tcambin_create_elements(GST_TCAMBIN(self));
    }

    GstIterator* it = gst_bin_iterate_elements(GST_BIN(self));
    GValue item = G_VALUE_INIT;
    for (GstIteratorResult res = gst_iterator_next(it, &item); res == GST_ITERATOR_OK;
         res = gst_iterator_next(it, &item))
    {
        GstElement* element = GST_ELEMENT(g_value_get_object(&item));
        if (TCAM_IS_PROP(element))
        {
            ret = g_strdup(tcam_prop_get_tcam_property_type(TCAM_PROP(element), name));
            if (ret != nullptr)
            {
                break;
            }
        }
        g_value_unset(&item);
    }

    gst_iterator_free(it);

    return ret;
}


/**
 * gst_tcam_bin_get_property_names:
 * @self: a #GstTcamBin
 *
 * Return a list of property names
 *
 * Returns: (element-type utf8) (transfer full): list of property names
 */
static GSList* gst_tcam_bin_get_property_names(TcamProp* iface)
{
    GSList* ret = NULL;
    GstTcamBin* self = GST_TCAMBIN(iface);

    if (!self->elements_created)
    {
        gst_tcambin_create_elements(GST_TCAMBIN(self));
    }

    GstIterator* it = gst_bin_iterate_elements(GST_BIN(self));
    GValue item = G_VALUE_INIT;
    for (GstIteratorResult res = gst_iterator_next(it, &item); res == GST_ITERATOR_OK;
         res = gst_iterator_next(it, &item))
    {
        GstElement* element = GST_ELEMENT(g_value_get_object(&item));
        if (TCAM_IS_PROP(element))
        {
            GSList* prop_names = tcam_prop_get_tcam_property_names(TCAM_PROP(element));
            ret = g_slist_concat(ret, prop_names);
        }
        g_value_unset(&item);
    }

    gst_iterator_free(it);

    return ret;
}


static gboolean gst_tcam_bin_get_tcam_property(TcamProp* iface,
                                               const gchar* name,
                                               GValue* value,
                                               GValue* min,
                                               GValue* max,
                                               GValue* def,
                                               GValue* step,
                                               GValue* type,
                                               GValue* flags,
                                               GValue* category,
                                               GValue* group)
{
    GstTcamBin* self = GST_TCAMBIN(iface);

    if (!self->elements_created)
    {
        gst_tcambin_create_elements(self);
    }

    gboolean ret = false;

    GstIterator* it = gst_bin_iterate_elements(GST_BIN(self));
    GValue item = G_VALUE_INIT;
    for (GstIteratorResult res = gst_iterator_next(it, &item); res == GST_ITERATOR_OK;
         res = gst_iterator_next(it, &item))
    {
        GstElement* element = GST_ELEMENT(g_value_get_object(&item));
        if (TCAM_IS_PROP(element))
        {
            if (tcam_prop_get_tcam_property(TCAM_PROP(element),
                                            name,
                                            value,
                                            min,
                                            max,
                                            def,
                                            step,
                                            type,
                                            flags,
                                            category,
                                            group))
            {
                ret = true;
                break;
            }
        }
        g_value_unset(&item);
    }

    gst_iterator_free(it);

    return ret;
}


/**
 * gst_tcam_bin_get_tcam_manu_entries:
 * @self: a #GstTcamBin
 * @name: a #char*
 *
 * Return a list of property names
 *
 * Returns: (element-type utf8) (transfer full): a #GSList
 */
static GSList* gst_tcam_bin_get_tcam_menu_entries(TcamProp* iface, const gchar* name)
{
    GstTcamBin* self = GST_TCAMBIN(iface);

    if (!self->elements_created)
    {
        gst_tcambin_create_elements(GST_TCAMBIN(self));
    }

    GSList* ret;

    GstIterator* it = gst_bin_iterate_elements(GST_BIN(self));
    GValue item = G_VALUE_INIT;
    for (GstIteratorResult res = gst_iterator_next(it, &item); res == GST_ITERATOR_OK;
         res = gst_iterator_next(it, &item))
    {
        GstElement* element = GST_ELEMENT(g_value_get_object(&item));
        if (TCAM_IS_PROP(element))
        {
            ret = tcam_prop_get_tcam_menu_entries(TCAM_PROP(element), name);
            if (ret)
            {
                break;
            }
        }
        g_value_unset(&item);
    }

    gst_iterator_free(it);

    return ret;
}

static gboolean gst_tcam_bin_set_tcam_property(TcamProp* iface,
                                               const gchar* name,
                                               const GValue* value)
{
    GstTcamBin* self = GST_TCAMBIN(iface);

    if (!self->elements_created)
    {
        gst_tcambin_create_elements(self);
    }

    gboolean ret = false;

    GstIterator* it = gst_bin_iterate_elements(GST_BIN(self));
    GValue item = G_VALUE_INIT;
    for (GstIteratorResult res = gst_iterator_next(it, &item); res == GST_ITERATOR_OK;
         res = gst_iterator_next(it, &item))
    {
        GstElement* element = GST_ELEMENT(g_value_get_object(&item));
        if (TCAM_IS_PROP(element))
        {
            if (g_strcmp0(gst_element_get_name(element), "tcambin-dutils") == 0)
            {
                GST_INFO("AHA");
            }
            if (tcam_prop_set_tcam_property(TCAM_PROP(element), name, value))
            {
                ret = true;
                break;
            }
        }
        g_value_unset(&item);
    }

    gst_iterator_free(it);

    return ret;
}


static GSList* gst_tcam_bin_get_device_serials(TcamProp* self __attribute((__unused__)))
{
    GstElement* src = gst_element_factory_make("tcamsrc", nullptr);
    if (src == nullptr)
    {
        g_critical("Failed to create a tcamsrc");
        return nullptr;
    }

    GSList* serials = tcam_prop_get_device_serials(TCAM_PROP(src));

    gst_object_unref(src);

    return serials;
}


static GSList* gst_tcam_bin_get_device_serials_backend(TcamProp* self __attribute((__unused__)))
{
    GstElement* src = gst_element_factory_make("tcamsrc", nullptr);
    if (src == nullptr)
    {
        g_critical("Failed to create a tcamsrc");
        return nullptr;
    }

    GSList* serials = tcam_prop_get_device_serials_backend(TCAM_PROP(src));

    gst_object_unref(src);

    return serials;
}


static gboolean gst_tcam_bin_get_device_info(TcamProp* self __attribute((__unused__)),
                                             const char* serial,
                                             char** name,
                                             char** identifier,
                                             char** connection_type)
{
    GstElement* src = gst_element_factory_make("tcamsrc", nullptr);
    if (src == nullptr)
    {
        g_critical("Failed to create a tcamsrc");
        return false;
    }
    gboolean ret =
        tcam_prop_get_device_info(TCAM_PROP(src), serial, name, identifier, connection_type);
    gst_object_unref(src);

    return ret;
}


//
// gstreamer module
//

static void gst_tcambin_class_init(GstTcamBinClass* klass);
static void gst_tcambin_init(GstTcamBin* klass);

enum
{
    PROP_0,
    PROP_SERIAL,
    PROP_DEVICE_TYPE,
    PROP_DEVICE_CAPS,
    PROP_USE_DUTILS,
    PROP_STATE
};

static GstStaticPadTemplate src_template =
    GST_STATIC_PAD_TEMPLATE("src", GST_PAD_SRC, GST_PAD_ALWAYS, GST_STATIC_CAPS_ANY);


static gboolean gst_tcambin_create_source(GstTcamBin* self)
{
    gst_tcambin_clear_source(self);

    GST_DEBUG("Creating source...");

    self->src = gst_element_factory_make("tcamsrc", "tcambin-source");
    gst_bin_add(GST_BIN(self), self->src);

    if (!self->data->device_type.empty())
    {
        GST_INFO("Setting source type to %s", self->data->device_type.c_str());
        g_object_set(G_OBJECT(self->src), "type", self->data->device_type.c_str(), NULL);
    }

    if (!self->data->device_serial.empty())
    {
        GST_INFO("Setting source serial to %s", self->data->device_serial.c_str());
        g_object_set(G_OBJECT(self->src), "serial", self->data->device_serial.c_str(), NULL);
    }


    // set to READY so that caps are always readable
    gst_element_set_state(self->src, GST_STATE_READY);

    char* serial_from_src = nullptr;
    char* type_from_src = nullptr;
    // query these as late as possible
    // src needs some time as things can happen async
    g_object_get(G_OBJECT(self->src), "serial", &serial_from_src, "type", &type_from_src, NULL);
    self->data->device_serial = serial_from_src != nullptr ? serial_from_src : std::string();
    self->data->device_type = type_from_src != nullptr ? type_from_src : std::string();

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
                    self->data->device_serial.c_str(),
                    self->data->device_type.c_str());

    self->data->src_caps = gst_helper::query_caps(gst_helper::get_static_pad(self->src, "src"));
    GST_INFO_OBJECT(
        self, "caps of src: %" GST_PTR_FORMAT, static_cast<void*>(self->data->src_caps.get()));

    return TRUE;
}


static void gst_tcambin_clear_source(GstTcamBin* self)
{
    if (self->src)
    {
        gst_element_set_state(self->src, GST_STATE_NULL);
        gst_bin_remove(GST_BIN(self), self->src);
        self->src = nullptr;
    }
}


static void gst_tcambin_clear_elements(GstTcamBin* self)
{

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

    if (self->pipeline_caps)
    {
        gst_element_set_state(self->pipeline_caps, GST_STATE_NULL);
        if (self->src)
        {
            gst_element_unlink_pads(self->src, "src", self->pipeline_caps, "sink");
        }
        gst_bin_remove(GST_BIN(self), self->pipeline_caps);
        self->pipeline_caps = nullptr;
    }

    if (self->dutils)
    {
        remove_element(&self->dutils);
    }

    if (self->bayer_transform)
    {
        remove_element(&self->bayer_transform);
    }
    if (self->exposure)
    {
        remove_element(&self->exposure);
    }
    if (self->whitebalance)
    {
        remove_element(&self->whitebalance);
    }
    if (self->debayer)
    {
        remove_element(&self->debayer);
    }
    if (self->focus)
    {
        remove_element(&self->focus);
    }
    if (self->jpegdec)
    {
        remove_element(&self->jpegdec);
    }
    if (self->convert)
    {
        remove_element(&self->convert);
    }

    self->elements_created = false;
}


static gboolean create_and_add_element(GstElement** element,
                                       const char* factory_name,
                                       const char* element_name,
                                       GstBin* bin)
{
    auto factory = gst_element_factory_find(factory_name);

    if (!factory)
    {
        return FALSE;
    }
    gst_object_unref(factory);

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


static gboolean gst_tcambin_create_elements(GstTcamBin* self)
{

    if (self->elements_created)
    {
        return TRUE;
    }

    gst_tcambin_clear_elements(self);

    GST_INFO("creating elements");

    if (self->src == nullptr)
    {
        gst_tcambin_create_source(self);
    }

    self->pipeline_caps = gst_element_factory_make("capsfilter", "tcambin-src_caps");

    if (self->pipeline_caps == nullptr)
    {
        GST_ERROR("Could not create internal pipeline caps. Aborting");
        return FALSE;
    }

    gst_bin_add(GST_BIN(self), self->pipeline_caps);

    auto send_missing_element_msg = [self](const std::string& element_name) {
        std::string msg_string = "Could not create element '" + element_name + "'.";
        GError* err =
            g_error_new_literal(GST_CORE_ERROR, GST_CORE_ERROR_MISSING_PLUGIN, msg_string.c_str());
        GstMessage* msg = gst_message_new_error(GST_OBJECT(self), err, msg_string.c_str());
        gst_element_post_message(GST_ELEMENT(self), msg);
        g_error_free(err);
        GST_ERROR("%s", msg_string.c_str());
    };

    if (self->has_dutils && self->toggles.use_dutils)
    {
        if (!create_and_add_element(&self->dutils, "tcamdutils", "tcambin-dutils", GST_BIN(self)))
        {
            send_missing_element_msg("tcamdutils");
            return FALSE;
        }

        // go to finish and do not create other elements
        goto finished_element_creation;
    }

    if (self->data->device_type == "pimipi")
    {
        if (!create_and_add_element(&self->bayer_transform,
                                    "tcamby1xtransform",
                                    "tcambin-bayertransform",
                                    GST_BIN(self)))
        {
            send_missing_element_msg("tcambayertransform");
            return FALSE;
        }
    }

    // the following elements only support mono or bayer
    // security check to prevent faulty pipelines

    if ((contains_bayer(self->data->src_caps.get())
         || tcam_gst_raw_only_has_mono(self->data->src_caps.get()))
        && self->data->device_type != "pimipi")
    {
        if (tcam_prop_get_tcam_property_type(TCAM_PROP(self->src), "Exposure Auto") == nullptr
            && tcam_prop_get_tcam_property_type(TCAM_PROP(self->src), "ExposureAuto") == nullptr)
        {
            if (!create_and_add_element(
                    &self->exposure, "tcamautoexposure", "tcambin-exposure", GST_BIN(self)))
            {
                send_missing_element_msg("tcamautoexposure");
                return FALSE;
            }
        }

        if (tcam_prop_get_tcam_property_type(TCAM_PROP(self->src), "Focus") != nullptr
            && (tcam_prop_get_tcam_property_type(TCAM_PROP(self->src), "Auto Focus") == nullptr
                && tcam_prop_get_tcam_property_type(TCAM_PROP(self->src), "Focus Auto") == nullptr
                && tcam_prop_get_tcam_property_type(TCAM_PROP(self->src), "FocusAuto") == nullptr))
        {
            if (!create_and_add_element(
                    &self->focus, "tcamautofocus", "tcambin-focus", GST_BIN(self)))
            {
                send_missing_element_msg("tcamautofocus");
                return FALSE;
            }
        }
    }

    // always add whitebalance when using bayer
    // we want it when debayering
    // users may still want it when asking for bayer
    if (contains_bayer(self->data->src_caps.get()))
    {
        // use this to see if the device already has the feature
        if (tcam_prop_get_tcam_property_type(TCAM_PROP(self->src), "Whitebalance Auto") == nullptr
            && tcam_prop_get_tcam_property_type(TCAM_PROP(self->src), "BalanceWhiteAuto")
                   == nullptr)
        {
            if (!create_and_add_element(
                    &self->whitebalance, "tcamwhitebalance", "tcambin-whitebalance", GST_BIN(self)))
            {
                send_missing_element_msg("tcamwhitebalance");
                return FALSE;
            }
        }
    }

    if (!create_and_add_element(&self->debayer, "bayer2rgb", "tcambin-debayer", GST_BIN(self)))
    {
        send_missing_element_msg("bayer2rgb");
        return FALSE;
    }

    if (contains_jpeg(self->data->src_caps.get()))
    {
        if (!create_and_add_element(&self->jpegdec, "jpegdec", "tcambin-jpegdec", GST_BIN(self)))
        {
            send_missing_element_msg("jpegdec");
            return FALSE;
        }
    }

// videoconvert can be needed by non-dutils and dutils pipelines
// thus include it after the label
finished_element_creation:

    // this is needed to allow for conversions such as
    // GRAY8 to BGRx that can exist when device-caps are set
    if (!create_and_add_element(&self->convert, "videoconvert", "tcambin-convert", GST_BIN(self)))
    {
        send_missing_element_msg("videoconvert");
        return FALSE;
    }

    self->elements_created = TRUE;

    return TRUE;
}


/**
 * Creating the required elements is responsibility of the caller
 */
static gboolean gst_tcambin_link_elements(GstTcamBin* self)
{
    GST_INFO("Linking elements");

    if (self->elements_linked)
    {
        GST_INFO("Already linked");
        return TRUE;
    }

    if (self->data->target_caps == nullptr)
    {
        GST_ERROR("Unknown target caps. Aborting.");
        return FALSE;
    }

    if (self->pipeline_caps == nullptr)
    {
        GST_ERROR("Could not create internal pipeline caps. Aborting");
        return FALSE;
    }

    if (self->data->src_caps == nullptr)
    {
        GST_ERROR("Could not find valid caps. Aborting pipeline creation.");
        return FALSE;
    }

    if (!gst_caps_is_fixed(self->data->src_caps.get()))
    {
        std::string sc = gst_helper::to_string(self->data->src_caps);
        GstCaps* tmp = tcam_gst_find_largest_caps(self->data->src_caps.get());
        GST_INFO("Caps were not fixed. Reduced '%s' to: '%s'",
                 sc.c_str(),
                 gst_helper::to_string(tmp).c_str());

        if (tmp)
        {
            self->data->src_caps.reset(tmp);
        }
        else
        {
            GST_WARNING("Unable to find largest caps. Continuing with unfixated caps.");
        }
    }
    else
    {
        GST_INFO("Caps are fixed. Using caps for src: %s",
                 gst_helper::to_string(self->data->src_caps).c_str());
    }

    // explicitly destroy the pipeline caps
    // when having a start/stop cycle that goes PLAYING-READY-PLAYING, etc
    // the capsfilter sometimes refuses to correctly link again
    // by simply destroying it and creating a new one we work around this issue

    if (self->pipeline_caps)
    {
        gst_element_set_state(self->pipeline_caps, GST_STATE_NULL);
        gst_bin_remove(GST_BIN(self), self->pipeline_caps);
    }

    self->pipeline_caps = gst_element_factory_make("capsfilter", "tcambin-src_caps");

    if (self->pipeline_caps == nullptr)
    {
        GST_ERROR("Could not create internal pipeline caps. Aborting");
        return FALSE;
    }

    gst_bin_add(GST_BIN(self), self->pipeline_caps);

    // back to normal linking

    g_object_set(self->pipeline_caps, "caps", self->data->src_caps.get(), NULL);

    gboolean ret = gst_element_link(self->src, self->pipeline_caps);

    if (!ret)
    {
        GST_ERROR("Unable to link src and capsfilter.");
        return ret;
    }

    // helper function to post error messages to GstBus
    auto send_linking_element_msg = [self](const std::string& element_name) {
        std::string msg_string = "Could not link element '" + element_name + "'.";
        GError* err =
            g_error_new(GST_CORE_ERROR, GST_CORE_ERROR_MISSING_PLUGIN, "%s", msg_string.c_str());
        GstMessage* msg = gst_message_new_error(GST_OBJECT(self), err, msg_string.c_str());
        gst_element_post_message(GST_ELEMENT(self), msg);
        g_error_free(err);
        GST_ERROR_OBJECT(self, "%s", msg_string.c_str());
    };

    // helper function to link elements
    auto link_elements = [self](bool condition,
                                GstElement** previous_element,
                                GstElement** element,
                                std::string& pipeline_description,
                                const std::string& name) {
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
    };

    std::string pipeline_description = "tcamsrc ! ";
    pipeline_description += gst_helper::to_string(self->data->src_caps);

    GstElement* previous_element = self->pipeline_caps;

    if (self->has_dutils && self->modules.dutils)
    {
        if (!link_elements((self->has_dutils && self->modules.dutils),
                           &previous_element,
                           &self->dutils,
                           pipeline_description,
                           "tcamdutils"))
        {
            send_linking_element_msg("tcamdutils");
            return FALSE;
        }

        // goto finished_element_linking;
    }

    if (self->bayer_transform && self->modules.bayertransform)
    {
        if (!link_elements(self->modules.bayertransform,
                           &previous_element,
                           &self->bayer_transform,
                           pipeline_description,
                           "tcambayertransform"))
        {
            send_linking_element_msg("tcambayertransform");
            return FALSE;
        }
    }

    // the following elements only support mono or bayer
    // security check to prevent faulty pipelines

    if (gst_caps_are_bayer_only(self->data->src_caps.get())
        || tcam_gst_raw_only_has_mono(self->data->src_caps.get()))
    {
        if (tcam_prop_get_tcam_property_type(TCAM_PROP(self->src), "Exposure Auto") == nullptr
            || tcam_prop_get_tcam_property_type(TCAM_PROP(self->src), "ExposureAuto") == nullptr)
        {
            if (!link_elements(self->exposure,
                               &previous_element,
                               &self->exposure,
                               pipeline_description,
                               "tcamautoexposure"))
            {
                send_linking_element_msg("tcamautoexposure");
                return FALSE;
            }
        }

        if (tcam_prop_get_tcam_property_type(TCAM_PROP(self->src), "Focus") != nullptr
            && (tcam_prop_get_tcam_property_type(TCAM_PROP(self->src), "Auto Focus") == nullptr
                || tcam_prop_get_tcam_property_type(TCAM_PROP(self->src), "FocusAuto") == nullptr))
        {
            if (!link_elements(self->focus,
                               &previous_element,
                               &self->focus,
                               pipeline_description,
                               "tcamautofocus"))
            {
                send_linking_element_msg("tcamautofocus");
                return FALSE;
            }
        }
    }

    // always add whitebalance when using bayer
    // we want it when debayering
    // users may still want it when asking for bayer
    if (contains_bayer(self->data->src_caps.get()))
    {
        // use this to see if the device already has the feature
        if (tcam_prop_get_tcam_property_type(TCAM_PROP(self->src), "Whitebalance Auto") == nullptr)
        {
            if (!link_elements(self->whitebalance,
                               &previous_element,
                               &self->whitebalance,
                               pipeline_description,
                               "tcamwhitebalance"))
            {
                send_linking_element_msg("tcamwhitebalance");
                return FALSE;
            }
        }
    }

    if (!link_elements(self->modules.bayer2rgb,
                       &previous_element,
                       &self->debayer,
                       pipeline_description,
                       "bayer2rgb"))
    {
        send_linking_element_msg("bayer2rgb");
        return FALSE;
    }

    if (!link_elements(self->modules.jpegdec,
                       &previous_element,
                       &self->jpegdec,
                       pipeline_description,
                       "jpegdec"))
    {
        send_linking_element_msg("jpegdec");
        return FALSE;
    }

    // this is needed to allow for conversions such as
    // GRAY8 to BGRx that can exist when device-caps are set
    if (!link_elements(self->modules.videoconvert,
                       &previous_element,
                       &self->convert,
                       pipeline_description,
                       "videoconvert"))
    {
        send_linking_element_msg("videoconvert");
        return FALSE;
    }

    // finished_element_linking:

    GST_INFO_OBJECT(self,
                    "Using %s as exit element for internal pipeline: %s",
                    gst_element_get_name(previous_element),
                    pipeline_description.c_str());
    self->data->target_pad = gst_helper::get_static_pad(previous_element, "src");

    if (gst_helper::caps_empty_or_any(self->data->target_caps))
    {
        self->data->target_caps.reset(gst_caps_copy(self->data->src_caps.get()));
    }

    GST_INFO_OBJECT(
        self, "Working with exit caps: %s", gst_helper::to_string(self->data->target_caps).c_str());
    self->elements_linked = TRUE;

    return TRUE;
}


/**
 * Generate GstCaps that contains all possible caps from src, bayer2rgb and videoconvert
 * in case of an error: return nullptr
 */
static GstCaps* generate_all_caps(GstTcamBin* self)
{
    gst_helper::gst_unique_ptr<GstPad> in_pad = gst_helper::get_static_pad(self->src, "src");
    GstCaps* incoming_caps = gst_pad_query_caps(in_pad.get(), NULL);

    // always can be passed through
    GstCaps* all_caps = gst_caps_copy(incoming_caps);

    // we have three scenarios:
    // 1. camera has video/x-raw,format=GRAY8 = passed through
    // 2. camera has video/x-bayer => bayer may be passed through or converted => bayer2rgb
    // 2. camera has video/x-raw,format=SOMETHING => passed through

    // this boils down to:
    // if camera is mono,jpeg,yuv => pass through and be done
    // if camera has bayer => add video/x-raw,format{EVERYTHING} with the correct settings

    GstCaps* to_remove = gst_caps_new_empty();

    for (guint i = 0; i < gst_caps_get_size(all_caps); ++i)
    {
        GstStructure* struc = gst_caps_get_structure(all_caps, i);

        if (gst_structure_get_field_type(struc, "format") == G_TYPE_STRING)
        {
            const char* string = gst_structure_get_string(struc, "format");

            if (tcam_gst_is_bayer8_string(string))
            {
                const GValue* width = gst_structure_get_value(struc, "width");
                const GValue* height = gst_structure_get_value(struc, "height");
                const GValue* framerate = gst_structure_get_value(struc, "framerate");

                GstStructure* s = gst_structure_new_empty("video/x-raw");

                GstCaps* tmp = get_caps_from_element_name("bayer2rgb", "src");

                GstStructure* tmp_struc = gst_structure_copy(gst_caps_get_structure(tmp, 0));
                gst_structure_set_value(s, "format", gst_structure_get_value(tmp_struc, "format"));

                gst_structure_set_value(s, "width", width);
                gst_structure_set_value(s, "height", height);
                gst_structure_set_value(s, "framerate", framerate);

                gst_caps_append_structure(all_caps, s);

                gst_caps_unref(tmp);
            }
            else if (!self->has_dutils
                     && (tcam_gst_is_bayer12_string(string)
                         || tcam_gst_is_bayer12_packed_string(string)
                         || tcam_gst_is_bayer16_string(string)))
            {
                gst_caps_append_structure(to_remove, gst_structure_copy(struc));
            }
        }
    }

    gst_caps_unref(incoming_caps);

    // TODO: find alternative
    // caps_substract implicitly calls gst_caps_simplify
    // this causes 'weird' caps like video/x-raw,format=rggb,width={2448, 2048},height=2048
    // these are hard to parse and should be avoided.
    // all_caps = gst_caps_subtract(all_caps, to_remove);

    gst_caps_unref(to_remove);
    return all_caps;
}


static void set_target_pad(GstTcamBin* self)
{

    gst_ghost_pad_set_target(GST_GHOST_PAD(self->data->src_ghost_pad), NULL);

    if (self->target_set == FALSE)
    {
        if (self->data->target_pad == nullptr)
        {
            GST_ERROR("target_pad not defined");
        }
        else
        {
            if (!gst_ghost_pad_set_target(GST_GHOST_PAD(self->data->src_ghost_pad),
                                          self->data->target_pad.get()))
            {
                GST_ERROR("Could not set target for ghostpad.");
            }
        }
        self->target_set = TRUE;
    }
}


static gboolean apply_state(GstTcamBin* self, const std::string& state)
{
    bool ret;
    if (self->data->device_serial.empty())
    {
        ret = load_device_settings(TCAM_PROP(self), "", state);
    }
    else
    {
        ret = load_device_settings(TCAM_PROP(self), self->data->device_serial, state);
    }

    if (!ret)
    {
        GST_WARNING_OBJECT(self, "Device may be in an undefined state.");
    }

    return ret;
}


static GstStateChangeReturn gst_tcam_bin_change_state(GstElement* element, GstStateChange trans)
{
    GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;

    GstTcamBin* self = GST_TCAMBIN(element);

    switch (trans)
    {
        case GST_STATE_CHANGE_NULL_TO_READY:
        {
            GST_INFO("NULL_TO_READY");

            if (self->src == nullptr)
            {
                gst_tcambin_create_source(self);
            }

            gst_element_set_state(self->src, GST_STATE_READY);

            self->data->out_caps.reset(gst_element_factory_make("capsfilter", "tcambin-out_caps"));

            gst_ghost_pad_set_target(GST_GHOST_PAD(self->data->src_ghost_pad),
                                     gst_helper::get_static_pad(self->data->out_caps, "src").get());

            GstCaps* all_caps = generate_all_caps(self);
            g_object_set(self->data->out_caps.get(), "caps", all_caps, NULL);
            gst_caps_unref(all_caps);

            if (!self->elements_created)
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

            gst_helper::gst_unique_ptr<GstPad> sinkpad { gst_pad_get_peer(
                self->data->src_ghost_pad) };

            if (sinkpad == nullptr)
            {
                self->data->target_caps.reset(gst_caps_new_empty());
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

                    self->data->target_caps.reset(ptr);
                }
                else
                {
                    self->data->target_caps = gst_helper::query_caps(sinkpad);
                }
                gst_object_unref(par);
                GST_INFO_OBJECT(self,
                                "caps of sink: %" GST_PTR_FORMAT,
                                static_cast<void*>(self->data->target_caps.get()));
            }

            gst_helper::gst_unique_ptr<GstCaps> src_caps =
                gst_helper::query_caps(gst_helper::get_static_pad(self->src, "src"));
            GST_INFO_OBJECT(
                self, "caps of src: %" GST_PTR_FORMAT, static_cast<void*>(src_caps.get()));

            // this flag is to used to make the creation of pipelines easier
            // while tcamby1xtransform can convert between many formats
            // it is not always obvious how to create a pipeline with it
            // when using non pimipi cameras
            // for now (2021.01.13) ignore it if that is the case
            self->toggles.use_by1xtransform =
                g_strcmp0("pimipi", self->data->device_type.c_str()) == 0;

            if (self->data->user_caps)
            {
                GstCaps* tmp = gst_caps_intersect(self->data->user_caps.get(), src_caps.get());
                if (tmp == nullptr)
                {
                    GST_ERROR_OBJECT(self,
                                     "The user defined device caps are not supported by the "
                                     "device. User caps are: %s",
                                     gst_helper::to_string(self->data->user_caps).c_str());
                    return GST_STATE_CHANGE_FAILURE;
                }

                self->data->user_caps.reset(tmp);


                // Use the intersected caps instead of the user defined ones.
                // This allows us to work with valid device caps even when
                // the user defines caps like 'video/x-raw,format=BGR' because
                // they want to define the device format but not the resolution etc.
                GST_INFO_OBJECT(self,
                                "Using user defined caps for tcamsrc. User caps are: %s",
                                gst_helper::to_string(self->data->user_caps).c_str());

                self->data->src_caps.reset(find_input_caps(self->data->user_caps.get(),
                                                           self->data->target_caps.get(),
                                                           self->modules,
                                                           self->toggles));
            }
            else
            {
                self->data->src_caps.reset(find_input_caps(
                    src_caps.get(), self->data->target_caps.get(), self->modules, self->toggles));
            }

            if (!self->data->src_caps || gst_caps_is_empty(self->data->src_caps.get()))
            {
                GST_ERROR_OBJECT(self,
                                 "Unable to work with given caps: %s",
                                 gst_helper::to_string(self->data->target_caps).c_str());
                return GST_STATE_CHANGE_FAILURE;
            }

            if (!gst_tcambin_link_elements(self))
            {
                GST_ERROR("Unable to link elements");
                return GST_STATE_CHANGE_FAILURE;
            }
            if (self->pipeline_caps && self->data->src_caps)
            {

                self->data->src_caps.reset(tcam_gst_find_largest_caps(self->data->src_caps.get()));

                g_object_set(self->pipeline_caps, "caps", self->data->src_caps.get(), NULL);
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

            gchar* caps_info_string = g_strdup_printf(
                "Working with src caps: %s", gst_helper::to_string(self->data->src_caps).c_str());

            GstMessage* msg = gst_message_new_info(GST_OBJECT(element), nullptr, caps_info_string);
            g_free(caps_info_string);
            gst_element_post_message(element, msg);

            ret = GST_STATE_CHANGE_NO_PREROLL;
            break;
        }
        case GST_STATE_CHANGE_PLAYING_TO_PLAYING:
        {
            GST_INFO("Changing state: %s", gst_state_change_get_name(trans));

            if (self->must_apply_state)
            {
                apply_state(self, self->data->state);
                self->must_apply_state = FALSE;
            }
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
    ret = GST_ELEMENT_CLASS(parent_class)->change_state(element, trans);
    gst_element_set_locked_state(element, FALSE);

    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        return ret;
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
            self->target_set = FALSE;
            self->elements_linked = FALSE;

            self->data->src_caps.reset();

            break;
        }
        case GST_STATE_CHANGE_READY_TO_NULL:
        {
            gst_tcambin_clear_source(self);
            gst_tcambin_clear_elements(self);
            gst_ghost_pad_set_target(GST_GHOST_PAD(self->data->src_ghost_pad), NULL);
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
    switch (prop_id)
    {
        case PROP_SERIAL:
        {
            if (self->src)
            {
                g_object_get_property(G_OBJECT(self->src), "serial", value);
            }
            else
            {
                g_value_set_string(value, self->data->device_serial.c_str());
            }
            break;
        }
        case PROP_DEVICE_TYPE:
        {

            if (self->src)
            {
                g_object_get_property(G_OBJECT(self->src), "type", value);
            }
            else
            {
                g_value_set_string(value, self->data->device_type.c_str());
            }
            break;
        }
        case PROP_DEVICE_CAPS:
        {
            g_value_set_string(value, gst_helper::to_string(self->data->user_caps).c_str());
            break;
        }
        case PROP_USE_DUTILS:
        {
            g_value_set_boolean(value, self->toggles.use_dutils);
            break;
        }
        case PROP_STATE:
        {
            if (!self->elements_created)
            {
                gst_tcambin_create_elements(GST_TCAMBIN(self));
            }
            if (!self->data->device_serial.empty())
            {
                std::string bla =
                    create_device_settings(self->data->device_serial, TCAM_PROP(self)).c_str();
                g_value_set_string(value, bla.c_str());
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

    switch (prop_id)
    {
        case PROP_SERIAL:
        {
            self->data->device_serial =
                g_value_get_string(value) != nullptr ? g_value_get_string(value) : std::string();
            if (self->src != nullptr)
            {
                GST_INFO("Setting source serial to %s", self->data->device_serial.c_str());
                g_object_set_property(G_OBJECT(self->src), "serial", value);
            }

            break;
        }
        case PROP_DEVICE_TYPE:
        {
            self->data->device_type =
                g_value_get_string(value) != nullptr ? g_value_get_string(value) : std::string();
            if (self->src != nullptr)
            {
                GST_INFO("Setting source device type to %s", g_value_get_string(value));
                g_object_set_property(G_OBJECT(self->src), "type", value);
            }
            break;
        }
        case PROP_DEVICE_CAPS:
        {
            self->data->user_caps.reset(gst_caps_from_string(g_value_get_string(value)));
            break;
        }
        case PROP_USE_DUTILS:
        {
            self->toggles.use_dutils = g_value_get_boolean(value);

            GstState state;

            GstStateChangeReturn ret =
                gst_element_get_state(GST_ELEMENT(self), &state, nullptr, 1000000000);

            // only change elements when we are in a clear state
            // and that state is not higher than READY
            if ((ret = GST_STATE_CHANGE_SUCCESS)
                && (state == GST_STATE_NULL || state == GST_STATE_READY))
            {
                gst_tcambin_clear_elements(self);
                gst_tcambin_create_elements(self);
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
                self->must_apply_state = TRUE;
                self->data->state = g_value_get_string(value) != nullptr ?
                                        g_value_get_string(value) :
                                        std::string();
            }
            else
            {
                self->must_apply_state = FALSE;

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


static bool verify_tcamdutils_version()
{
    std::string dutils_version = get_plugin_version("tcamdutils");

    std::string tcam_version = get_version_major();
    tcam_version += ".";
    tcam_version += get_version_minor();

    // Only check major.minor
    // everything else is potential bugfix
    if (dutils_version.find(tcam_version) == std::string::npos)
    {
        GST_WARNING(
            "Version missmatch for tcamdutils. Auto usage disabled. Found '%s' Required: '%s'",
            dutils_version.c_str(),
            tcam_version.c_str());
        return false;
    }

    return true;
}


static void gst_tcambin_init(GstTcamBin* self)
{
    GST_DEBUG_OBJECT(self, "init");


    self->toggles.use_dutils = TRUE;
    self->elements_linked = FALSE;

    self->data = new tcambin_data;

    auto factory = gst_element_factory_find("tcamdutils");

    if (factory != nullptr)
    {
        self->has_dutils = TRUE;
        gst_object_unref(factory);
        self->toggles.use_dutils = verify_tcamdutils_version();
    }
    else
    {
        self->has_dutils = FALSE;
        self->toggles.use_dutils = FALSE;
    }

    self->src = nullptr;
    self->pipeline_caps = nullptr;
    self->dutils = nullptr;
    self->bayer_transform = nullptr;
    self->exposure = nullptr;
    self->whitebalance = nullptr;
    self->debayer = nullptr;
    self->focus = nullptr;
    self->jpegdec = nullptr;
    self->convert = nullptr;

    // NOTE: the result of gst_ghost_pad_new_no_target is a floating reference that is consumer by gst_element_add_pad
    self->data->src_ghost_pad = gst_ghost_pad_new_no_target("src", GST_PAD_SRC);
    gst_element_add_pad(GST_ELEMENT(self), self->data->src_ghost_pad);

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
        PROP_USE_DUTILS,
        g_param_spec_boolean("use-dutils",
                             "Allow usage of dutils element",
                             "",
                             TRUE,
                             static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

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
