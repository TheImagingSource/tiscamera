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

#include "tcamgstbase.h"
#include "tcamprop.h"
#include <glib-object.h>

#include <unistd.h>

#include <vector>
#include <string>
#include <cstring>

//
// Do not error on format warnings.
// They should happen only in debug statements anyway
//
#pragma GCC diagnostic ignored "-Wformat"


#define gst_tcambin_parent_class parent_class

GST_DEBUG_CATEGORY_STATIC(gst_tcambin_debug);
#define GST_CAT_DEFAULT gst_tcambin_debug


static gboolean gst_tcambin_create_source (GstTcamBin* self);
static gboolean gst_tcambin_create_elements (GstTcamBin* self, const char* serial);
static void gst_tcambin_clear_source (GstTcamBin* self);

//
// introspection interface
//

static GSList* gst_tcam_bin_get_property_names (TcamProp* self);

static gchar* gst_tcam_bin_get_property_type (TcamProp* self,
                                              const gchar* name);

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

static gboolean gst_tcam_bin_set_tcam_property (TcamProp* self,
                                                const gchar* name,
                                                const GValue* value);

static GSList* gst_tcam_bin_get_tcam_menu_entries (TcamProp* self,
                                                   const gchar* name);

static GSList* gst_tcam_bin_get_device_serials (TcamProp* self);

static gboolean gst_tcam_bin_get_device_info (TcamProp* self,
                                              const char* serial,
                                              char** name,
                                              char** identifier,
                                              char** connection_type);

static void gst_tcam_bin_prop_init (TcamPropInterface* iface)
{
    iface->get_property_names = gst_tcam_bin_get_property_names;
    iface->get_property_type = gst_tcam_bin_get_property_type;
    iface->get_property = gst_tcam_bin_get_tcam_property;
    iface->get_menu_entries = gst_tcam_bin_get_tcam_menu_entries;
    iface->set_property = gst_tcam_bin_set_tcam_property;
    iface->get_device_serials = gst_tcam_bin_get_device_serials;
    iface->get_device_info = gst_tcam_bin_get_device_info;
}


G_DEFINE_TYPE_WITH_CODE (GstTcamBin, gst_tcambin, GST_TYPE_BIN,
                         G_IMPLEMENT_INTERFACE (TCAM_TYPE_PROP,
                                                gst_tcam_bin_prop_init))


/**
 * gst_tcam_get_property_type:
 * @self: a #GstTcamBin
 * @name: a #char* identifying the property to query
 *
 * Return the type of a property
 *
 * Returns: (transfer full): A string describing the property type
 */
static gchar* gst_tcam_bin_get_property_type (TcamProp* iface,
                                              const gchar* name)
{
    gchar* ret = NULL;

    GstTcamBin* self = GST_TCAMBIN (iface);

    if (!self->elements_created)
    {
        gst_tcambin_create_elements(GST_TCAMBIN(self), nullptr);
    }

    GstIterator* it = gst_bin_iterate_elements(GST_BIN(self));
    GValue item = G_VALUE_INIT;
    for (GstIteratorResult res = gst_iterator_next(it, &item);
         res == GST_ITERATOR_OK;
         res = gst_iterator_next(it, &item))
    {
        GstElement* element = GST_ELEMENT(g_value_get_object(&item));
        if(TCAM_IS_PROP(element))
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
static GSList* gst_tcam_bin_get_property_names (TcamProp* iface)
{
    GSList* ret = NULL;
    GstTcamBin* self = GST_TCAMBIN(iface);

    if (!self->elements_created)
    {
        gst_tcambin_create_elements(GST_TCAMBIN(self), nullptr);
    }

    GstIterator* it = gst_bin_iterate_elements(GST_BIN(self));
    GValue item = G_VALUE_INIT;
    for (GstIteratorResult res = gst_iterator_next(it, &item);
         res == GST_ITERATOR_OK;
         res = gst_iterator_next(it, &item))
    {
        GstElement* element = GST_ELEMENT(g_value_get_object(&item));
        if(TCAM_IS_PROP(element))
        {
            GSList* prop_names = tcam_prop_get_tcam_property_names(TCAM_PROP(element));
            ret = g_slist_concat(ret, prop_names);
        }
        g_value_unset(&item);
    }

    gst_iterator_free(it);

    return ret;
}


static gboolean gst_tcam_bin_get_tcam_property (TcamProp* iface,
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
        gst_tcambin_create_elements(self, nullptr);
    }

    gboolean ret = false;

    GstIterator* it = gst_bin_iterate_elements(GST_BIN(self));
    GValue item = G_VALUE_INIT;
    for (GstIteratorResult res = gst_iterator_next(it, &item);
         res == GST_ITERATOR_OK;
         res = gst_iterator_next(it, &item))
    {
        GstElement* element = GST_ELEMENT(g_value_get_object(&item));
        if (TCAM_IS_PROP(element))
        {
            if (tcam_prop_get_tcam_property(TCAM_PROP(element),
                                            name, value,
                                            min, max,
                                            def, step,
                                            type,
                                            flags,
                                            category, group))
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
static GSList* gst_tcam_bin_get_tcam_menu_entries (TcamProp* iface,
                                                   const gchar* name)
{
    GstTcamBin* self = GST_TCAMBIN(iface);

    if (!self->elements_created)
    {
        gst_tcambin_create_elements(GST_TCAMBIN(self), nullptr);
    }

    GSList* ret;

    GstIterator* it = gst_bin_iterate_elements(GST_BIN(self));
    GValue item = G_VALUE_INIT;
    for (GstIteratorResult res = gst_iterator_next(it, &item);
         res == GST_ITERATOR_OK;
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

static gboolean gst_tcam_bin_set_tcam_property (TcamProp* iface,
                                                const gchar* name,
                                                const GValue* value)
{
    GstTcamBin* self = GST_TCAMBIN(iface);

    if (!self->elements_created)
    {
        gst_tcambin_create_elements(self, nullptr);
    }

    gboolean ret = false;

    GstIterator* it = gst_bin_iterate_elements(GST_BIN(self));
    GValue item = G_VALUE_INIT;
    for (GstIteratorResult res = gst_iterator_next(it, &item);
         res == GST_ITERATOR_OK;
         res = gst_iterator_next(it, &item))
    {
        GstElement* element = GST_ELEMENT(g_value_get_object(&item));
        if (TCAM_IS_PROP(element))
        {
            if (tcam_prop_set_tcam_property(TCAM_PROP(element),
                                            name,
                                            value))
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


static GSList* gst_tcam_bin_get_device_serials (TcamProp* self __attribute((__unused__)))
{
    GstElement* src = gst_element_factory_make("tcamsrc", nullptr);
    if (src == nullptr)
    {
        g_critical("Failed to create a tcamsrc");
        return nullptr;
    }

    GSList *serials = tcam_prop_get_device_serials(TCAM_PROP(src));

    gst_object_unref(src);

    return serials;
}


static gboolean gst_tcam_bin_get_device_info (TcamProp* self __attribute((__unused__)),
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
    gboolean ret = tcam_prop_get_device_info(TCAM_PROP(src),
                                             serial,
                                             name,
                                             identifier,
                                             connection_type);
    g_object_unref(src);

    return ret;
}


//
// gstreamer module
//

static void gst_tcambin_class_init (GstTcamBinClass* klass);
static void gst_tcambin_init (GstTcamBin* klass);

enum
{
    PROP_0,
    PROP_SERIAL,
    PROP_DEVICE_CAPS,
    PROP_USE_DUTILS
};

static GstStaticPadTemplate src_template = GST_STATIC_PAD_TEMPLATE("src",
                                                                   GST_PAD_SRC,
                                                                   GST_PAD_ALWAYS,
                                                                   GST_STATIC_CAPS_ANY);


static gboolean gst_tcambin_create_source (GstTcamBin* self)
{
    gst_tcambin_clear_source(self);

    GST_DEBUG("Creating source...");

    self->src = gst_element_factory_make("tcamsrc", "tcambin-source");
    gst_bin_add(GST_BIN(self), self->src);

    if (self->device_serial != nullptr)
    {
        GST_INFO("Setting source serial to %s", self->device_serial);
        g_object_set(G_OBJECT(self->src), "serial", self->device_serial, NULL);
    }

    self->src_caps = gst_pad_query_caps(gst_element_get_static_pad(self->src, "src"), NULL);
    GST_INFO("caps of src: %" GST_PTR_FORMAT, static_cast<void*>(self->src_caps));

    return TRUE;
}


static void gst_tcambin_clear_source (GstTcamBin* self)
{
    if (self->src)
    {
        gst_element_set_state(self->src, GST_STATE_NULL);
        gst_bin_remove(GST_BIN(self), self->src);
        self->src = nullptr;
    }
}


static void gst_tcambin_clear_elements (GstTcamBin* self)
{

    auto remove_element = [=] (GstElement** element)
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
    if (self->biteater)
    {
        remove_element(&self->biteater);
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


static gboolean create_and_add_element (GstElement** element,
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


static gboolean gst_tcambin_create_elements (GstTcamBin* self,
                                             const char* serial=nullptr)
{

    if (self->elements_created)
    {
        return TRUE;
    }
    GST_INFO("creating elements");

    if (self->src == nullptr)
    {
        if (serial)
        {
            self->device_serial = g_strdup(serial);
        }

        gst_tcambin_create_source(self);
    }

    self->pipeline_caps = gst_element_factory_make("capsfilter", "tcambin-src_caps");

    if (self->pipeline_caps == nullptr)
    {
        GST_ERROR("Could not create internal pipeline caps. Aborting");
        return FALSE;
    }

    gst_bin_add(GST_BIN(self), self->pipeline_caps);

    auto send_missing_element_msg = [self] (const std::string& element_name)
        {
            std::string msg_string = "Could not create element '" + element_name + "'.";
            GError* err = g_error_new(GST_CORE_ERROR, GST_CORE_ERROR_MISSING_PLUGIN, msg_string.c_str());
            GstMessage* msg = gst_message_new_error(GST_OBJECT(self), err, msg_string.c_str());
            gst_element_post_message(GST_ELEMENT(self), msg);
            g_error_free(err);
        };

    if (self->has_dutils && self->use_dutils)
    {
        if (!create_and_add_element(&self->dutils,
                                    "tcamdutils", "tcambin-dutils",
                                    GST_BIN(self)))
        {
            send_missing_element_msg("tcamdutils");
            return FALSE;
        }

        if (!create_and_add_element(&self->biteater,
                                    "tcambiteater", "tcambin-biteater",
                                    GST_BIN(self)))
        {
            send_missing_element_msg("tcambiteater");
            return FALSE;
        }
        // go to finish and do not create other elements
        goto finished_element_creation;
    }


    // the following elements only support mono or bayer
    // security check to prevent faulty pipelines

    //if (gst_caps_are_bayer_only(self->src_caps)
    if (contains_bayer(self->src_caps)
        || tcam_gst_raw_only_has_mono(self->src_caps))
    {
        if (tcam_prop_get_tcam_property_type(TCAM_PROP(self->src), "Exposure Auto") == nullptr)
        {
            if (!create_and_add_element(&self->exposure,
                                        "tcamautoexposure", "tcambin-exposure",
                                        GST_BIN(self)))
            {
                send_missing_element_msg("tcamautoexposure");
                return FALSE;
            }
        }

        if (tcam_prop_get_tcam_property_type(TCAM_PROP(self->src), "Focus") != nullptr
            && tcam_prop_get_tcam_property_type(TCAM_PROP(self->src), "Auto Focus") == nullptr)
        {
            if (!create_and_add_element(&self->focus,
                                        "tcamautofocus", "tcambin-focus",
                                        GST_BIN(self)))
            {
                send_missing_element_msg("tcamautofocus");
                return FALSE;
            }
        }
    }

    // always add whitebalance when using bayer
    // we want it when debayering
    // users may still want it when asking for bayer
    if (contains_bayer(self->src_caps))
    {
        // use this to see if the device already has the feature
        if (tcam_prop_get_tcam_property_type(TCAM_PROP(self->src), "Whitebalance Auto") == nullptr)
        {
            if (!create_and_add_element(&self->whitebalance,
                                        "tcamwhitebalance", "tcambin-whitebalance",
                                        GST_BIN(self)))
            {
                send_missing_element_msg("tcamwhitebalance");
                return FALSE;
            }
        }
    }

    if (!create_and_add_element(&self->debayer,
                                "bayer2rgb", "tcambin-debayer",
                                GST_BIN(self)))
    {
        send_missing_element_msg("bayer2rgb");
        return FALSE;
    }

    if (!create_and_add_element(&self->jpegdec,
                                "jpegdec", "tcambin-jpegdec",
                                GST_BIN(self)))
    {
        send_missing_element_msg("jpegdec");
        return FALSE;
    }

// videoconvert can be needed by non-dutils and dutils pipelines
// thus include it after the label
finished_element_creation:

    // this is needed to allow for conversions such as
    // GRAY8 to BGRx that can exist when device-caps are set
    if (!create_and_add_element(&self->convert,
                                "videoconvert", "tcambin-convert",
                                GST_BIN(self)))
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
static gboolean gst_tcambin_link_elements (GstTcamBin* self)
{
    GST_INFO("Linking elements");

    if (self->elements_linked)
    {
        GST_INFO("Already linked");
        return TRUE;
    }

    if (self->target_caps == NULL)
    {
        GST_ERROR("Unknown target caps. Aborting.");
        return FALSE;
    }

    if (self->pipeline_caps == nullptr)
    {
        GST_ERROR("Could not create internal pipeline caps. Aborting");
        return FALSE;
    }

    if (self->src_caps == nullptr)
    {
        GST_ERROR("Could not find valid caps. Aborting pipeline creation.");
        return FALSE;
    }

    if (!gst_caps_is_fixed(self->src_caps))
    {
        std::string sc = gst_caps_to_string(self->src_caps);
        self->src_caps = tcam_gst_find_largest_caps(self->src_caps);
        GST_INFO("Caps were not fixed. Reduced '%s' to: '%s'",
                 sc.c_str(),
                 gst_caps_to_string(self->src_caps));
    }
    else
    {
        GST_INFO("Caps are fixed. Using caps for src: %s", gst_caps_to_string(self->src_caps));
    }

    g_object_set(self->pipeline_caps, "caps", self->src_caps, NULL);

    gboolean ret = gst_element_link(self->src,
                                    self->pipeline_caps);

    if (!ret)
    {
        GST_ERROR("Unable to link src and capsfilter.");
        return ret;
    }

    // helper function to post error messages to GstBus
    auto send_linking_element_msg = [self] (const std::string& element_name)
        {
            std::string msg_string = "Could not create element '" + element_name + "'.";
            GError* err = g_error_new(GST_CORE_ERROR, GST_CORE_ERROR_MISSING_PLUGIN, msg_string.c_str());
            GstMessage* msg = gst_message_new_error(GST_OBJECT(self), err, msg_string.c_str());
            gst_element_post_message(GST_ELEMENT(self), msg);
            g_error_free(err);
        };

    // helper function to link elements
    auto link_elements = [self] (bool condition,
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

                gboolean ret = gst_element_link(*previous_element, *element);
                if (!ret)
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
    gchar* capsstr = gst_caps_to_string(self->src_caps);
    pipeline_description += capsstr;
    g_free (capsstr);

    GstElement* previous_element = self->pipeline_caps;

    if (self->has_dutils && self->needs_dutils)
    {
        if (!link_elements((self->has_dutils && self->needs_dutils),
                           &previous_element,
                           &self->dutils,
                           pipeline_description,
                           "tcamdutils"))
        {
            send_linking_element_msg("tcamdutils");
            return FALSE;
        }

        if (!link_elements(self->needs_biteater,
                           &previous_element,
                           &self->biteater,
                           pipeline_description,
                           "tcambiteater"))
        {
                send_linking_element_msg("tcambiteater");
                return FALSE;
        }
        // goto finished_element_linking;

    }

    // the following elements only support mono or bayer
    // security check to prevent faulty pipelines

    if (gst_caps_are_bayer_only(self->src_caps)
        || tcam_gst_raw_only_has_mono(self->src_caps))
    {
        if (tcam_prop_get_tcam_property_type(TCAM_PROP(self->src), "Exposure Auto") == nullptr)
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
            && tcam_prop_get_tcam_property_type(TCAM_PROP(self->src), "Auto Focus") == nullptr)
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
    if (contains_bayer(self->src_caps))
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

    if (!link_elements(self->needs_debayer,
                       &previous_element,
                       &self->debayer,
                       pipeline_description,
                       "bayer2rgb"))
    {
        send_linking_element_msg("bayer2rgb");
        return FALSE;
    }

    if (!link_elements(self->needs_jpegdec,
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
    if (!link_elements(self->needs_videoconvert,
                       &previous_element,
                       &self->convert,
                       pipeline_description,
                       "videoconvert"))
    {
        send_linking_element_msg("videoconvert");
        return FALSE;
    }

// finished_element_linking:

    GST_INFO("Using %s as exit element for internal pipeline", gst_element_get_name(previous_element));
    self->target_pad = gst_element_get_static_pad(previous_element, "src");
    GST_INFO("Internal pipeline: %s", pipeline_description.c_str());

    if (gst_caps_is_any(self->target_caps) || gst_caps_is_empty(self->target_caps))
    {
        gst_caps_unref(self->target_caps);
        self->target_caps = gst_caps_copy(self->src_caps);
    }

    GST_INFO("Working with exit caps: %s", gst_caps_to_string(self->target_caps));
    self->elements_linked = TRUE;

    return TRUE;
}


/**
 * Generate GstCaps that contains all possible caps from src, bayer2rgb and videoconvert
 * in case of an error: return nullptr
 */
static GstCaps* generate_all_caps (GstTcamBin* self)
{
    GstCaps* incoming = gst_pad_query_caps(gst_element_get_static_pad(self->src, "src"), NULL);

    // always can be passed through
    GstCaps* all_caps = gst_caps_copy(incoming);

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

        if (gst_structure_get_field_type (struc, "format") == G_TYPE_STRING)
        {
            const char* string = gst_structure_get_string (struc, "format");

            if (tcam_gst_is_bayer8_string(string))
            {
                const GValue* width = gst_structure_get_value(struc, "width");
                const GValue* height = gst_structure_get_value(struc, "height");
                const GValue* framerate = gst_structure_get_value(struc, "framerate");

                GstStructure* s = gst_structure_new_empty("video/x-raw");

                std::string tmp_format_string;
                GstCaps* tmp = get_caps_from_element_name("bayer2rgb", "src");

                GstStructure* tmp_struc = gst_structure_copy(gst_caps_get_structure(tmp, 0));
                gst_structure_set_value(s, "format", gst_structure_get_value(tmp_struc, "format"));

                gst_structure_set_value(s, "width", width);
                gst_structure_set_value(s, "height", height);
                gst_structure_set_value(s, "framerate", framerate );

                gst_caps_append_structure(all_caps, s);

                gst_caps_unref(tmp);
            }
            else if (!self->has_dutils &&
                     (tcam_gst_is_bayer12_string(string)
                      || tcam_gst_is_bayer12_packed_string(string)
                      || tcam_gst_is_bayer16_string(string)))
            {
                gst_caps_append_structure(to_remove, gst_structure_copy(struc));
            }
        }
    }

    gst_caps_unref(incoming);

    // TODO: find alternative
    // caps_substract implicitly calls gst_caps_simplify
    // this causes 'weird' caps like video/x-raw,format=rggb,width={2448, 2048},height=2048
    // these are hard to parse and should be avoided.
    // all_caps = gst_caps_subtract(all_caps, to_remove);

    gst_caps_unref(to_remove);
    return all_caps;
}


void set_target_pad (GstTcamBin* self, GstPad* pad __attribute__((unused)))
{

    gst_ghost_pad_set_target(GST_GHOST_PAD(self->pad), NULL);

    if (self->target_set == FALSE)
    {
        if (self->target_pad == nullptr)
        {
            GST_ERROR("target_pad not defined");
        }
        else
        {
            if (!gst_ghost_pad_set_target(GST_GHOST_PAD(self->pad), self->target_pad))
            {
                GST_ERROR("Could not set target for ghostpad.");
            }
        }
        self->target_set = TRUE;
    }
}


static GstStateChangeReturn gst_tcam_bin_change_state (GstElement* element,
                                                       GstStateChange trans)
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
                g_object_set(self->src, "serial", self->device_serial, NULL);
            }

            gst_element_set_state(self->src, GST_STATE_READY);

            if (self->out_caps)
            {
                g_object_unref(self->out_caps);
            }

            self->out_caps = gst_element_factory_make("capsfilter", "tcambin-out_caps");

            gst_ghost_pad_set_target(GST_GHOST_PAD(self->pad),
                                     gst_element_get_static_pad(self->out_caps, "src"));
            GstCaps* all_caps = generate_all_caps(self);
            g_object_set(self->out_caps, "caps", all_caps, NULL);
            gst_caps_unref(all_caps);

            if (!gst_tcambin_create_elements(self))
            {
                GST_ERROR("Error while creating elements");
            }

            break;
        }
        case GST_STATE_CHANGE_READY_TO_PAUSED:
        {
            GST_INFO("READY_TO_PAUSED");

            GstPad* sinkpad = gst_pad_get_peer(self->pad);

            if (sinkpad == nullptr)
            {
                self->target_caps = gst_caps_new_empty();
            }
            else
            {
                GstElement* par = gst_pad_get_parent_element(sinkpad);

                if (strcmp(g_type_name(gst_element_factory_get_element_type(gst_element_get_factory(par))),
                           "GstCapsFilter") == 0)
                {
                    if (self->target_caps)
                    {
                        gst_caps_unref(self->target_caps);
                        self->target_caps = nullptr;
                    }

                    g_object_get(par, "caps", &self->target_caps, NULL);
                }
                else
                {
                    self->target_caps = gst_pad_query_caps (sinkpad, NULL);
                }
                gst_object_unref(par);
                GST_INFO("caps of sink: %" GST_PTR_FORMAT, static_cast<void*>(self->target_caps));
            }

            GstCaps* src_caps = gst_pad_query_caps(gst_element_get_static_pad(self->src, "src"), NULL);
            GST_INFO("caps of src: %" GST_PTR_FORMAT, static_cast<void*>(src_caps));

            if (self->src_caps != nullptr)
            {
                gst_caps_unref(self->src_caps);
            }

            if (self->user_caps)
            {
                GstCaps* tmp = gst_caps_intersect(self->user_caps, src_caps);
                if (tmp == nullptr)
                {
                    GST_ERROR("The user defined device caps are not supported by the device. User caps are: %s",
                              gst_caps_to_string(self->user_caps));
                    return GST_STATE_CHANGE_FAILURE;
                }

                // Use the intersected caps instead of the user defined ones.
                // This allows us to work with valid device caps even when
                // the user defines caps like 'video/x-raw,format=BGR' because
                // they want to define the device format but not the resolution etc.
                GST_INFO("Using user defined caps for tcamsrc. User caps are: %s",
                         gst_caps_to_string(tmp));
                gst_caps_unref(self->user_caps);
                self->user_caps = tmp;

                self->src_caps = find_input_caps(self->user_caps, self->target_caps,
                                                 self->needs_debayer,
                                                 self->needs_videoconvert,
                                                 self->needs_jpegdec,
                                                 self->needs_dutils,
                                                 self->needs_biteater,
                                                 self->use_dutils);
            }
            else
            {
                self->src_caps = find_input_caps(src_caps, self->target_caps,
                                                 self->needs_debayer,
                                                 self->needs_videoconvert,
                                                 self->needs_jpegdec,
                                                 self->needs_dutils,
                                                 self->needs_biteater,
                                                 self->use_dutils);
            }

            gst_caps_unref(src_caps);

            if (!self->src_caps || gst_caps_is_empty(self->src_caps))
            {
                GST_ERROR("Unable to work with given caps: %s",
                          gst_caps_to_string(self->target_caps));
                return GST_STATE_CHANGE_FAILURE;
            }

            if (!gst_tcambin_link_elements(self))
            {
                GST_ERROR("Unable to link elements");
                return GST_STATE_CHANGE_FAILURE;
            }
            if (self->pipeline_caps && self->src_caps)
            {

                self->src_caps = tcam_gst_find_largest_caps(self->src_caps);

                g_object_set(self->pipeline_caps, "caps", self->src_caps, NULL);
            }
            set_target_pad(self, self->target_pad);

            /*
             * We send this message as a means of always notifying
             * applications of the output caps we use.
             * This is done for the case where no output caps are given
             * and the bin selected the caps that shall go out.
             * With this message applications have a way of displaying
             * the selected caps, no matter what.
             */

            gchar* caps_info_string = g_strdup_printf("Working with src caps: %s",
                                                      gst_caps_to_string(self->src_caps));

            GstMessage* msg = gst_message_new_info(GST_OBJECT(element),
                                                   nullptr,
                                                   caps_info_string);
            g_free(caps_info_string);
            gst_element_post_message(element, msg);

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

    ret = GST_ELEMENT_CLASS(parent_class)->change_state(element, trans);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        return ret;
    }

    switch(trans)
    {
        case GST_STATE_CHANGE_PAUSED_TO_READY:
        {
            self->target_set = FALSE;

            if (self->src_caps)
            {
                gst_caps_unref(self->src_caps);
                self->src_caps = nullptr;
            }

            break;
        }
        case GST_STATE_CHANGE_READY_TO_NULL:
        {
            gst_tcambin_clear_source(self);
            gst_tcambin_clear_elements(self);
            gst_ghost_pad_set_target(GST_GHOST_PAD(self->pad), NULL);
            break;
        }
        default:
        {
            break;
        }
    }

    return ret;
}


static void gst_tcambin_get_property (GObject* object,
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
            } else {
                g_value_set_string(value, self->device_serial);
            }
            break;
        }
        case PROP_DEVICE_CAPS:
        {
            g_value_set_string(value, gst_caps_to_string(self->user_caps));
            break;
        }
        case PROP_USE_DUTILS:
        {
            g_value_set_boolean(value, self->use_dutils);
            break;
        }
        default:
        {
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
        }
    }
}


static void gst_tcambin_set_property (GObject* object,
                                      guint prop_id,
                                      const GValue* value,
                                      GParamSpec* pspec)
{
    GstTcamBin* self = GST_TCAMBIN(object);

    switch (prop_id)
    {
        case PROP_SERIAL:
        {
            self->device_serial = g_strdup(g_value_get_string(value));
            if (self->src != nullptr)
            {
                GST_INFO("Setting source serial to %s", self->device_serial);
                g_object_set(G_OBJECT(self->src), "serial", self->device_serial, NULL);
            }

            break;
        }
        case PROP_DEVICE_CAPS:
        {
            self->user_caps = gst_caps_from_string(g_value_get_string(value));
            break;
        }
        case PROP_USE_DUTILS:
        {
            self->use_dutils = g_value_get_boolean(value);

            GstState state;

            GstStateChangeReturn ret = gst_element_get_state(GST_ELEMENT(self),
                                                             &state, nullptr, 1000000000);

            // only change elements when we are in a clear state
            // and that state is not higher than READY
            if ((ret = GST_STATE_CHANGE_SUCCESS) &&
                (state == GST_STATE_NULL || state == GST_STATE_READY))
            {
                gst_tcambin_clear_elements(self);
                gst_tcambin_create_elements(self);
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


static void gst_tcambin_init (GstTcamBin* self)
{
    GST_DEBUG("init");

    self->use_dutils = TRUE;
    self->needs_biteater = TRUE;

    auto factory = gst_element_factory_find("tcamdutils");

    if (factory != nullptr)
    {
        self->has_dutils = TRUE;
        gst_object_unref(factory);
    }
    else
    {
        self->has_dutils = FALSE;
        self->use_dutils = FALSE;
    }

    self->src = nullptr;
    self->pipeline_caps = nullptr;
    self->dutils = nullptr;
    self->biteater = nullptr;
    self->exposure = nullptr;
    self->whitebalance = nullptr;
    self->debayer = nullptr;
    self->focus = nullptr;
    self->jpegdec = nullptr;
    self->convert = nullptr;
    self->out_caps = nullptr;

    self->pad = gst_ghost_pad_new_no_target("src", GST_PAD_SRC);
    gst_element_add_pad(GST_ELEMENT(self), self->pad);

    GST_OBJECT_FLAG_SET(self, GST_ELEMENT_FLAG_SOURCE);
}


static void gst_tcambin_finalize (GObject* object)
{
    G_OBJECT_CLASS (parent_class)->finalize(object);
}


static void gst_tcambin_dispose (GstTcamBin* self)
{
    GST_DEBUG("dispose");

    gst_tcambin_clear_source(self);
    gst_tcambin_clear_elements(self);
    if (self->pad)
    {
        gst_element_remove_pad(GST_ELEMENT(self), self->pad);
    }

    G_OBJECT_CLASS(parent_class)->dispose((GObject*) self);
}


static void gst_tcambin_class_init (GstTcamBinClass* klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS (klass);
    GstElementClass* element_class = GST_ELEMENT_CLASS (klass);

    object_class->dispose = (GObjectFinalizeFunc) gst_tcambin_dispose;
    object_class->finalize = gst_tcambin_finalize;
    object_class->set_property = gst_tcambin_set_property;
    object_class->get_property = gst_tcambin_get_property;

    element_class->change_state = GST_DEBUG_FUNCPTR(gst_tcam_bin_change_state);

    g_object_class_install_property(object_class,
                                    PROP_SERIAL,
                                    g_param_spec_string("serial",
                                                        "Camera serial",
                                                        "Serial of the camera that shall be used",
                                                        "",
                                                        static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));


    g_object_class_install_property(object_class,
                                    PROP_DEVICE_CAPS,
                                    g_param_spec_string("device-caps",
                                                        "Device Caps",
                                                        "GstCaps the tcamsrc shall use",
                                                        "",
                                                        static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

   g_object_class_install_property(object_class,
                                    PROP_USE_DUTILS,
                                    g_param_spec_boolean("use-dutils",
                                                        "Allow usage of dutils element",
                                                        "",
                                                        TRUE,
                                                        static_cast<GParamFlags>(G_PARAM_READWRITE)));


    gst_element_class_add_pad_template (element_class,
                                        gst_static_pad_template_get (&src_template));

    gst_element_class_set_details_simple (element_class,
                                          "Tcam Video Bin",
                                          "Source/Video",
                                          "Tcam based bin",
                                          "The Imaging Source <support@theimagingsource.com>");
}


static gboolean plugin_init (GstPlugin* plugin)
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

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
                   GST_VERSION_MINOR,
                   tcambin,
                   "Tcam Video Bin",
                   plugin_init,
                   TCAMBIN_VERSION,
                   "Proprietary",
                   "tcambin",
                   "theimagingsource.com")
