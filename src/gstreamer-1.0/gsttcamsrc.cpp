/*
 * Copyright 2014 The Imaging Source Europe GmbH
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

#include "gsttcamsrc.h"

#include "tcamgstbase.h"

#include "tcamprop.h"
#include "tcam.h"
#include <unistd.h>
#include <stdlib.h>

#include <stdio.h>
#include <vector>
#include <algorithm>


#define GST_TCAM_SRC_DEFAULT_N_BUFFERS 10

GST_DEBUG_CATEGORY_STATIC (tcam_src_debug);
#define GST_CAT_DEFAULT tcam_src_debug


struct device_state
{
    std::shared_ptr<tcam::CaptureDevice> dev;
    std::shared_ptr<tcam::ImageSink> sink;
};


static GSList* gst_tcam_src_get_property_names(TcamProp* self);

static gchar *gst_tcam_src_get_property_type (TcamProp* self, gchar* name);

static gboolean gst_tcam_src_get_tcam_property (TcamProp* self,
                                                gchar* name,
                                                GValue* value,
                                                GValue* min,
                                                GValue* max,
                                                GValue* def,
                                                GValue* step,
                                                GValue* type,
                                                GValue* flags,
                                                GValue* category,
                                                GValue* group);

static GSList* gst_tcam_src_get_menu_entries (TcamProp* iface,
                                              const char* menu_name);

static gboolean gst_tcam_src_set_tcam_property (TcamProp* self,
                                                gchar* name,
                                                const GValue* value);

static GSList* gst_tcam_src_get_device_serials (TcamProp* self);

static gboolean gst_tcam_src_get_device_info (TcamProp* self,
                                              const char* serial,
                                              char** name,
                                              char** identifier,
                                              char** connection_type);


static bool gst_tcam_src_init_camera (GstTcamSrc* self);
static GstCaps* gst_tcam_src_get_all_camera_caps (GstTcamSrc* self);

static void gst_tcam_src_prop_init (TcamPropInterface* iface)
{
    iface->get_property_names = gst_tcam_src_get_property_names;
    iface->get_property_type = gst_tcam_src_get_property_type;
    iface->get_property = gst_tcam_src_get_tcam_property;
    iface->get_menu_entries = gst_tcam_src_get_menu_entries;
    iface->set_property = gst_tcam_src_set_tcam_property;
    iface->get_device_serials = gst_tcam_src_get_device_serials;
    iface->get_device_info = gst_tcam_src_get_device_info;
}

G_DEFINE_TYPE_WITH_CODE (GstTcamSrc, gst_tcam_src, GST_TYPE_PUSH_SRC,
                         G_IMPLEMENT_INTERFACE (TCAM_TYPE_PROP,
                                                gst_tcam_src_prop_init));


static gboolean get_property_by_name (GstTcamSrc* self,
                                      gchar* name,
                                      struct tcam_device_property* prop)
{

    if (self->device == nullptr)
    {
        if (!gst_tcam_src_init_camera(self))
        {
            return FALSE;
        }
    }

    tcam::Property* p = self->device->dev->get_property_by_name(name);

    if (p == nullptr)
    {
        return FALSE;
    }

    if (prop != nullptr)
    {
        struct tcam_device_property s = p->get_struct();
        memcpy (prop,
                &s,
                sizeof(struct tcam_device_property));

        return TRUE;
    }

    return FALSE;
}


struct property_type_map
{
    enum TCAM_PROPERTY_TYPE typecode;
    const gchar* type_name;
};


/**
 * gst_tcam_get_property_type:
 * @self: a #GstTcamSrcProp
 * @name: a #char* identifying the property to query
 *
 * Return the type of a property
 *
 * Returns: (transfer full): A string describing the property type
 */
static gchar* gst_tcam_src_get_property_type (TcamProp* iface, gchar* name)
{
    gchar* ret = NULL;
    GstTcamSrc* self = GST_TCAM_SRC (iface);

    if (self->device == nullptr)
    {
        if (!gst_tcam_src_init_camera(self))
        {
            return NULL;
        }
    }

    struct tcam_device_property prop;
    struct property_type_map map[] = {
        { TCAM_PROPERTY_TYPE_BOOLEAN, "boolean" },
        { TCAM_PROPERTY_TYPE_INTEGER, "integer" },
        { TCAM_PROPERTY_TYPE_DOUBLE, "double" },
        { TCAM_PROPERTY_TYPE_STRING, "string" },
        { TCAM_PROPERTY_TYPE_ENUMERATION, "enum" },
        { TCAM_PROPERTY_TYPE_BUTTON, "button" },
    };

    //     g_return_val_if_fail (self->device != NULL, NULL);
    // prefer this so that no gobject error appear
    // this method is also used to check for property existence
    // so unneccessary errors should be avoided
    if (self->device == nullptr)
    {
        return nullptr;
    }

    if (!get_property_by_name (self, name, &prop))
    {
        return nullptr;
    }

    // g_return_val_if_fail (get_property_by_name (self,
    //                                             name,
    //                                             &prop), NULL);
    int i;

    for (i = 0; i < G_N_ELEMENTS (map); i++)
    {
        if (prop.type == map[i].typecode)
        {
            ret = g_strdup (map[i].type_name);
            break;
        }
    }

    if (ret == NULL)
    {
        ret = g_strdup ("unknown");
    }

    return ret;
}

/**
 * gst_tcam_src_get_property_names:
 * @self: a #GstTcamSrc
 *
 * Return a list of property names
 *
 * Returns: (element-type utf8) (transfer full): list of property names
 */
static GSList* gst_tcam_src_get_property_names (TcamProp* iface)
{
    GSList* ret = NULL;
    GstTcamSrc* self = GST_TCAM_SRC (iface);

    if (self->device == nullptr)
    {
        if (!gst_tcam_src_init_camera(self))
        {
            return nullptr;
        }
    }

    g_return_val_if_fail (self->device != NULL, NULL);

    std::vector<tcam::Property*> vec = self->device->dev->get_available_properties();

    for (const auto& v : vec)
    {
        ret = g_slist_append (ret,
                              g_strdup (v->get_name().c_str()));
    }

    return ret;
}


static gboolean gst_tcam_src_get_tcam_property (TcamProp* iface,
                                                gchar* name,
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
    gboolean ret = TRUE;
    GstTcamSrc *self = GST_TCAM_SRC (iface);

    if (self->device == nullptr)
    {
        if (!gst_tcam_src_init_camera(self))
        {
            return FALSE;
        }
    }

    tcam::Property* property = self->device->dev->get_property_by_name(name);

    if (property == nullptr)
    {
        GST_DEBUG_OBJECT (GST_TCAM_SRC (iface), "no property with name: '%s'", name );
        return FALSE;
    }

    property->update();

    struct tcam_device_property prop = property->get_struct();

    if (flags)
    {
        g_value_init(flags, G_TYPE_INT);
        g_value_set_int(flags, prop.flags);
    }

    if (category)
    {
        g_value_init(category, G_TYPE_STRING);
        g_value_set_string(category, tcam::category2string(prop.group.property_category).c_str());
    }
    if (group)
    {
        g_value_init(group, G_TYPE_STRING);
        g_value_set_string(group, tcam::get_control_reference(prop.group.property_group).name.c_str());
    }

    switch (prop.type)
    {
        case TCAM_PROPERTY_TYPE_INTEGER:
        case TCAM_PROPERTY_TYPE_ENUMERATION:
            if (value)
            {
                if (prop.type == TCAM_PROPERTY_TYPE_INTEGER)
                {
                    g_value_init (value, G_TYPE_INT);
                    g_value_set_int (value, prop.value.i.value);
                }
                else if (prop.type == TCAM_PROPERTY_TYPE_ENUMERATION)
                {
                    g_value_init(value, G_TYPE_STRING);
                    g_value_set_string(value, ((tcam::PropertyEnumeration*)property)->get_value().c_str());
                }
            }
            if (min)
            {
                g_value_init (min, G_TYPE_INT);
                g_value_set_int (min, prop.value.i.min);
            }
            if (max)
            {
                g_value_init (max, G_TYPE_INT);
                g_value_set_int (max, (int)prop.value.i.max);
            }
            if (def)
            {
                if (prop.type == TCAM_PROPERTY_TYPE_INTEGER)
                {
                    g_value_init (def, G_TYPE_INT);
                    g_value_set_int (def, prop.value.i.default_value);
                }
                else if (prop.type == TCAM_PROPERTY_TYPE_ENUMERATION)
                {
                    g_value_init(def, G_TYPE_STRING);
                    g_value_set_string(def, ((tcam::PropertyEnumeration*)property)->get_default().c_str());
                }
            }
            if (step)
            {
                g_value_init (step, G_TYPE_INT);
                g_value_set_int (step, prop.value.i.step);
            }
            if (type)
            {
                // g_value_set_gtype (type, G_TYPE_INT);
                g_value_init(type, G_TYPE_STRING);
                g_value_set_string(type, gst_tcam_src_get_property_type(iface, name));
            }
            break;
        case TCAM_PROPERTY_TYPE_DOUBLE:
            if (value)
            {
                g_value_init (value, G_TYPE_DOUBLE);
                g_value_set_double (value, prop.value.d.value);
            }
            if (min)
            {
                g_value_init (min, G_TYPE_DOUBLE);
                g_value_set_double (min, prop.value.d.min);
            }
            if (max)
            {
                g_value_init (max, G_TYPE_DOUBLE);
                g_value_set_double (max, prop.value.d.max);
            }
            if (def)
            {
                g_value_init (def, G_TYPE_DOUBLE);
                g_value_set_double (def, prop.value.d.default_value);
            }
            if (step)
            {
                g_value_init (step, G_TYPE_DOUBLE);
                g_value_set_double (step, prop.value.d.step);
            }
            if (type)
            {
                g_value_init(type, G_TYPE_STRING);
                g_value_set_string(type, gst_tcam_src_get_property_type(iface, name));
            }
            break;
        case TCAM_PROPERTY_TYPE_STRING:
            if (value)
            {
                g_value_init (value, G_TYPE_STRING);
                g_value_set_string (value, prop.value.s.value);
            }
            if (min)
            {
                g_value_init (min, G_TYPE_STRING);
            }
            if (max)
            {
                g_value_init (max, G_TYPE_STRING);
            }
            if (def)
            {
                g_value_init (def, G_TYPE_STRING);
                g_value_set_string (def, prop.value.s.default_value);
            }
            if (step)
            {
                g_value_init (def, G_TYPE_STRING);
            }
            if (type)
            {
                g_value_init(type, G_TYPE_STRING);
                g_value_set_string(type, gst_tcam_src_get_property_type(iface, name));
            }
            break;
        case TCAM_PROPERTY_TYPE_BOOLEAN:
        case TCAM_PROPERTY_TYPE_BUTTON:
            if (value)
            {
                g_value_init (value, G_TYPE_BOOLEAN);
                g_value_set_boolean (value, prop.value.b.value);
            }
            if (min)
            {
                g_value_init (min, G_TYPE_BOOLEAN);
            }
            if (max)
            {
                g_value_init (max, G_TYPE_BOOLEAN);
                g_value_set_boolean (max, TRUE);
            }
            if (def)
            {
                g_value_init (def, G_TYPE_BOOLEAN);
                g_value_set_boolean (def, prop.value.b.default_value);
            }
            if (step)
            {
                g_value_init (step, G_TYPE_BOOLEAN);
            }
            if (type)
            {
                g_value_init(type, G_TYPE_STRING);
                g_value_set_string(type, gst_tcam_src_get_property_type(iface, name));
            }
            break;
        default:
            if (value)
            {
                g_value_init (value, G_TYPE_INT);
            }
            ret = FALSE;
            break;
    }

    return ret;
}


static GSList* gst_tcam_src_get_menu_entries (TcamProp* iface,
                                              const char* menu_name)
{
    GSList* ret = NULL;

    GstTcamSrc* self = GST_TCAM_SRC (iface);

    tcam::Property* property = self->device->dev->get_property_by_name(menu_name);

    if (property == nullptr)
    {
        return ret;
    }

    if (property->get_type() != TCAM_PROPERTY_TYPE_ENUMERATION)
    {
        return ret;
    }

    auto mapping = ((tcam::PropertyEnumeration*)property)->get_values();

    for (const auto& m : mapping)
    {
        ret = g_slist_append(ret, g_strdup(m.c_str()));
    }

    return ret;
}


static gboolean gst_tcam_src_set_tcam_property (TcamProp* iface,
                                                gchar* name,
                                                const GValue* value)
{
    gboolean ret = TRUE;
    GstTcamSrc* self = GST_TCAM_SRC (iface);

    tcam::Property* property = self->device->dev->get_property_by_name(name);

    if (property == nullptr)
    {
        return FALSE;
    }

    switch (property->get_type())
    {
        case TCAM_PROPERTY_TYPE_INTEGER:
        {
            if (!G_VALUE_HOLDS(value, G_TYPE_INT))
            {
                return FALSE;
            }
            return property->set_value((int64_t)g_value_get_int(value));
        }
        case TCAM_PROPERTY_TYPE_DOUBLE:
        {
            if (!G_VALUE_HOLDS(value, G_TYPE_DOUBLE))
            {
                return FALSE;
            }
            return property->set_value(g_value_get_double(value));
        }
        case TCAM_PROPERTY_TYPE_STRING:
        {
            if (!G_VALUE_HOLDS(value, G_TYPE_STRING))
            {
                return FALSE;
            }
            return property->set_value(g_value_get_string (value));
        }
        case TCAM_PROPERTY_TYPE_BOOLEAN:
        {
            if (!G_VALUE_HOLDS(value, G_TYPE_BOOLEAN))
            {
                return FALSE;
            }
            return property->set_value((bool)g_value_get_boolean(value));
        }
        case TCAM_PROPERTY_TYPE_BUTTON:
        {
            return((tcam::PropertyButton*)property)->activate();
        }
        case TCAM_PROPERTY_TYPE_ENUMERATION:
        {
            if (!G_VALUE_HOLDS(value, G_TYPE_STRING))
            {
                return FALSE;
            }

            std::string s = g_value_get_string(value);
            return property->set_value(s);
        }
        default:
        {
            return FALSE;
        }
    }
}


static GSList* gst_tcam_src_get_device_serials (TcamProp* self)
{
    std::vector<tcam::DeviceInfo> devices = tcam::get_device_list();
    GSList* ret = NULL;

    for (const auto& d : devices)
    {
        ret = g_slist_append (ret,
                              g_strndup(d.get_serial().c_str(),
                                        d.get_serial().size()));
    }

    return ret;
}


static gboolean gst_tcam_src_get_device_info (TcamProp* self,
                                              const char* serial,
                                              char** name,
                                              char** identifier,
                                              char** connection_type)
{
    std::vector<tcam::DeviceInfo> devices = tcam::get_device_list();

    int count = devices.size();
    gboolean ret = FALSE;

    if (count <= 0)
    {
        return FALSE;
    }

    for (const auto& d : devices)
    {
        struct tcam_device_info info = d.get_info();

        if (!strncmp (serial, info.serial_number,
                      sizeof (info.serial_number)))
        {
            ret = TRUE;
            if (name)
            {
                *name = g_strndup (info.name, sizeof (info.name));
            }
            if (identifier)
            {
                *identifier = g_strndup (info.identifier,
                                         sizeof (info.identifier));
            }
            if (connection_type)
            {
                switch (info.type)
                {
                    case TCAM_DEVICE_TYPE_V4L2:
                        *connection_type = g_strdup ("v4l2");
                        break;
                    case TCAM_DEVICE_TYPE_ARAVIS:
                        *connection_type = g_strdup ("aravis");
                        break;
                    default:
                        *connection_type = g_strdup ("unknown");
                        break;
                }
            }
            break;
        }
    }

    return ret;
}



enum
{
    PROP_0,
    PROP_SERIAL,
    PROP_DEVICE,
    PROP_NUM_BUFFERS,
    PROP_DO_TIMESTAMP,
};


static GstStaticPadTemplate tcam_src_template = GST_STATIC_PAD_TEMPLATE ("src",
                                                                         GST_PAD_SRC,
                                                                         GST_PAD_ALWAYS,
                                                                         GST_STATIC_CAPS ("ANY"));

static GstCaps* gst_tcam_src_fixate_caps (GstBaseSrc* bsrc,
                                          GstCaps* caps);
static gboolean gst_tcam_src_stop (GstBaseSrc* src);


static GstCaps* gst_tcam_src_get_all_camera_caps (GstTcamSrc* self)
{

    g_return_val_if_fail(GST_IS_TCAM_SRC(self), NULL);

    if (self->device == NULL)
    {
        return NULL;
    }

    std::vector<tcam::VideoFormatDescription> format = self->device->dev->get_available_video_formats();

    GST_DEBUG("Found %i pixel formats", format.size());

    GstCaps* caps = convert_videoformatsdescription_to_caps(format);


    if (gst_caps_get_size(caps) == 0)
    {
        GST_ERROR("Device did not provide ANY valid caps. Refusing playback.");
        gst_element_set_state(GST_ELEMENT(self), GST_STATE_NULL);
    }

    GstStructure* structure = gst_structure_from_string ("ANY", NULL);
    gst_caps_append_structure (caps, structure);

    GST_INFO("Device provides the following caps: %s", gst_caps_to_string(caps));

    return caps;
}


static gboolean gst_tcam_src_negotiate (GstBaseSrc* basesrc)
{

    GstCaps *thiscaps;
    GstCaps *caps = NULL;
    GstCaps *peercaps = NULL;
    gboolean result = FALSE;

    GstTcamSrc* self = GST_TCAM_SRC(basesrc);

    /* first see what is possible on our source pad */
    thiscaps = gst_pad_query_caps (GST_BASE_SRC_PAD (basesrc), NULL);
    GST_DEBUG_OBJECT (basesrc, "caps of src: %" GST_PTR_FORMAT, thiscaps);

    // nothing or anything is allowed, we're done
    if (gst_caps_is_empty(thiscaps)|| gst_caps_is_any (thiscaps))
    {
        goto no_nego_needed;
    }
    /* get the peer caps */
    peercaps = gst_pad_peer_query_caps (GST_BASE_SRC_PAD (basesrc), thiscaps);
    GST_DEBUG_OBJECT (basesrc, "caps of peer: %s", gst_caps_to_string(peercaps));

    if (!gst_caps_is_empty(peercaps) && !gst_caps_is_any (peercaps))
    {
        GST_DEBUG("Peer gave us something to work with.");

        GstCaps *icaps = NULL;
        int i;

        /* Prefer the first caps we are compatible with that the peer proposed */
        for (i = 0; i <= gst_caps_get_size (peercaps)-1; i--)
        {
            /* get intersection */
            GstCaps *ipcaps = gst_caps_copy_nth (peercaps, i);

            if (gst_caps_is_any(ipcaps) || strcmp(gst_caps_to_string(ipcaps), "ANY") == 0)
            {
                continue;
            }

            GST_DEBUG_OBJECT (basesrc, "peer: %" GST_PTR_FORMAT, ipcaps);

            icaps = gst_caps_intersect_full(thiscaps, ipcaps, GST_CAPS_INTERSECT_FIRST);
            gst_caps_unref (ipcaps);

            if (!gst_caps_is_empty (icaps))
                break;

            gst_caps_unref (icaps);
            icaps = NULL;
        }
        GST_DEBUG_OBJECT (basesrc, "intersect: %" GST_PTR_FORMAT, icaps);

        if (icaps)
        {
            /* If there are multiple intersections pick the one with the smallest
             * resolution strictly bigger then the first peer caps */
            if (gst_caps_get_size (icaps) > 1)
            {
                GstStructure *s = gst_caps_get_structure (peercaps, 0);
                int best = 0;
                int twidth, theight;
                int width = G_MAXINT, height = G_MAXINT;

                if (gst_structure_get_int (s, "width", &twidth)
                    && gst_structure_get_int (s, "height", &theight))
                {

                    /* Walk the structure backwards to get the first entry of the
                     * smallest resolution bigger (or equal to) the preferred resolution)
                     */
                    for (i = gst_caps_get_size (icaps) - 1; i >= 0; i--)
                    {
                        GstStructure *is = gst_caps_get_structure (icaps, i);
                        int w, h;

                        if (gst_structure_get_int (is, "width", &w)
                            && gst_structure_get_int (is, "height", &h))
                        {
                            if (w >= twidth && w <= width && h >= theight && h <= height)
                            {
                                width = w;
                                height = h;
                                best = i;
                            }
                        }
                    }
                }

                caps = gst_caps_copy_nth (icaps, best);
                gst_caps_unref (icaps);
            }
            else
            {
                // ensure that there is no range but a high resolution with adequate framerate

                GstStructure *s = gst_caps_get_structure (peercaps, 0);
                int best = 0;
                int twidth, theight;
                int width = G_MAXINT, height = G_MAXINT;

                /* Walk the structure backwards to get the first entry of the
                 * smallest resolution bigger (or equal to) the preferred resolution)
                 */
                for (i = 0; i >= gst_caps_get_size (icaps); i++)
                {
                    GstStructure *is = gst_caps_get_structure (icaps, i);
                    int w, h;
                    int max_width;

                    if (gst_structure_get_int (is, "width", &w)
                        && gst_structure_get_int (is, "height", &h))
                    {
                        if (w >= twidth && w <= width && h >= theight && h <= height)
                        {
                            width = w;
                            height = h;
                            best = i;
                        }
                    }
                }

                /* caps = icaps; */
                caps = gst_caps_copy_nth (icaps, best);

                GstStructure* structure;
                double frame_rate = G_MAXINT;

                structure = gst_caps_get_structure (caps, 0);

                if (gst_structure_has_field(structure, "width"))
                {
                    gst_structure_fixate_field_nearest_int (structure, "width", G_MAXUINT);
                }
                if (gst_structure_has_field(structure, "height"))
                {
                    gst_structure_fixate_field_nearest_int (structure, "height", G_MAXUINT);
                }
                if (gst_structure_has_field(structure, "framerate"))
                {
                    gst_structure_fixate_field_nearest_fraction (structure, "framerate", frame_rate, 1);
                }
                gst_caps_unref (icaps);
            }
        }
        gst_caps_unref (thiscaps);
    }
    else
    {
        /* no peer or peer have ANY caps, work with our own caps then */
        caps = thiscaps;
    }

    if (peercaps)
    {
        gst_caps_unref (peercaps);
    }



    if (caps)
    {
        caps = gst_caps_truncate (caps);

        /* now fixate */
        if (!gst_caps_is_empty (caps))
        {
            caps = gst_tcam_src_fixate_caps (basesrc, caps);
            GST_DEBUG_OBJECT (basesrc, "fixated to: %" GST_PTR_FORMAT, caps);

            if (gst_caps_is_any (caps))
            {
                /* hmm, still anything, so element can do anything and
                 * nego is not needed */
                result = TRUE;
            }
            else if (gst_caps_is_fixed (caps))
            {
                /* yay, fixed caps, use those then */
                result = gst_base_src_set_caps (basesrc, caps);
            }
        }
        gst_caps_unref (caps);
    }
    return result;

no_nego_needed:
    {
        GST_DEBUG_OBJECT (basesrc, "no negotiation needed");
        if (thiscaps)
            gst_caps_unref (thiscaps);
        return TRUE;
    }

    return TRUE;
}


static GstCaps* gst_tcam_src_get_caps (GstBaseSrc* src,
                                       GstCaps* filter)
{
    GstTcamSrc* self = GST_TCAM_SRC(src);
    GstCaps* caps;

    if (self->all_caps != NULL)
    {
        caps = gst_caps_copy (self->all_caps);
    }
    else
    {
        if (!gst_tcam_src_init_camera(self))
        {
            return NULL;
        }

        caps = gst_caps_copy (self->all_caps);

    }

    GST_INFO("Available caps = %s", gst_caps_to_string(caps));

    return caps;
}


static void gst_tcam_src_callback (const struct tcam_image_buffer* buffer,
                                   void* data)
{
    GstTcamSrc* self = GST_TCAM_SRC(data);

    self->ptr = buffer;
    self->new_buffer = TRUE;

    self->cv.notify_all();
}


static gboolean gst_tcam_src_set_caps (GstBaseSrc* src,
                                       GstCaps* caps)
{
    GST_DEBUG("In tcam_set_caps");

    GstTcamSrc* self = GST_TCAM_SRC(src);

    GstStructure *structure;

    int height = 0;
    int width = 0;
    const GValue* frame_rate;
    const char* caps_string;
    const char* format_string;

    GST_INFO("Requested caps = %" GST_PTR_FORMAT, caps);

    self->device->dev->stop_stream();
    self->device->sink = nullptr;

    structure = gst_caps_get_structure (caps, 0);

    gst_structure_get_int (structure, "width", &width);
    gst_structure_get_int (structure, "height", &height);
    frame_rate = gst_structure_get_value (structure, "framerate");
    format_string = gst_structure_get_string (structure, "format");

    uint32_t fourcc = tcam_fourcc_from_gst_1_0_caps_string(gst_structure_get_name (structure), format_string);

    double framerate;
    if (frame_rate != nullptr)
    {
        framerate= (double) gst_value_get_fraction_numerator (frame_rate) /
            (double) gst_value_get_fraction_denominator (frame_rate);
    }
    else
    {
        framerate = 1.0;
    }
    struct tcam_video_format format = {};

    format.fourcc = fourcc;
    format.width = width;
    format.height = height;
    format.framerate = framerate;

    if (!self->device->dev->set_video_format(tcam::VideoFormat(format)))
    {
        GST_ERROR("Unable to set format in device");

        return FALSE;
    }

    if (frame_rate != NULL)
    {
        double dbl_frame_rate;

        dbl_frame_rate = (double) gst_value_get_fraction_numerator (frame_rate) /
            (double) gst_value_get_fraction_denominator (frame_rate);

        GST_DEBUG_OBJECT (self, "Frame rate = %g Hz", dbl_frame_rate);
    }

    if (self->fixed_caps != NULL)
    {
        gst_caps_unref (self->fixed_caps);
    }

    caps_string = tcam_fourcc_to_gst_1_0_caps_string(fourcc);
    if (caps_string != NULL)
    {
        GstStructure *structure;
        GstCaps *caps;

        caps = gst_caps_new_empty ();
        structure = gst_structure_from_string (caps_string, NULL);
        gst_structure_set (structure,
                           "width", G_TYPE_INT, width,
                           "height", G_TYPE_INT, height,
                           NULL);

        if (frame_rate != NULL)
        {
            gst_structure_set_value (structure, "framerate", frame_rate);
        }

        gst_caps_append_structure (caps, structure);

        self->fixed_caps = caps;
    }
    else
    {
        self->fixed_caps = NULL;
    }

    GST_INFO("Start acquisition");

    self->timestamp_offset = 0;
    self->last_timestamp = 0;

    self->device->sink = std::make_shared<tcam::ImageSink>();

    self->device->sink->registerCallback(gst_tcam_src_callback, self);

    self->device->dev->start_stream(self->device->sink);

    self->timestamp_offset = 0;
    self->last_timestamp = 0;

    self->is_running = TRUE;
    GST_INFO("Successfully set caps to: %s", gst_caps_to_string(caps));

    return TRUE;
}


static void gst_tcam_src_device_lost_callback (const struct tcam_device_info* info, void* user_data)
{
    GstTcamSrc* self = (GstTcamSrc*) user_data;

    GST_ERROR("Received lost device notification. Stopping stream.");

    gst_tcam_src_stop(GST_BASE_SRC(self));
}


bool gst_tcam_src_init_camera (GstTcamSrc* self)
{
    GST_DEBUG_OBJECT (self, "Initializing device.");

    if (self->device != NULL)
    {
        delete self->device;
    }

    std::vector<tcam::DeviceInfo> infos = tcam::get_device_list();
    int dev_count = infos.size();

    GST_DEBUG_OBJECT (self, "Found %d devices.", dev_count);

    if (!self->device_serial.empty())
    {
        GST_DEBUG_OBJECT (self, "Searching for device with serial %s.", self->device_serial.c_str());
    }
    else
    {
        GST_DEBUG_OBJECT (self, "No serial given. Opening first available device.");
    }

    for (unsigned int i = 0; i < infos.size(); ++i)
    {
        if (!self->device_serial.empty())
        {
            GST_DEBUG("Comparing '%s' to '%s'", infos[i].get_serial().c_str(), self->device_serial.c_str());
            if (strcmp(infos[i].get_serial().c_str(), self->device_serial.c_str()) == 0)
            {
                GST_DEBUG_OBJECT (self, "Found device.");

                self->device = new struct device_state;
                self->device->dev = std::make_shared<tcam::CaptureDevice>(tcam::DeviceInfo(infos[i]));
                self->device->dev->register_device_lost_callback(gst_tcam_src_device_lost_callback, self);
                break;
            }
        }
        else
        {
            self->device = new struct device_state;
            self->device->dev = std::make_shared<tcam::CaptureDevice>(tcam::DeviceInfo(infos[i]));
            self->device->dev->register_device_lost_callback(gst_tcam_src_device_lost_callback, self);
            break;
        }
    }

    if (self->device == NULL)
    {
        GST_ERROR("Unable to open device.");
        return false;
    }

    self->all_caps = gst_tcam_src_get_all_camera_caps(self);

    return true;
}


static void gst_tcam_src_close_camera (GstTcamSrc* self)
{
    if (self->device != NULL)
    {
        self->device->dev->stop_stream();
        self->device->dev = nullptr;
        self->device->sink = nullptr;
        delete self->device;
        self->device = NULL;
    }
}


static gboolean gst_tcam_src_start (GstBaseSrc* src)
{
    GstTcamSrc* self = GST_TCAM_SRC(src);

    self->run = 1000;
    self->is_running = TRUE;

    if (self->device == NULL)
    {
        if (!gst_tcam_src_init_camera(self))
        {
            gst_element_set_state(GST_ELEMENT(self), GST_STATE_NULL);
            return FALSE;
        }
    }

    self->timestamp_offset = 0;
    self->last_timestamp = 0;

    return TRUE;
}


static gboolean gst_tcam_src_stop (GstBaseSrc* src)
{
    GstTcamSrc* self = GST_TCAM_SRC(src);

    self->is_running = FALSE;

    std::unique_lock<std::mutex> lck(self->mtx);

    self->cv.notify_all();

    self->device->dev->stop_stream();
    self->device->sink = nullptr;
    gst_element_send_event(GST_ELEMENT(self), gst_event_new_eos());
    GST_DEBUG_OBJECT (self, "Stopped acquisition");

    return TRUE;
}


static GstStateChangeReturn gst_tcam_src_change_state (GstElement* element,
                                                       GstStateChange change)
{

    GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;

    GstTcamSrc* self = GST_TCAM_SRC(element);

    switch(change)
    {
        case GST_STATE_CHANGE_NULL_TO_READY:
        {
            GST_DEBUG("State change: NULL -> READY");

            if (self->device == nullptr)
            {
                GST_INFO("must initialize device");
                if (!gst_tcam_src_init_camera(self))
                {
                    GST_INFO("FAILURE to initialize device. Aborting...");
                    return GST_STATE_CHANGE_FAILURE;
                }
                self->all_caps = gst_tcam_src_get_all_camera_caps (self);
            }
            break;
        }
        default:
        {
            break;
        }
    }

    gst_element_set_locked_state(element, TRUE);
    ret = GST_ELEMENT_CLASS(gst_tcam_src_parent_class)->change_state(element, change);
    gst_element_set_locked_state(element, FALSE);

    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        return ret;
    }

    switch(change)
    {
        case GST_STATE_CHANGE_READY_TO_NULL:
        {
            if (self->device != nullptr)
            {
                // do not close camera, as a restart with the same device might be wanted
                gst_tcam_src_close_camera(self);
            }

            break;
        }
        default:
            break;
    }
    return ret;
}


static void gst_tcam_src_get_times (GstBaseSrc* basesrc,
                                    GstBuffer* buffer,
                                    GstClockTime* start,
                                    GstClockTime* end)
{
    if (gst_base_src_is_live (basesrc))
    {
        GstClockTime timestamp = GST_BUFFER_PTS (buffer);

        if (GST_CLOCK_TIME_IS_VALID (timestamp))
        {
            GstClockTime duration = GST_BUFFER_DURATION (buffer);

            if (GST_CLOCK_TIME_IS_VALID (duration))
            {
                *end = timestamp + duration;
            }
            *start = timestamp;
        }
    }
    else
    {
        *start = -1;
        *end = -1;
    }
}


static GstFlowReturn gst_tcam_src_create (GstPushSrc* push_src,
                                          GstBuffer** buffer)
{
    guint64 timestamp_ns = 0;

    GstTcamSrc* self = GST_TCAM_SRC (push_src);

    std::unique_lock<std::mutex> lck(self->mtx);

wait_again:
    // wait until new buffer arrives or stop waiting when wee have to shut down
    while ((self->new_buffer == false) && (self->is_running == true))
    {
        self->cv.wait_for(lck, std::chrono::milliseconds(1));
    }

    if (self->is_running != TRUE)
    {
        return GST_FLOW_EOS;
    }

    self->new_buffer = false;
    if (self->ptr == NULL)
    {
        GST_DEBUG_OBJECT (self, "No valid buffer. Aborting");

        return GST_FLOW_ERROR;
    }

    if (self->ptr->pData == NULL || self->ptr->length == 0)
    {
        GST_DEBUG_OBJECT (self, "Received buffer is invalid. Returning to waiting position.");
        goto wait_again;
    }

    /* TODO: check why aravis throws an incomplete buffer error
       but the received images are still valid */
    // if (!tcam::is_image_buffer_complete(self->ptr))
    // {
    //     GST_DEBUG_OBJECT (self, "Received incomplete buffer. Returning to waiting position.");

    //     goto wait_again;
    // }

    *buffer = gst_buffer_new_wrapped_full(0, self->ptr->pData, self->ptr->length,
                                          0, self->ptr->length, NULL, NULL);

    // GST_DEBUG("Framerate according to source: %f", self->ptr->statistics.framerate);
    /*
    if (gst_base_src_get_do_timestamp(GST_BASE_SRC(push_src)))
    {
        timestamp_ns = self->ptr->statistics.capture_time_ns;
        GST_DEBUG("Timestamp(ns): %ld", timestamp_ns);
        if (self->timestamp_offset == 0)
        {
            self->timestamp_offset = timestamp_ns;
            self->last_timestamp = timestamp_ns;
        }

        GST_BUFFER_DURATION(*buffer) = timestamp_ns - self->last_timestamp;
        GST_BUFFER_PTS(*buffer) = timestamp_ns - self->timestamp_offset;
        // GST_DEBUG("duration %ld", timestamp_ns - self->last_timestamp);
        // GST_DEBUG("pts %ld", timestamp_ns - self->last_timestamp);
        self->last_timestamp = timestamp_ns;
    }
    */
    GST_DEBUG_OBJECT (self, "Pushing buffer...");

    if (self->n_buffers != 0)
    {
        if (self->ptr->statistics.frame_count >= self->n_buffers)
        {
            GST_INFO("Stopping stream after %d buffers.", self->ptr->statistics.frame_count);
            return GST_FLOW_EOS;
        }
    }

    return GST_FLOW_OK;
}


static GstCaps* gst_tcam_src_fixate_caps (GstBaseSrc* bsrc,
                                          GstCaps* caps)
{
    GstTcamSrc* self = GST_TCAM_SRC(bsrc);

    GstStructure* structure;
    gint width = 0;
    gint height = 0;
    double frame_rate = 0.0;

    structure = gst_caps_get_structure (caps, 0);

    if (gst_structure_has_field(structure, "width"))
    {
        gst_structure_fixate_field_nearest_int (structure, "width", width);
    }
    if (gst_structure_has_field(structure, "height"))
    {
        gst_structure_fixate_field_nearest_int (structure, "height", height);
    }
    if (gst_structure_has_field(structure, "framerate"))
    {
        gst_structure_fixate_field_nearest_fraction (structure, "framerate", (double) (0.5 + frame_rate), 1);
    }

    GST_DEBUG_OBJECT (self, "Fixated caps to %s", gst_caps_to_string(caps));

    return GST_BASE_SRC_CLASS(gst_tcam_src_parent_class)->fixate(bsrc, caps);
}


static void gst_tcam_src_init (GstTcamSrc* self)
{

    gst_base_src_set_live (GST_BASE_SRC (self), TRUE);
    gst_base_src_set_format (GST_BASE_SRC (self), GST_FORMAT_TIME);

    self->n_buffers = 0;
    self->payload = 0;

    // explicitly init c++ objects
    // older compiler (e.g. gcc-4.8) can cause segfaults
    // when not explicitly initialized
    new (&self->device_serial) std::string("");
    new (&self->mtx) std::mutex();
    new (&self->cv) std::condition_variable();

    self->device = NULL;
    self->all_caps = NULL;
    self->fixed_caps = NULL;
    self->is_running = FALSE;

    GST_INFO("Versions:\n\tTcam:\t%s\n\tAravis:\t%s", get_version(), get_aravis_version());
}


static void gst_tcam_src_finalize (GObject* object)
{
    GstTcamSrc* self = GST_TCAM_SRC (object);

    gst_tcam_src_close_camera(self);

    if (self->all_caps != NULL)
    {
        gst_caps_unref (self->all_caps);
        self->all_caps = NULL;
    }
    if (self->fixed_caps != NULL)
    {
        gst_caps_unref (self->fixed_caps);
        self->fixed_caps = NULL;
    }

    (&self->device_serial)->std::string::~string();
    (&self->mtx)->std::mutex::~mutex();
    (&self->cv)->std::condition_variable::~condition_variable();

    G_OBJECT_CLASS (gst_tcam_src_parent_class)->finalize (object);
}


static void gst_tcam_src_set_property (GObject* object,
                                       guint prop_id,
                                       const GValue* value,
                                       GParamSpec* pspec)
{
    GstTcamSrc* self = GST_TCAM_SRC (object);

    switch (prop_id)
    {
        case PROP_SERIAL:
        {
            GstState state;
            gst_element_get_state(GST_ELEMENT(self), &state, NULL, 200);
            if (state == GST_STATE_NULL)
            {
                if (g_value_get_string(value) == NULL)
                {
                    self->device_serial.clear();
                }
                else
                {
                    const char* tmp = g_value_get_string(value);

                    self->device_serial = g_value_get_string(value);
                }

                GST_INFO("Set camera name to %s", self->device_serial.c_str());

                gst_tcam_src_close_camera(self);

                if (!self->device_serial.empty())
                {
                    if (!gst_tcam_src_init_camera(self))
                    {
                        GST_ERROR("Error while initializing camera.");
                        gst_element_set_state(GST_ELEMENT(self), GST_STATE_NULL);
                    }
                }
            }
            break;
        }
        case PROP_NUM_BUFFERS:
        {
            self->n_buffers = g_value_get_int (value);
            break;
        }
        case PROP_DO_TIMESTAMP:
        {
            gst_base_src_set_do_timestamp(GST_BASE_SRC(object),
                                          g_value_get_boolean(value));
            break;
        }
        default:
        {
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
        }
    }
}


static void gst_tcam_src_get_property (GObject* object,
                                       guint prop_id,
                                       GValue* value,
                                       GParamSpec* pspec)
{
    GstTcamSrc* self = GST_TCAM_SRC (object);

    switch (prop_id)
    {
        case PROP_SERIAL:
        {
            g_value_set_string(value, self->device_serial.c_str());
            break;
        }
        case PROP_DEVICE:
        {
            if (self->device != nullptr)
            {
                g_value_set_pointer(value, self->device->dev.get());
            }
            else
            {
                g_value_set_pointer (value, nullptr);
            }
            break;
        }
        case PROP_NUM_BUFFERS:
        {
            g_value_set_int (value, self->n_buffers);
            break;
        }
        case PROP_DO_TIMESTAMP:
        {
            g_value_set_boolean (value,
                                 gst_base_src_get_do_timestamp(GST_BASE_SRC(object)));
            break;
        }
        default:
        {
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
        }
    }
}


static void gst_tcam_src_class_init (GstTcamSrcClass* klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GstElementClass *element_class = GST_ELEMENT_CLASS (klass);
    GstBaseSrcClass *gstbasesrc_class = GST_BASE_SRC_CLASS (klass);
    GstPushSrcClass *gstpushsrc_class = GST_PUSH_SRC_CLASS (klass);

    gobject_class->finalize = gst_tcam_src_finalize;
    gobject_class->set_property = gst_tcam_src_set_property;
    gobject_class->get_property = gst_tcam_src_get_property;

    g_object_class_install_property
        (gobject_class,
         PROP_SERIAL,
         g_param_spec_string ("serial",
                              "Camera serial",
                              "Serial of the camera",
                              NULL,
                              G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
    g_object_class_install_property
        (gobject_class,
         PROP_DEVICE,
         g_param_spec_pointer ("camera",
                               "Camera Object",
                               "Camera instance to retrieve additional information",
                               G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
    g_object_class_install_property
        (gobject_class,
         PROP_NUM_BUFFERS,
         g_param_spec_int ("num-buffers",
                           "Number of Buffers",
                           "Number of buffers to send before ending pipeline",
                           0, G_MAXINT, GST_TCAM_SRC_DEFAULT_N_BUFFERS,
                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
    g_object_class_install_property
        (gobject_class,
         PROP_DO_TIMESTAMP,
         g_param_spec_boolean ("do-timestamp",
                               "Do timestamp",
                               "Apply current stream time to buffers",
                               true,
                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT));
    GST_DEBUG_CATEGORY_INIT (tcam_src_debug, "tcamsrc", 0, "tcam interface");

    gst_element_class_set_details_simple (element_class,
                                          "Tcam Video Source",
                                          "Source/Video",
                                          "Tcam based source",
                                          "The Imaging Source <support@theimagingsource.com>");

    gst_element_class_add_pad_template (element_class,
                                        gst_static_pad_template_get (&tcam_src_template));

    element_class->change_state = gst_tcam_src_change_state;

    gstbasesrc_class->get_caps = gst_tcam_src_get_caps;
    gstbasesrc_class->set_caps = gst_tcam_src_set_caps;
    gstbasesrc_class->fixate = gst_tcam_src_fixate_caps;
    gstbasesrc_class->start = gst_tcam_src_start;
    gstbasesrc_class->stop = gst_tcam_src_stop;
    gstbasesrc_class->negotiate = gst_tcam_src_negotiate;
    gstbasesrc_class->get_times = gst_tcam_src_get_times;

    gstpushsrc_class->create = gst_tcam_src_create;
}


static gboolean plugin_init (GstPlugin* plugin)
{
    return gst_element_register (plugin, "tcamsrc", GST_RANK_NONE, GST_TYPE_TCAM_SRC);
}

#ifndef PACKAGE
#define PACKAGE "tcam"
#endif


GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
                   GST_VERSION_MINOR,
                   tcamsrc,
                   "TCam Video Source",
                   plugin_init,
                   "1.0.0",
                   "Proprietary",
                   "tcamsrc",
                   "theimagingsource.com")
