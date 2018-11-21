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


static gboolean gst_tcambin_create_elements (GstTcamBin* self,
                                             const gchar *serial);
static gboolean gst_tcambin_link_elements(GstTcamBin *self);
static GstCaps* generate_all_caps (GstTcamBin* self);

//
// introspection interface
//

static GSList* gst_tcambin_get_property_names (TcamProp* self);

static gchar* gst_tcambin_get_property_type (TcamProp* self,
                                              const gchar* name);

static gboolean gst_tcambin_get_tcam_property(TcamProp* self,
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

static gboolean gst_tcambin_set_tcam_property (TcamProp* self,
                                                const gchar* name,
                                                const GValue* value);

static GSList* gst_tcambin_get_tcam_menu_entries (TcamProp* self,
                                                   const gchar* name);

static GSList* gst_tcambin_get_device_serials (TcamProp* self);

static gboolean gst_tcambin_get_device_info (TcamProp* self,
                                              const char* serial,
                                              char** name,
                                              char** identifier,
                                              char** connection_type);

static void gst_tcambin_prop_init (TcamPropInterface* iface)
{
    iface->get_property_names = gst_tcambin_get_property_names;
    iface->get_property_type = gst_tcambin_get_property_type;
    iface->get_property = gst_tcambin_get_tcam_property;
    iface->get_menu_entries = gst_tcambin_get_tcam_menu_entries;
    iface->set_property = gst_tcambin_set_tcam_property;
    iface->get_device_serials = gst_tcambin_get_device_serials;
    iface->get_device_info = gst_tcambin_get_device_info;
}


G_DEFINE_TYPE_WITH_CODE (GstTcamBin, gst_tcambin, GST_TYPE_BIN,
                         G_IMPLEMENT_INTERFACE (TCAM_TYPE_PROP,
                                                gst_tcambin_prop_init))


static std::vector<GstElement **>gst_tcambin_get_internal_element_refs(GstTcamBin *self)
{
    GstElement **elements[] = {
        &self->src,
        &self->dutils,
        &self->biteater,
        &self->exposure,
        &self->whitebalance,
        &self->debayer,
        &self->focus,
        &self->jpegdec,
        &self->convert,
        &self->out_caps,
        &self->pipeline_caps
    };
    std::vector<GstElement**> elemv(elements, elements + sizeof(elements)/sizeof(elements[0]));
    return elemv;
}


/**
 * gst_tcam_get_property_type:
 * @self: a #GstTcamBin
 * @name: a #char* identifying the property to query
 *
 * Return the type of a property
 *
 * Returns: (transfer full): A string describing the property type
 */
static gchar* gst_tcambin_get_property_type(TcamProp* iface,
                                            const gchar* name)
{
    gchar* ret = NULL;

    GstTcamBin* self = GST_TCAMBIN (iface);

    if (!self->elements_created)
    {
        gst_tcambin_create_elements(GST_TCAMBIN(self), nullptr);
    }

    GstIterator *it = gst_bin_iterate_elements(GST_BIN(self));
    GValue item = G_VALUE_INIT;
    for (GstIteratorResult res = gst_iterator_next(it, &item);
         res == GST_ITERATOR_OK;
         res = gst_iterator_next(it, &item))
    {
        GstElement *element = GST_ELEMENT(g_value_get_object(&item));
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
 * gst_tcambin_get_property_names:
 * @self: a #GstTcamBin
 *
 * Return a list of property names
 *
 * Returns: (element-type utf8) (transfer full): list of property names
 */
static GSList* gst_tcambin_get_property_names (TcamProp* iface)
{
    GSList* ret = NULL;
    GstTcamBin* self = GST_TCAMBIN(iface);

    if (!self->elements_created)
    {
        gst_tcambin_create_elements(GST_TCAMBIN(self), nullptr);
    }

    GstIterator *it = gst_bin_iterate_elements(GST_BIN(self));
    GValue item = G_VALUE_INIT;
    for (GstIteratorResult res = gst_iterator_next(it, &item);
         res == GST_ITERATOR_OK;
         res = gst_iterator_next(it, &item))
    {
        GstElement *element = GST_ELEMENT(g_value_get_object(&item));
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


static gboolean gst_tcambin_get_tcam_property (TcamProp* iface,
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

    GstIterator *it = gst_bin_iterate_elements(GST_BIN(self));
    GValue item = G_VALUE_INIT;
    for (GstIteratorResult res = gst_iterator_next(it, &item);
         res == GST_ITERATOR_OK;
         res = gst_iterator_next(it, &item))
    {
        GstElement *element = GST_ELEMENT(g_value_get_object(&item));
        if(TCAM_IS_PROP(element))
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


static gboolean gst_tcambin_set_tcam_property (TcamProp* iface,
                                                const gchar* name,
                                                const GValue* value)
{
    GstTcamBin* self = GST_TCAMBIN(iface);

    if (!self->elements_created)
    {
        gst_tcambin_create_elements(self, nullptr);
    }

    gboolean ret = false;

    GstIterator *it = gst_bin_iterate_elements(GST_BIN(self));
    GValue item = G_VALUE_INIT;
    for (GstIteratorResult res = gst_iterator_next(it, &item);
         res == GST_ITERATOR_OK;
         res = gst_iterator_next(it, &item))
    {
        GstElement *element = GST_ELEMENT(g_value_get_object(&item));
        if(TCAM_IS_PROP(element))
        {
            if (tcam_prop_set_tcam_property(TCAM_PROP(element),
                                            name, value))
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
 * gst_tcambin_get_tcam_menu_entries:
 * @self: a #GstTcamBin
 * @name: a #char*
 *
 * Return a list of property names
 *
 * Returns: (element-type utf8) (transfer full): a #GSList
 */
static GSList* gst_tcambin_get_tcam_menu_entries (TcamProp* iface,
                                                   const gchar* name)
{
    GstTcamBin* self = GST_TCAMBIN(iface);

    if (!self->elements_created)
    {
        gst_tcambin_create_elements(GST_TCAMBIN(self), nullptr);
    }

    GSList * ret;

    GstIterator *it = gst_bin_iterate_elements(GST_BIN(self));
    GValue item = G_VALUE_INIT;
    for (GstIteratorResult res = gst_iterator_next(it, &item);
         res == GST_ITERATOR_OK;
         res = gst_iterator_next(it, &item))
    {
        GstElement *element = GST_ELEMENT(g_value_get_object(&item));
        if(TCAM_IS_PROP(element))
        {
            ret = tcam_prop_get_tcam_menu_entries(TCAM_PROP(element), name);
            if (ret)
                break;
        }
        g_value_unset(&item);
    }

    gst_iterator_free(it);

    return ret;
}


static GSList* gst_tcambin_get_device_serials (TcamProp* self __attribute((__unused__)))
{
    GstElement *src = gst_element_factory_make("tcamsrc", nullptr);
    if (src == nullptr)
    {
        g_critical("Failed to create a tcamsrc");
        return nullptr;
    }

    GSList *serials = tcam_prop_get_device_serials(TCAM_PROP(src));

    gst_object_unref(src);

    return serials;
}


static gboolean gst_tcambin_get_device_info (TcamProp* self __attribute((__unused__)),
                                              const char* serial,
                                              char** name,
                                              char** identifier,
                                              char** connection_type)
{
    GstElement *src = gst_element_factory_make("tcamsrc", nullptr);
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


static void gst_tcambin_clear_elements(GstTcamBin* self)
{

    auto remove_element = [=] (GstElement** element)
        {
            if (!*element)
                return;

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

    auto elemv = gst_tcambin_get_internal_element_refs(self);

    for(GstElement ** element: elemv)
    {
        remove_element(element);
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
        g_critical("Failed to create element '%s", factory_name);
        return FALSE;
    }
    return TRUE;
}


static gboolean gst_tcambin_create_elements (GstTcamBin* self,
                                             const gchar *serial)
{
    auto send_missing_element_msg = [self] (const std::string& element_name)
        {
            std::string msg_string = "Could not create element '" + element_name + "'.";
            GError* err = g_error_new(GST_CORE_ERROR, GST_CORE_ERROR_MISSING_PLUGIN, msg_string.c_str());
            GstMessage* msg = gst_message_new_error(GST_OBJECT(self), err, msg_string.c_str());
            gst_element_post_message(GST_ELEMENT(self), msg);
            g_error_free(err);
        };


    if (self->elements_created)
    {
        g_critical("Trying to create elements while elements already created");
        return TRUE;
    }
    GST_INFO("creating elements");

    if (self->src == nullptr)
    {
        if (!create_and_add_element(&self->src,
                                    "tcamsrc", "tcambin-source",
                                    GST_BIN(self)))
        {
            send_missing_element_msg("tcamsrc");
            return FALSE;
        }
        g_object_set(self->src, "serial", serial, nullptr);
    }

    self->pipeline_caps = gst_element_factory_make("capsfilter",
                                                   "tcambin-src_caps");

    if (self->pipeline_caps == nullptr)
    {
        GST_ERROR("Could not create internal pipeline caps. Aborting");
        return FALSE;
    }

    gst_bin_add(GST_BIN(self), self->pipeline_caps);

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
    } else {
        GstPad *srcpad = gst_element_get_static_pad(self->src, "src");
        GstCaps *srccaps = gst_pad_query_caps(srcpad, NULL);

        if (contains_bayer(srccaps)
            || tcam_gst_raw_only_has_mono(srccaps))
        {
            if (tcam_prop_get_tcam_property_type(TCAM_PROP(self->src), "Exposure Auto") == nullptr)
            {
                if (!create_and_add_element(&self->exposure,
                                            "tcamautoexposure",
                                            "tcambin-exposure",
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
                                            "tcamautofocus",
                                            "tcambin-focus",
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
        if (contains_bayer(srccaps))
        {
            // use this to see if the device already has the feature
            if (tcam_prop_get_tcam_property_type(TCAM_PROP(self->src), "Whitebalance Auto") == nullptr)
            {
                if (!create_and_add_element(&self->whitebalance,
                                            "tcamwhitebalance",
                                            "tcambin-whitebalance",
                                            GST_BIN(self)))
                {
                    send_missing_element_msg("tcamwhitebalance");
                    return FALSE;
                }
            }
            if (!create_and_add_element(&self->debayer,
                                        "bayer2rgb",
                                        "tcambin-debayer",
                                        GST_BIN(self)))
            {
                send_missing_element_msg("bayer2rgb");
                return FALSE;
            }
        }

        if (contains_jpeg(srccaps))
        {
            if (!create_and_add_element(&self->jpegdec,
                                        "jpegdec",
                                        "tcambin-jpegdec",
                                        GST_BIN(self)))
            {
                send_missing_element_msg("jpegdec");
                return FALSE;
            }
        }

        gst_caps_unref(srccaps);
        gst_object_unref(srcpad);
    }

    GstCaps *caps = generate_all_caps(self);
    self->out_caps = gst_element_factory_make("capsfilter",
                                              "tcambin-out_caps");
    g_object_set(self->out_caps, "caps", caps, NULL);
    gst_caps_unref(caps);
    gst_bin_add(GST_BIN(self), self->out_caps);

    self->elements_created = TRUE;

    // Initial link
    gst_tcambin_link_elements(self);
    return TRUE;
}


static void gst_tcambin_unlink_all(GstTcamBin *self)
{
    GST_DEBUG("unlink");
    GstIterator *it = gst_bin_iterate_elements(GST_BIN(self));
    GValue item = G_VALUE_INIT;
    for (GstIteratorResult res = gst_iterator_next(it, &item);
         res == GST_ITERATOR_OK;
         res = gst_iterator_next(it, &item))
    {
        GstElement *element = GST_ELEMENT(g_value_get_object(&item));
        GstPad *srcpad = gst_element_get_static_pad(element, "src");
        if (gst_pad_is_linked(srcpad))
        {
            GstPad *sinkpad = gst_pad_get_peer(srcpad);
            gst_pad_unlink(srcpad, sinkpad);
            gst_object_unref(sinkpad);
        }
        gst_object_unref(srcpad);
        g_value_unset(&item);
    }

    gst_iterator_free(it);
}


/*
    Link our elements.

    Scenarios:

    1.) With DUTILS:
      - target caps and dutils intersect:
        - link tcamsrc -> dutils
        - link dutils to ghost pad -> DONE

      - target caps and biteater intersect:
        - link tcamsrc -> dutils -> biteater
        - link biteater to ghost pad -> DONE

      - target could not be linked, FAIL

    2.) Without DUTILS:
      - target caps is GRAY8 or bayer?
        - link tcamsrc -> tcamautoexposure -> tcamautofocus  IF elements are present

      - target caps is bayer?
        - link tcamwhitebalance  IF element is present

      - target caps and tcamsrc caps intersect?
        - link last element to ghost pad -> DONE

      - target caps and bayer2rgb intersect?
        - srccaps and bayer2rgb intersect?
          - link tcamsrc -> tcamautoexposure -> tcamwhitebalance -> tcamautofocus  IF elements are present
          - link last element -> bayer2rgb
          - link bayer2rgb to ghost pad -> DONE

      - target caps could not be linked, FAIL
*/
static gboolean gst_tcambin_link_elements(GstTcamBin *self)
{
    if (!self->elements_created)
    {
        g_critical("Trying to link but elements are not created");
        return false;
    }

    gst_tcambin_unlink_all(self);

    GstPad *pad = gst_element_get_static_pad(self->out_caps, "src");
    if (!gst_ghost_pad_set_target(GST_GHOST_PAD(self->pad), pad))
    {
        g_critical("Failed to sed pad target");
    }

    GstCaps *filter_caps = gst_caps_new_any();
    GstCaps *target_caps = gst_pad_peer_query_caps(self->pad, filter_caps);
    gst_caps_unref(filter_caps);
    if (target_caps == nullptr)
    {
        g_critical("target caps are empty");
        return false;
    }

    if (!gst_element_link(self->src, self->pipeline_caps))
    {
        g_critical("Failed to link source to internal caps!");
        gst_caps_unref(target_caps);
        return false;
    }

    GST_DEBUG("Linking elements with caps: %" GST_PTR_FORMAT, target_caps);
    if (gst_caps_is_any(target_caps))
    {
        gst_caps_unref(target_caps);
        return true;
    }
    gboolean is_linked = false;

    if (self->dutils)
    {
        GstCaps *dutils_caps = get_caps_from_element(self->dutils, "src");
        if (gst_caps_can_intersect(target_caps, dutils_caps))
        {
            g_return_val_if_fail(gst_element_link(self->pipeline_caps, self->dutils), false);
            GstPad *pad = gst_element_get_static_pad(self->dutils, "src");
            gst_ghost_pad_set_target(GST_GHOST_PAD(self->pad), pad);
            gst_object_unref(pad);

            is_linked = true;
        }
        else
        {
            GstCaps *biteater_caps = get_caps_from_element(self->biteater, "src");
            if (gst_caps_can_intersect(target_caps, biteater_caps))
            {
                g_return_val_if_fail(gst_element_link(self->pipeline_caps, self->dutils), false);
                g_return_val_if_fail(gst_element_link(self->dutils, self->biteater), false);
                GstPad *pad = gst_element_get_static_pad(self->biteater, "src");
                gst_ghost_pad_set_target(GST_GHOST_PAD(self->pad), pad);
                gst_object_unref(pad);

                is_linked = true;
            }
            gst_caps_unref(biteater_caps);
        }
        gst_caps_unref(dutils_caps);
    }
    else
    {
        GstElement *prev = self->pipeline_caps;
        if (tcam_gst_can_intersect_simple(target_caps, "video/x-bayer") ||
            tcam_gst_can_intersect_simple(target_caps, "video/x-raw,format=GRAY8"))
        {
            if (self->exposure)
            {
                g_return_val_if_fail(gst_element_link(prev, self->exposure), false);
                prev = self->exposure;
            }
            if (self->focus)
            {
                g_return_val_if_fail(gst_element_link(prev, self->focus), false);
                prev = self->focus;
            }
        }
        if (tcam_gst_can_intersect_simple(target_caps, "video/x-bayer"))
        {
            if (self->whitebalance)
            {
                g_return_val_if_fail(gst_element_link(prev, self->whitebalance), false);
                prev = self->whitebalance;
            }
        }
        GstCaps *srccaps = get_caps_from_element(self->pipeline_caps, "src");
        GST_DEBUG("Linking with source caps: %" GST_PTR_FORMAT, srccaps);
        if (gst_caps_can_intersect(target_caps, srccaps))
        {
            g_return_val_if_fail(gst_element_link(prev, self->out_caps), false);

            is_linked = true;
        }
        else
        {
            if (self->debayer)
            {
                prev = self->pipeline_caps;
                GstCaps *bayercaps = get_caps_from_element(self->debayer, "src");
                if (gst_caps_can_intersect(target_caps, bayercaps))
                {
                    GstCaps *bayersinkcaps = get_caps_from_element(self->debayer, "sink");
                    if (gst_caps_can_intersect(srccaps, bayersinkcaps))
                    {
                        if (self->exposure)
                        {
                            g_return_val_if_fail(gst_element_link(prev, self->exposure), false);
                            prev = self->exposure;
                        }
                        if (self->whitebalance)
                        {
                            g_return_val_if_fail(gst_element_link(prev, self->whitebalance), false);
                            prev = self->whitebalance;
                        }
                        if (self->focus)
                        {
                            g_return_val_if_fail(gst_element_link(prev, self->focus), false);
                            prev = self->focus;
                        }
                        g_return_val_if_fail(gst_element_link(prev, self->debayer), false);
                        g_return_val_if_fail(gst_element_link(self->debayer, self->out_caps), false);

                        is_linked = true;
                    }
                    gst_caps_unref(bayersinkcaps);
                }
                gst_caps_unref(bayercaps);
            }
        }
    }
    gst_object_unref(pad);
    return is_linked;
}


/**
 * Generate GstCaps that contains all possible caps from src, bayer2rgb and videoconvert
 * in case of an error: return nullptr
 */
static GstCaps* generate_all_caps (GstTcamBin* self)
{
    GstPad *pad = gst_element_get_static_pad(self->src, "src");
    GstCaps* incoming = gst_pad_query_caps(pad, NULL);
    gst_object_unref(pad);

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
            const char *string = gst_structure_get_string (struc, "format");

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
            else if (!self->dutils &&
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


static GstStateChangeReturn gst_tcambin_change_state (GstElement* element,
                                                      GstStateChange trans)
{
    GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;

    GstTcamBin* self = GST_TCAMBIN(element);

    switch (trans)
    {
        case GST_STATE_CHANGE_NULL_TO_READY:
        {
            // From the docs:
            // state change from NULL to READY.
            // The element must check if the resources it needs are available.
            // Device sinks and -sources typically try
            // to probe the device to constrain their caps.
            // The element opens the device (in case feature need to be probed).
            GST_INFO("NULL_TO_READY");

            // The pad does not get a CAPS event in null state so we need to
            // link here for the case the element is already connected
            gst_tcambin_link_elements(self);
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
            if (!self->elements_created)
            {
                gst_tcambin_create_elements(self, nullptr);
            }
            g_object_get_property(G_OBJECT(self->src), "serial", value);
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
            const gchar *serial = g_value_get_string(value);
            GST_INFO("Setting source serial to %s", serial);
            if (!self->elements_created)
            {
                gst_tcambin_create_elements(self, serial);
            }
            else
            {
                gst_tcambin_clear_elements(self);
                gst_tcambin_create_elements(self, serial);
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
            break;
        }
        default:
        {
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
        }
    }
}

static GstPadProbeReturn gst_tcambin_pad_probe(GstPad *pad,
                                               GstPadProbeInfo *info,
                                               gpointer user_data)
{
    GstTcamBin* self = GST_TCAMBIN(user_data);
    GstEvent *event = GST_EVENT(info->data);
    if (GST_EVENT_TYPE(event) == GST_EVENT_CAPS)
    {
        GstCaps *caps;
        gst_event_parse_caps(event, &caps);
        GST_DEBUG("Got event caps %" GST_PTR_FORMAT, caps);
        gst_tcambin_link_elements(self);
    }
    return GST_PAD_PROBE_OK;
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

    gst_pad_add_probe(self->pad,
                      GST_PAD_PROBE_TYPE_EVENT_BOTH,
                      gst_tcambin_pad_probe,
                      self,
                      nullptr);

    if (!gst_tcambin_create_elements(self, nullptr))
    {
        GST_ERROR("Error while creating elements");
    }


    GST_OBJECT_FLAG_SET(self, GST_ELEMENT_FLAG_SOURCE);
}


static void gst_tcambin_finalize (GObject* object)
{
    G_OBJECT_CLASS (parent_class)->finalize(object);
}


static void gst_tcambin_dispose (GstTcamBin* self)
{
    GST_DEBUG("dispose");

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

    element_class->change_state = GST_DEBUG_FUNCPTR(gst_tcambin_change_state);

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
