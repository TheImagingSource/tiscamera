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
#include "gsttcambase.h"

#include "tcamprop.h"
#include "tcam.h"
#include <unistd.h>
#include <stdlib.h>

#include <stdio.h>
#include <vector>

#define GST_TCAM_DEFAULT_N_BUFFERS 10

GST_DEBUG_CATEGORY_STATIC (tcam_debug);
#define GST_CAT_DEFAULT tcam_debug


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
                                                GValue* type);

static gboolean gst_tcam_src_set_tcam_property (TcamProp* self,
                                                gchar* name,
                                                const GValue* value);

static GSList* gst_tcam_src_get_device_serials (TcamProp* self);

static gboolean gst_tcam_src_get_device_info (TcamProp* self,
                                              const char* serial,
                                              char** name,
                                              char** identifier,
                                              char** connection_type);

static void gst_tcam_src_prop_init (TcamPropInterface* iface)
{
    iface->get_property_names = gst_tcam_src_get_property_names;
    iface->get_property_type = gst_tcam_src_get_property_type;
    iface->get_property = gst_tcam_src_get_tcam_property;
    iface->set_property = gst_tcam_src_set_tcam_property;
    iface->get_device_serials = gst_tcam_src_get_device_serials;
    iface->get_device_info = gst_tcam_src_get_device_info;
}

G_DEFINE_TYPE_WITH_CODE (GstTcam, gst_tcam, GST_TYPE_PUSH_SRC,
                         G_IMPLEMENT_INTERFACE (TCAM_TYPE_PROP,
                                                gst_tcam_src_prop_init));


static gboolean get_property_by_name (GstTcam* self,
                                      gchar* name,
                                      struct tcam_device_property* prop)
{
    struct device_state* ds = (struct device_state*)self->device;

    std::vector<tcam::Property*> properties = ds->dev->get_available_properties();

    tcam::Property* p = ds->dev->get_property_by_name(name);

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
 * @self: a #GstTcamProp
 * @name: a #char* identifying the property to query
 *
 * Return the type of a property
 *
 * Returns: (transfer full): A string describing the property type
 */
static gchar* gst_tcam_src_get_property_type (TcamProp* iface, gchar* name)
{
    gchar* ret = NULL;
    GstTcam* self = GST_TCAM (iface);
    struct tcam_device_property prop;
    struct property_type_map map[] = {
        { TCAM_PROPERTY_TYPE_BOOLEAN, "boolean" },
        { TCAM_PROPERTY_TYPE_INTEGER, "integer" },
        { TCAM_PROPERTY_TYPE_DOUBLE, "double" },
        { TCAM_PROPERTY_TYPE_STRING, "string" },
        { TCAM_PROPERTY_TYPE_ENUMERATION, "enum" },
        { TCAM_PROPERTY_TYPE_BUTTON, "button" },
    };

    g_return_val_if_fail (self->device != NULL, NULL);

    g_return_val_if_fail (get_property_by_name (self,
                                                name,
                                                &prop), NULL);
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
 * @self: a #GstTcam
 *
 * Return a list of property names
 *
 * Returns: (element-type utf8) (transfer full): list of property names
 */
static GSList* gst_tcam_src_get_property_names(TcamProp* iface)
{
    GSList* ret = NULL;
    GstTcam* self = GST_TCAM (iface);
    struct device_state* ds = (struct device_state*)self->device;

    g_return_val_if_fail (self->device != NULL, NULL);

    std::vector<tcam::Property*> vec = ds->dev->get_available_properties();

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
                                                GValue* type)
{
    gboolean ret = TRUE;
    GstTcam *self = GST_TCAM (iface);
    struct device_state* ds = (struct device_state*)self->device;

    tcam::Property* property = ds->dev->get_property_by_name(name);

    if (property == nullptr)
    {
        GST_DEBUG_OBJECT (GST_TCAM (iface), "no property with name: '%s'", name );
        return FALSE;
    }

    struct tcam_device_property prop = property->get_struct();

    if (type)
    {
        g_value_init (type, G_TYPE_GTYPE);
    }

    switch (prop.type)
    {
        case TCAM_PROPERTY_TYPE_INTEGER:
        case TCAM_PROPERTY_TYPE_ENUMERATION:
            if (value)
            {
                g_value_init (value, G_TYPE_INT);
                g_value_set_int (value, prop.value.i.value);
            }
            if (min)
            {
                g_value_init (min, G_TYPE_INT);
                g_value_set_int (min, prop.value.i.min);
            }
            if (max)
            {
                g_value_init (max, G_TYPE_INT);
                g_value_set_int (max, prop.value.i.max);
            }
            if (def)
            {
                g_value_init (def, G_TYPE_INT);
                g_value_set_int (def, prop.value.i.default_value);
            }
            if (step)
            {
                g_value_init (step, G_TYPE_INT);
                g_value_set_int (step, prop.value.i.step);
            }
            if (type)
            {
                g_value_set_gtype (type, G_TYPE_INT);
            }
            break;
        case TCAM_PROPERTY_TYPE_DOUBLE:
            if (value)
            {
                g_value_init (value, G_TYPE_DOUBLE);
                g_value_set_int (value, prop.value.d.value);
            }
            if (min)
            {
                g_value_init (min, G_TYPE_DOUBLE);
                g_value_set_int (min, prop.value.d.min);
            }
            if (max)
            {
                g_value_init (max, G_TYPE_DOUBLE);
                g_value_set_int (max, prop.value.d.max);
            }
            if (def)
            {
                g_value_init (def, G_TYPE_DOUBLE);
                g_value_set_int (def, prop.value.d.default_value);
            }
            if (step)
            {
                g_value_init (step, G_TYPE_DOUBLE);
                g_value_set_int (step, prop.value.d.step);
            }
            if (type)
            {
                g_value_set_gtype (type, G_TYPE_DOUBLE);
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
                g_value_set_gtype (type, G_TYPE_STRING);
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
                g_value_set_gtype (type, G_TYPE_BOOLEAN);
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


static gboolean gst_tcam_src_set_tcam_property (TcamProp* iface,
                                                gchar* name,
                                                const GValue* value)
{
    gboolean ret = TRUE;
    GstTcam* self = GST_TCAM (iface);
    struct device_state* ds = (struct device_state*)self->device;

    tcam::Property* property = ds->dev->get_property_by_name(name);

    if (property == nullptr)
    {
        return FALSE;
    }

    struct tcam_device_property prop = property->get_struct();

    switch (prop.type)
    {
        case TCAM_PROPERTY_TYPE_INTEGER:
        case TCAM_PROPERTY_TYPE_ENUMERATION:
        {
            return property->set_value((int64_t)g_value_get_int(value));
        }
        case TCAM_PROPERTY_TYPE_DOUBLE:
        {
            property->set_value(g_value_get_double(value));
        }
        case TCAM_PROPERTY_TYPE_STRING:
        {
            return property->set_value(g_value_get_string (value));
        }
        case TCAM_PROPERTY_TYPE_BOOLEAN:
        {
            return property->set_value((bool)g_value_get_boolean(value));
        }
        case TCAM_PROPERTY_TYPE_BUTTON:
        {
            return((tcam::PropertyButton*)property)->activate();
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
    int count = devices.size();
    GSList* ret = NULL;

    if (count <= 0)
    {
        return NULL;
    }

    for (const auto& d : devices)
    {
        ret = g_slist_append (ret,
                              g_strndup (d.get_serial().c_str(),
                                         sizeof(d.get_serial().c_str())));
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
};


static GstStaticPadTemplate tcam_src_template = GST_STATIC_PAD_TEMPLATE ("src",
                                                                         GST_PAD_SRC,
                                                                         GST_PAD_ALWAYS,
                                                                         GST_STATIC_CAPS ("ANY"));

static GstCaps* gst_tcam_fixate_caps (GstBaseSrc* bsrc,
                                      GstCaps* caps);


static bool gst_tcam_fill_structure_fixed_resolution (GstStructure* structure,
                                                      const tcam::VideoFormatDescription format,
                                                      const tcam_resolution_description& res)
{

    std::vector<double> framerates = format.get_framerates(res.min_size);
    int framerate_count = framerates.size();

    GValue fps_list = {0};
    g_value_init(&fps_list, GST_TYPE_LIST);

    for (int f = 0; f < framerate_count; f++)
    {
        int frame_rate_numerator;
        int frame_rate_denominator;
        gst_util_double_to_fraction(framerates[f],
                                    &frame_rate_numerator,
                                    &frame_rate_denominator);

        GValue fraction = {0};
        g_value_init(&fraction, GST_TYPE_FRACTION);
        gst_value_set_fraction(&fraction, frame_rate_numerator, frame_rate_denominator);
        gst_value_list_append_value(&fps_list, &fraction);
        g_value_unset(&fraction);
    }

    gst_structure_set (structure,
                       "width", G_TYPE_INT, res.max_size.width,
                       "height", G_TYPE_INT, res.max_size.height,
                       NULL);

    gst_structure_take_value(structure, "framerate", &fps_list);

    return true;
}


static GstCaps* gst_tcam_get_all_camera_caps (GstTcam* self)
{
    GstCaps* caps;
    gint64* pixel_formats;
    double min_frame_rate, max_frame_rate;
    int min_height, min_width;
    int max_height, max_width;
    int n_pixel_formats;

    g_return_val_if_fail (GST_IS_TCAM (self), NULL);

    if (self->device == NULL)
    {
        return NULL;
    }

    struct device_state* ds = (struct device_state*)self->device;

    std::vector<tcam::VideoFormatDescription> format = ds->dev->get_available_video_formats();
    n_pixel_formats = format.size();

    GST_DEBUG("Found %i pixel formats", n_pixel_formats);

    caps = gst_caps_new_empty ();

    for (unsigned int i = 0; i < n_pixel_formats; i++)
    {
        if (format[i].get_fourcc() == 0)
        {
            GST_ERROR("Format has empty fourcc. Ignoring.");
            continue;
        }

        const char* caps_string = tcam_fourcc_to_gst_1_0_caps_string(format[i].get_fourcc());

        GST_DEBUG("Found '%s' pixel format string", caps_string);

        std::vector<struct tcam_resolution_description> res = format[i].get_resolutions();

        int res_count = res.size();

        GST_DEBUG("Found %i resolutions", res_count);


        for (unsigned int j = 0; j < res_count; j++)
        {

            min_width = res[j].min_size.width;
            min_height = res[j].min_size.height;

            max_width = res[j].max_size.width;
            max_height = res[j].max_size.height;

            if (caps_string != NULL)
            {


                if (res[j].type == TCAM_RESOLUTION_TYPE_RANGE)
                {
                    // std::vector<double> framerates = format[i].get_frame_rates(res[j]);
                    std::vector<struct tcam_image_size> framesizes = tcam::get_standard_resolutions(res[j].min_size, res[j].max_size);
                    framesizes.insert(framesizes.begin(), res[j].min_size);
                    framesizes.push_back(res[j].max_size);
                    for (const auto& reso : framesizes)
                    {
                        GstStructure* structure = gst_structure_from_string (caps_string, NULL);

                        std::vector<double> framerates = format[i].get_framerates(reso);

                        GValue fps_list = {0};
                        g_value_init(&fps_list, GST_TYPE_LIST);

                        for (int f = 0; f < framerates.size(); f++)
                        {
                            int frame_rate_numerator;
                            int frame_rate_denominator;
                            gst_util_double_to_fraction(framerates[f],
                                                        &frame_rate_numerator,
                                                        &frame_rate_denominator);

                            if ((frame_rate_denominator == 0) || (frame_rate_numerator == 0))
                            {
                                continue;
                            }

                            GValue fraction = {0};
                            g_value_init(&fraction, GST_TYPE_FRACTION);
                            gst_value_set_fraction(&fraction, frame_rate_numerator, frame_rate_denominator);
                            gst_value_list_append_value(&fps_list, &fraction);
                            g_value_unset(&fraction);
                        }


                        gst_structure_set (structure,
                                           "width", G_TYPE_INT, reso.width,
                                           "height", G_TYPE_INT, reso.height,
                                           NULL);

                        gst_structure_take_value(structure, "framerate", &fps_list);
                        gst_caps_append_structure (caps, structure);

                    }
                }
                else // fixed resolution
                {
                    GstStructure* structure = gst_structure_from_string (caps_string, NULL);

                    gst_tcam_fill_structure_fixed_resolution(structure, format[i], res[j]);
                    gst_caps_append_structure (caps, structure);

                }
            }
        }
    }

    GstStructure* structure = gst_structure_from_string ("ANY", NULL);

    // gst_tcam_fill_structure_fixed_resolution(structure, format[i], res[j]);
    gst_caps_append_structure (caps, structure);

    GST_INFO(gst_caps_to_string(caps));

    return caps;
}


static gboolean gst_tcam_negotiate (GstBaseSrc* basesrc)
{

    GstCaps *thiscaps;
    GstCaps *caps = NULL;
    GstCaps *peercaps = NULL;
    gboolean result = FALSE;

    GstTcam* self = GST_TCAM(basesrc);

    /* /\* We don't allow renegotiation, just return TRUE in that case *\/ */
    /* if (GST_V4L2_IS_ACTIVE (obj)) */
    /* return TRUE; */

    /* first see what is possible on our source pad */
    thiscaps = gst_pad_query_caps (GST_BASE_SRC_PAD (basesrc), NULL);
    GST_DEBUG_OBJECT (basesrc, "!!!!!caps of src: %" GST_PTR_FORMAT, thiscaps);

    // nothing or anything is allowed, we're done
    if (gst_caps_is_empty(thiscaps)|| gst_caps_is_any (thiscaps))
    {
        goto no_nego_needed;
    }
    /* get the peer caps */
    peercaps = gst_pad_peer_query_caps (GST_BASE_SRC_PAD (basesrc), thiscaps);
    GST_DEBUG_OBJECT (basesrc, "=====-caps of peer: %s", gst_caps_to_string(peercaps));

    if (!gst_caps_is_empty(peercaps) && !gst_caps_is_any (peercaps))
    {
        GstCaps *icaps = NULL;
        int i;

        /* Prefer the first caps we are compatible with that the peer proposed */
        for (i = 0; i < gst_caps_get_size (peercaps); i++)
        {
            /* get intersection */
            GstCaps *ipcaps = gst_caps_copy_nth (peercaps, i);

            GST_DEBUG_OBJECT (basesrc, "peer: %" GST_PTR_FORMAT, ipcaps);

            icaps = gst_caps_intersect (thiscaps, ipcaps);
            gst_caps_unref (ipcaps);

            if (!gst_caps_is_empty (icaps))
                break;

            gst_caps_unref (icaps);
            icaps = NULL;
        }
        GST_DEBUG_OBJECT (basesrc, "/////intersect: %" GST_PTR_FORMAT, icaps);

        if (icaps)
        {
            /* If there are multiple intersections pick the one with the smallest
             * resolution strictly bigger then the first peer caps */
            if (gst_caps_get_size (icaps) > 1)
            {
                GST_DEBUG_OBJECT(basesrc, "1111111111111");

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
                double frame_rate = 0.0;

                structure = gst_caps_get_structure (caps, 0);

                gst_structure_fixate_field_nearest_int (structure, "width", G_MAXINT);
                gst_structure_fixate_field_nearest_int (structure, "height", G_MAXINT);
                gst_structure_fixate_field_nearest_fraction (structure, "framerate", (double) (0.5 + frame_rate), 1);

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
            caps = gst_tcam_fixate_caps (basesrc, caps);
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


static GstCaps* gst_tcam_get_caps (GstBaseSrc* src,
                                   GstCaps* filter)
{
    GstTcam* self = GST_TCAM(src);
    GstCaps* caps;

    if (self->all_caps != NULL)
        caps = gst_caps_copy (self->all_caps);
    else
        caps = gst_caps_new_any ();

    GST_LOG_OBJECT (self, "Available caps = %" GST_PTR_FORMAT, caps);

    return caps;
}


static void gst_tcam_callback (const struct tcam_image_buffer* buffer,
                               void* data)
{
    GstTcam* self = GST_TCAM(data);

    self->ptr = buffer;
    self->new_buffer = TRUE;

    std::unique_lock<std::mutex> lck(self->mtx);

    self->cv.notify_all();
}


static gboolean gst_tcam_set_caps (GstBaseSrc* src,
                                   GstCaps* caps)
{

    GstTcam* self = GST_TCAM(src);
    struct device_state* ds = (struct device_state*)self->device;

    GstStructure *structure;

    int height = 0;
    int width = 0;
    const GValue* frame_rate;
    const char* caps_string;
    const char* format_string;

    GST_LOG_OBJECT (self, "Requested caps = %" GST_PTR_FORMAT, caps);

    ds->dev->stop_stream();
    ds->sink = nullptr;

    structure = gst_caps_get_structure (caps, 0);

    gst_structure_get_int (structure, "width", &width);
    gst_structure_get_int (structure, "height", &height);
    frame_rate = gst_structure_get_value (structure, "framerate");
    format_string = gst_structure_get_string (structure, "format");

    uint32_t fourcc = tcam_fourcc_from_gst_1_0_caps_string(gst_structure_get_name (structure), format_string);

    double framerate = (double) gst_value_get_fraction_numerator (frame_rate) /
        (double) gst_value_get_fraction_denominator (frame_rate);

    struct tcam_video_format format = {};

    format.fourcc = fourcc;
    format.width = width;
    format.height = height;
    format.framerate = framerate;

    bool ret = ds->dev->set_video_format(tcam::VideoFormat(format));

    if (!ret)
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
        gst_caps_unref (self->fixed_caps);

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
            gst_structure_set_value (structure, "framerate", frame_rate);

        gst_caps_append_structure (caps, structure);

        self->fixed_caps = caps;
    }
    else
    {
        self->fixed_caps = NULL;
    }
    GST_LOG_OBJECT (self, "Start acquisition");

    self->timestamp_offset = 0;
    self->last_timestamp = 0;

    ds->sink = std::make_shared<tcam::ImageSink>();

    ds->sink->registerCallback(gst_tcam_callback, self);

    ds->dev->start_stream(ds->sink);

    self->is_running = TRUE;
    GST_INFO("successfully set caps to: %s", gst_caps_to_string(caps));

    return TRUE;
}


bool gst_tcam_init_camera (GstTcam* self)
{
    GST_DEBUG_OBJECT (self, "Initializing device.");

    if (self->device != NULL)
    {
        delete self->device;
    }

    std::vector<tcam::DeviceInfo> infos = tcam::get_device_list();
    int dev_count = infos.size();

    GST_DEBUG_OBJECT (self, "Found %d devices.", dev_count);

    if (self->device_serial != NULL)
    {
        GST_DEBUG_OBJECT (self, "Searching for device with serial %s.", self->device_serial);
    }
    else
    {
        GST_DEBUG_OBJECT (self, "No serial given. Opening first available device.");
    }

    for (unsigned int i = 0; i < infos.size(); ++i)
    {
        if (self->device_serial != NULL)
        {
            if (strcmp(infos[i].get_serial().c_str(), self->device_serial) == 0)
            {
                GST_DEBUG_OBJECT (self, "Found device.");

                self->device = new struct device_state;
                ((struct device_state*)self->device)->dev = std::make_shared<tcam::CaptureDevice>(tcam::DeviceInfo(infos[i]));
                break;
            }
        }
        else
        {
            self->device = new struct device_state;
            ((struct device_state*)self->device)->dev = std::make_shared<tcam::CaptureDevice>(tcam::DeviceInfo(infos[i]));
            break;
        }
    }

    if (self->device == NULL)
    {
        GST_ERROR("Unable to open device.");
        /* TODO add pipeline termination */
        return false;
    }
    return true;
}


static void gst_tcam_close_camera (GstTcam* self)
{
    if (self->device != NULL)
    {
        struct device_state* ds = (struct device_state*)self->device;

        ds->dev->stop_stream();
        ds->sink = nullptr;
        delete self->device;
        self->device = NULL;
    }
}


static gboolean gst_tcam_start (GstBaseSrc *src)
{
    GstTcam* self = GST_TCAM(src);
    struct device_state* ds = (struct device_state*)self->device;

    self->run = 1000;
    self->is_running = TRUE;

    if (self->device == NULL)
    {
        if (!gst_tcam_init_camera(self))
        {
            return FALSE;
        }
    }
    self->all_caps = gst_tcam_get_all_camera_caps (self);

    return TRUE;
}


gboolean gst_tcam_stop (GstBaseSrc* src)
{
    GstTcam* self = GST_TCAM(src);
    struct device_state* ds = (struct device_state*)self->device;

    self->is_running = FALSE;

    ds->dev->stop_stream();
    ds->sink = nullptr;

    if (self->all_caps != NULL)
    {
        gst_caps_unref (self->all_caps);
        self->all_caps = NULL;
    }

    GST_DEBUG_OBJECT (self, "Stopped acquisition");

    return TRUE;
}


static void gst_tcam_get_times (GstBaseSrc* basesrc,
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


static GstFlowReturn gst_tcam_create (GstPushSrc* push_src,
                                      GstBuffer** buffer)
{
    guint64 timestamp_ns = 0;

    GstTcam* self = GST_TCAM (push_src);

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

    *buffer = gst_buffer_new_wrapped_full (0, self->ptr->pData, self->ptr->length,
                                           0, self->ptr->length, NULL, NULL);

    if (!gst_base_src_get_do_timestamp(GST_BASE_SRC(push_src)))
    {
        if (self->timestamp_offset == 0)
        {
            self->timestamp_offset = timestamp_ns;
            self->last_timestamp = timestamp_ns;
        }

        GST_BUFFER_DURATION (*buffer) = timestamp_ns - self->last_timestamp;

        (*buffer)->pts = timestamp_ns;

        self->last_timestamp = timestamp_ns;
    }

    GST_DEBUG_OBJECT (self, "Pushing buffer...");

    return GST_FLOW_OK;
}


static GstCaps* gst_tcam_fixate_caps (GstBaseSrc* bsrc,
                                      GstCaps* caps)
{
    GstTcam* self = GST_TCAM(bsrc);

    GstStructure* structure;
    gint width = 0;
    gint height = 0;
    double frame_rate = 0.0;

    structure = gst_caps_get_structure (caps, 0);

    gst_structure_fixate_field_nearest_int (structure, "width", width);
    gst_structure_fixate_field_nearest_int (structure, "height", height);
    gst_structure_fixate_field_nearest_fraction (structure, "framerate", (double) (0.5 + frame_rate), 1);

    GST_DEBUG_OBJECT (self, "Fixated caps to %s", gst_caps_to_string(caps));

    return GST_BASE_SRC_CLASS(gst_tcam_parent_class)->fixate(bsrc, caps);
}


static void gst_tcam_init (GstTcam* self)
{

    gst_base_src_set_live (GST_BASE_SRC (self), TRUE);
    gst_base_src_set_format (GST_BASE_SRC (self), GST_FORMAT_TIME);

    self->device_serial = NULL;

    self->n_buffers = GST_TCAM_DEFAULT_N_BUFFERS;
    self->payload = 0;

    self->device = NULL;
    self->all_caps = NULL;
    self->fixed_caps = NULL;
    self->is_running = TRUE;

    gst_tcam_init_camera(self);
}


static void gst_tcam_finalize (GObject* object)
{
    GstTcam* self = GST_TCAM (object);

    if (self->device != NULL)
    {
        self->device = NULL;
    }

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

    if (self->device_serial != NULL)
    {
        g_free (self->device_serial);
        self->device_serial = NULL;
    }

    G_OBJECT_CLASS (gst_tcam_parent_class)->finalize (object);
}


static void gst_tcam_set_property (GObject* object,
                                   guint prop_id,
                                   const GValue* value,
                                   GParamSpec* pspec)
{
    GstTcam* self = GST_TCAM (object);

    switch (prop_id)
    {
        case PROP_SERIAL:
        {
            GstState state;
            gst_element_get_state(GST_ELEMENT(self), &state, NULL, 200);
            if (state == GST_STATE_NULL)
            {
                g_free (self->device_serial);
                self->device_serial = g_strdup (g_value_get_string (value));
                GST_LOG_OBJECT (self, "Set camera name to %s", self->device_serial);

                gst_tcam_close_camera(self);

                gst_tcam_init_camera(self);
            }
            break;
        }
        case PROP_NUM_BUFFERS:
        {
            self->n_buffers = g_value_get_int (value);
            break;
        }
        default:
        {
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
        }
    }
}


static void gst_tcam_get_property (GObject* object,
                                   guint prop_id,
                                   GValue* value,
                                   GParamSpec* pspec)
{
    GstTcam* self = GST_TCAM (object);

    switch (prop_id)
    {
        case PROP_SERIAL:
            g_value_set_string (value, self->device_serial);
            break;
        case PROP_DEVICE:
            g_value_set_pointer (value, ((struct device_state*)self->device)->dev.get());
            break;
        case PROP_NUM_BUFFERS:
            g_value_set_int (value, self->n_buffers);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void gst_tcam_class_init (GstTcamClass* klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GstElementClass *element_class = GST_ELEMENT_CLASS (klass);
    GstBaseSrcClass *gstbasesrc_class = GST_BASE_SRC_CLASS (klass);
    GstPushSrcClass *gstpushsrc_class = GST_PUSH_SRC_CLASS (klass);

    gobject_class->finalize = gst_tcam_finalize;
    gobject_class->set_property = gst_tcam_set_property;
    gobject_class->get_property = gst_tcam_get_property;

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
                           "Number of video buffers to allocate for video frames",
                           1, G_MAXINT, GST_TCAM_DEFAULT_N_BUFFERS,
                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

    GST_DEBUG_CATEGORY_INIT (tcam_debug, "tcamsrc", 0, "tcam interface");

    gst_element_class_set_details_simple (element_class,
                                          "Tcam Video Source",
                                          "Source/Video",
                                          "Tcam based source",
                                          "The Imaging Source <support@theimagingsource.com>");

    gst_element_class_add_pad_template (element_class,
                                        gst_static_pad_template_get (&tcam_src_template));

    gstbasesrc_class->get_caps = gst_tcam_get_caps;
    gstbasesrc_class->set_caps = gst_tcam_set_caps;
    gstbasesrc_class->fixate = gst_tcam_fixate_caps;
    gstbasesrc_class->start = gst_tcam_start;
    gstbasesrc_class->stop = gst_tcam_stop;
    gstbasesrc_class->negotiate = gst_tcam_negotiate;
    gstbasesrc_class->get_times = gst_tcam_get_times;

    gstpushsrc_class->create = gst_tcam_create;
}


static gboolean plugin_init (GstPlugin* plugin)
{
    return gst_element_register (plugin, "tcamsrc", GST_RANK_NONE, GST_TYPE_TCAM);
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
