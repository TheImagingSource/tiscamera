/*
 * Copyright 2013 The Imaging Source Europe GmbH
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

/**
 * SECTION:element-gsttis_auto_exposure
 *
 * The tis_auto_exposure element handles automatic exposure and gain adjustments.
 *
 * processing is done in a simple manner:
 *
 * process only a few frames to give the camera time to adjust
 *
 * on selected frame determine brightness of the whole image
 *
 * The configuration from exposure and gain. Works under the following assumptions:
 * Gain should always be as low as possible.
 * Exposure can only be set to a certain value due to framerate limitations.
 *
 * This leads to following workflow:
 *
 * Determine image brightness by taking sample pixel and analyzing them.
 * If the brightness is not our prefered area, we have to adjust.
 * Calculate a new gain value. Since we prefer it small we only set it if it is smaller to the current one.
 * Next we calculate exposure
 * If exposure can not be moved, we increase gain to the already calculated value.
 *
 * In every case we check if exposure and gain can be switched,
 * meaning we try to reduce gain by increasing exposure.
 *
 *
 * Currently v4l2 and aravis compatible cameras are supported.
 *
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tcam.h"
#include  "tcamprop.h"

#include <math.h>
#include <stdlib.h>
#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include "gsttcamautoexposure.h"

#include "bayer.h"
#include "image_sampling.h"

GST_DEBUG_CATEGORY_STATIC (gst_tcamautoexposure_debug_category);
#define GST_CAT_DEFAULT gst_tcamautoexposure_debug_category

#define CLIP(val,l,h) ( (val) < (l) ? (l) : (val) > (h) ? (h) : (val) )


/* prototypes */

static void gst_tcamautoexposure_set_property (GObject* object,
                                               guint property_id,
                                               const GValue* value,
                                               GParamSpec* pspec);
static void gst_tcamautoexposure_get_property (GObject* object,
                                               guint property_id,
                                               GValue* value,
                                               GParamSpec* pspec);
static void gst_tcamautoexposure_finalize (GObject* object);

static GstFlowReturn gst_tcamautoexposure_transform_ip (GstBaseTransform* trans,
                                                        GstBuffer* buf);

static void init_camera_resources (GstTcamautoexposure* self);


enum
{
    PROP_0,
    PROP_AUTO_EXPOSURE,
    PROP_AUTO_GAIN,
    PROP_CAMERA,
    PROP_BRIGHTNESS_REFERENCE,
    PROP_EXPOSURE_MAX,
    PROP_GAIN_MAX,
    PROP_X0,
    PROP_Y0,
    PROP_X1,
    PROP_Y1,
};


/* prototypes */

static void gst_tcamautoexposure_set_property (GObject* object,
                                               guint property_id,
                                               const GValue* value,
                                               GParamSpec* pspec);
static void gst_tcamautoexposure_get_property (GObject* object,
                                               guint property_id,
                                               GValue* value,
                                               GParamSpec* pspec);
static void gst_tcamautoexposure_finalize (GObject* object);

static GstFlowReturn gst_tcamautoexposure_transform_ip (GstBaseTransform* trans, GstBuffer* buf);
static GstCaps* gst_tcamautoexposure_transform_caps (GstBaseTransform* trans,
                                                     GstPadDirection direction,
                                                     GstCaps* caps);

static void gst_tcamautoexposure_fixate_caps (GstBaseTransform* base,
                                              GstPadDirection direction,
                                              GstCaps* caps,
                                              GstCaps* othercaps);

static GSList* gst_tcamautoexposure_get_property_names(TcamProp* self);

static gchar *gst_tcamautoexposure_get_property_type (TcamProp* self, gchar* name);

static gboolean gst_tcamautoexposure_get_tcam_property (TcamProp* self,
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

static gboolean gst_tcamautoexposure_set_tcam_property (TcamProp* self,
                                                        gchar* name,
                                                        const GValue* value);

static GSList* gst_tcamautoexposure_get_tcam_menu_entries (TcamProp* self,
                                                           const gchar* name);

static GSList* gst_tcamautoexposure_get_device_serials (TcamProp* self);

static gboolean gst_tcamautoexposure_get_device_info (TcamProp* self,
                                                      const char* serial,
                                                      char** name,
                                                      char** identifier,
                                                      char** connection_type);

static void gst_tcamautoexposure_prop_init (TcamPropInterface* iface)
{
    iface->get_property_names = gst_tcamautoexposure_get_property_names;
    iface->get_property_type = gst_tcamautoexposure_get_property_type;
    iface->get_property = gst_tcamautoexposure_get_tcam_property;
    iface->get_menu_entries = gst_tcamautoexposure_get_tcam_menu_entries;
    iface->set_property = gst_tcamautoexposure_set_tcam_property;
    iface->get_device_serials = gst_tcamautoexposure_get_device_serials;
    iface->get_device_info = gst_tcamautoexposure_get_device_info;
}


G_DEFINE_TYPE_WITH_CODE (GstTcamautoexposure, gst_tcamautoexposure, GST_TYPE_BASE_TRANSFORM,
                         G_IMPLEMENT_INTERFACE (TCAM_TYPE_PROP,
                                                gst_tcamautoexposure_prop_init));


static const char* tcamautoexposure_property_id_to_string (guint id)
{
    switch (id)
    {
        case PROP_AUTO_EXPOSURE:
        {
            return "Exposure Auto";
        }
        case PROP_AUTO_GAIN:
        {
            return "Gain Auto";
        }
        case PROP_BRIGHTNESS_REFERENCE:
        {
            return "Brightness Reference";
        }
        case PROP_EXPOSURE_MAX:
        {
            return "Exposure Max";
        }
        case PROP_GAIN_MAX:
        {
            return "Gain Max";
        }
        case PROP_X0:
        {
            return "ROI X0";
        }
        case PROP_X1:
        {
            return "ROI X1";
        }
        case PROP_Y0:
        {
            return "ROI Y0";
        }
        case PROP_Y1:
        {
            return "ROI Y1";
        }
        default:
            return "";
    }
}

static guint tcamautoexposure_string_to_property_id (const char* str)
{
    if (str == nullptr)
    {
        return 0;
    }
    if (g_strcmp0(str, "Exposure Auto") == 0)
    {
        return PROP_AUTO_EXPOSURE;
    }
    else if (g_strcmp0(str, "Gain Auto") == 0)
    {
        return PROP_AUTO_GAIN;
    }
    else if (g_strcmp0(str, "Brightness Reference") == 0)
    {
        return PROP_BRIGHTNESS_REFERENCE;
    }
    else if (g_strcmp0(str, "Exposure Max") == 0)
    {
        return PROP_EXPOSURE_MAX;
    }
    else if (g_strcmp0(str, "Gain Max") == 0)
    {
        return PROP_GAIN_MAX;
    }
    else if (g_strcmp0(str, "ROI X0") == 0)
    {
        return PROP_X0;
    }
    else if (g_strcmp0(str, "ROI X1") == 0)
    {
        return PROP_X1;
    }
    else if (g_strcmp0(str, "ROI Y0") == 0)
    {
        return PROP_Y0;
    }
    else if (g_strcmp0(str, "ROI Y1") == 0)
    {
        return PROP_Y1;
    }
    return 0;
}



static GSList* gst_tcamautoexposure_get_property_names (TcamProp* self)
{
    GSList* names = nullptr;

    names = g_slist_append(names, tcamautoexposure_property_id_to_string(PROP_AUTO_EXPOSURE));
    names = g_slist_append(names, tcamautoexposure_property_id_to_string(PROP_AUTO_GAIN));
    names = g_slist_append(names, tcamautoexposure_property_id_to_string(PROP_BRIGHTNESS_REFERENCE));
    names = g_slist_append(names, tcamautoexposure_property_id_to_string(PROP_EXPOSURE_MAX));
    names = g_slist_append(names, tcamautoexposure_property_id_to_string(PROP_GAIN_MAX));
    names = g_slist_append(names, tcamautoexposure_property_id_to_string(PROP_X0));
    names = g_slist_append(names, tcamautoexposure_property_id_to_string(PROP_X1));
    names = g_slist_append(names, tcamautoexposure_property_id_to_string(PROP_Y0));
    names = g_slist_append(names, tcamautoexposure_property_id_to_string(PROP_Y1));

    return names;
}


static gchar* gst_tcamautoexposure_get_property_type (TcamProp* self, gchar* name)
{
    if (name == nullptr)
    {
        return 0;
    }

    if (g_strcmp0(name, "Exposure Auto") == 0)
    {
        return strdup("boolean");
    }
    else if (g_strcmp0(name, "Gain Auto") == 0)
    {
        return strdup("boolean");
    }
    else if (g_strcmp0(name, "Brightness Reference") == 0)
    {
        return strdup("integer");
    }
    else if (g_strcmp0(name, "Exposure Max") == 0)
    {
        return strdup("integer");
    }
    else if (g_strcmp0(name, "Gain Max") == 0)
    {
        return strdup("integer");
    }
    else if (g_strcmp0(name, "ROI X0") == 0)
    {
        return strdup("integer");
    }
    else if (g_strcmp0(name, "ROI X1") == 0)
    {
        return strdup("integer");
    }
    else if (g_strcmp0(name, "ROI Y0") == 0)
    {
        return strdup("integer");
    }
    else if (g_strcmp0(name, "ROI Y1") == 0)
    {
        return strdup("integer");
    }
    return nullptr;
}


static gboolean gst_tcamautoexposure_get_tcam_property (TcamProp* prop,
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
    if (name == nullptr)
    {
        return 0;
    }

    GstTcamautoexposure* self = GST_TCAMAUTOEXPOSURE(prop);

    flags = nullptr;

    if (g_strcmp0(name, "Exposure Auto") == 0)
    {
        if (value)
        {
            g_value_init(value, G_TYPE_BOOLEAN);
            g_value_set_boolean(value, self->auto_exposure);
        }
        if (min)
        {
            g_value_init(min, G_TYPE_BOOLEAN);
            g_value_set_boolean(min, FALSE);
        }
        if (max)
        {
            g_value_init(max, G_TYPE_BOOLEAN);
            g_value_set_boolean(max, TRUE);
        }
        if (def)
        {
            g_value_init(def, G_TYPE_BOOLEAN);
            g_value_set_boolean(def, TRUE);
        }
        if (step)
        {
            g_value_init(step, G_TYPE_INT);
            g_value_set_int(step, 1);
        }
        if (type)
        {
            g_value_init(type, G_TYPE_STRING);
            g_value_set_string(type, gst_tcamautoexposure_get_property_type(prop, name));
        }
        if (category)
        {
            g_value_init(category, G_TYPE_STRING);
            g_value_set_string(category, "");
        }
        if (group)
        {
            g_value_init(group, G_TYPE_STRING);
            g_value_set_string(group, "");
        }
        return TRUE;
    }
    else if (g_strcmp0(name, "Gain Auto") == 0)
    {
        if (value)
        {
            g_value_init(value, G_TYPE_BOOLEAN);
            g_value_set_boolean(value, self->auto_gain);
        }
        if (min)
        {
            g_value_init(min, G_TYPE_BOOLEAN);
            g_value_set_boolean(min, FALSE);
        }
        if (max)
        {
            g_value_init(max, G_TYPE_BOOLEAN);
            g_value_set_boolean(max, TRUE);
        }
        if (def)
        {
            g_value_init(def, G_TYPE_BOOLEAN);
            g_value_set_boolean(def, TRUE);
        }
        if (step)
        {
            g_value_init(step, G_TYPE_INT);
            g_value_set_int(step, 1);
        }
        if (type)
        {
            g_value_init(type, G_TYPE_STRING);
            g_value_set_string(type, gst_tcamautoexposure_get_property_type(prop, name));
        }
        if (category)
        {
            g_value_init(category, G_TYPE_STRING);
            g_value_set_string(category, "");
        }
        if (group)
        {
            g_value_init(group, G_TYPE_STRING);
            g_value_set_string(group, "");
        }
        return TRUE;
    }
    else if (g_strcmp0(name, "Brightness Reference") == 0)
    {
        if (value)
        {
            g_value_init(value, G_TYPE_INT);
            g_value_set_int(value, self->brightness_reference);
        }
        if (min)
        {
            g_value_init(min, G_TYPE_INT);
            g_value_set_int(min, 0);
        }
        if (max)
        {
            g_value_init(max, G_TYPE_INT);
            g_value_set_int(max, 255);
        }
        if (def)
        {
            g_value_init(def, G_TYPE_INT);
            g_value_set_int(def, 128);
        }
        if (step)
        {
            g_value_init(step, G_TYPE_INT);
            g_value_set_int(step, 1);
        }
        if (type)
        {
            g_value_init(type, G_TYPE_STRING);
            g_value_set_string(type,gst_tcamautoexposure_get_property_type(prop, name));
        }
        if (category)
        {
            g_value_init(category, G_TYPE_STRING);
            g_value_set_string(category, "");
        }
        if (group)
        {
            g_value_init(group, G_TYPE_STRING);
            g_value_set_string(group, "");
        }
        return TRUE;
    }
    else if (g_strcmp0(name, "Exposure Max") == 0)
    {
        if (value)
        {
            g_value_init(value, G_TYPE_INT);
            g_value_set_int(value, self->exposure.value);
        }
        if (min)
        {
            g_value_init(min, G_TYPE_INT);
            g_value_set_int(min, self->exposure.min);
        }
        if (max)
        {
            g_value_init(max, G_TYPE_INT);
            g_value_set_int(max, self->exposure.max);
        }
        if (def)
        {
            g_value_init(def, G_TYPE_INT);
            g_value_set_int(def, 0);
        }
        if (step)
        {
            g_value_init(step, G_TYPE_INT);
            g_value_set_int(step, 1);
        }
        if (type)
        {
            g_value_init(type, G_TYPE_STRING);
            g_value_set_string(type,gst_tcamautoexposure_get_property_type(prop, name));
        }
        if (category)
        {
            g_value_init(category, G_TYPE_STRING);
            g_value_set_string(category, "");
        }
        if (group)
        {
            g_value_init(group, G_TYPE_STRING);
            g_value_set_string(group, "");
        }
        return TRUE;
    }
    else if (g_strcmp0(name, "Gain Max") == 0)
    {
        if (value)
        {
            g_value_init(value, G_TYPE_INT);
            g_value_set_int(value, self->gain.value);
        }
        if (min)
        {
            g_value_init(min, G_TYPE_INT);
            g_value_set_int(min, self->gain.min);
        }
        if (max)
        {
            g_value_init(max, G_TYPE_INT);
            g_value_set_int(max, self->gain.max);
        }
        if (def)
        {
            g_value_init(def, G_TYPE_INT);
            g_value_set_int(def, 0);
        }
        if (step)
        {
            g_value_init(step, G_TYPE_INT);
            g_value_set_int(step, 1);
        }
        if (type)
        {
            g_value_init(type, G_TYPE_STRING);
            g_value_set_string(type,gst_tcamautoexposure_get_property_type(prop, name));
        }
        if (category)
        {
            g_value_init(category, G_TYPE_STRING);
            g_value_set_string(category, "");
        }
        if (group)
        {
            g_value_init(group, G_TYPE_STRING);
            g_value_set_string(group, "");
        }

        return TRUE;
    }
    else if (g_strcmp0(name, "ROI X0") == 0)
    {
        if (value)
        {
            g_value_init(value, G_TYPE_INT);
            g_value_set_int(value, self->image_region.x0);
        }
        if (min)
        {
            g_value_init(min, G_TYPE_INT);
            g_value_set_int(min, 0);
        }
        if (max)
        {
            g_value_init(max, G_TYPE_INT);
            g_value_set_int(max, G_MAXINT);
        }
        if (def)
        {
            g_value_init(def, G_TYPE_INT);
            g_value_set_int(def, 0);
        }
        if (step)
        {
            g_value_init(step, G_TYPE_INT);
            g_value_set_int(step, 1);
        }
        if (type)
        {
            g_value_init(type, G_TYPE_STRING);
            g_value_set_string(type,gst_tcamautoexposure_get_property_type(prop, name));
        }
        if (category)
        {
            g_value_init(category, G_TYPE_STRING);
            g_value_set_string(category, "");
        }
        if (group)
        {
            g_value_init(group, G_TYPE_STRING);
            g_value_set_string(group, "");
        }
        return TRUE;
    }
    else if (g_strcmp0(name, "ROI X1") == 0)
    {
        if (value)
        {
            g_value_init(value, G_TYPE_INT);
            g_value_set_int(value, self->image_region.x1);
        }
        if (min)
        {
            g_value_init(min, G_TYPE_INT);
            g_value_set_int(min, 0);
        }
        if (max)
        {
            g_value_init(max, G_TYPE_INT);
            g_value_set_int(max, G_MAXINT);
        }
        if (def)
        {
            g_value_init(def, G_TYPE_INT);
            g_value_set_int(def, 0);
        }
        if (step)
        {
            g_value_init(step, G_TYPE_INT);
            g_value_set_int(step, 1);
        }
        if (type)
        {
            g_value_init(type, G_TYPE_STRING);
            g_value_set_string(type,gst_tcamautoexposure_get_property_type(prop, name));
        }
        if (category)
        {
            g_value_init(category, G_TYPE_STRING);
            g_value_set_string(category, "");
        }
        if (group)
        {
            g_value_init(group, G_TYPE_STRING);
            g_value_set_string(group, "");
        }
        return TRUE;
    }
    else if (g_strcmp0(name, "ROI Y0") == 0)
    {
        if (value)
        {
            g_value_init(value, G_TYPE_INT);
            g_value_set_int(value, self->image_region.y0);
        }
        if (min)
        {
            g_value_init(min, G_TYPE_INT);
            g_value_set_int(min, 0);
        }
        if (max)
        {
            g_value_init(max, G_TYPE_INT);
            g_value_set_int(max, G_MAXINT);
        }
        if (def)
        {
            g_value_init(def, G_TYPE_INT);
            g_value_set_int(def, 0);
        }
        if (step)
        {
            g_value_init(step, G_TYPE_INT);
            g_value_set_int(step, 1);
        }
        if (type)
        {
            g_value_init(type, G_TYPE_STRING);
            g_value_set_string(type,gst_tcamautoexposure_get_property_type(prop, name));
        }
        if (category)
        {
            g_value_init(category, G_TYPE_STRING);
            g_value_set_string(category, "");
        }
        if (group)
        {
            g_value_init(group, G_TYPE_STRING);
            g_value_set_string(group, "");
        }
        return TRUE;
    }
    else if (g_strcmp0(name, "ROI Y1") == 0)
    {
        if (value)
        {
            g_value_init(value, G_TYPE_INT);
            g_value_set_int(value, self->image_region.y1);
        }
        if (min)
        {
            g_value_init(min, G_TYPE_INT);
            g_value_set_int(min, 0);
        }
        if (max)
        {
            g_value_init(max, G_TYPE_INT);
            g_value_set_int(max, G_MAXINT);
        }
        if (def)
        {
            g_value_init(def, G_TYPE_INT);
            g_value_set_int(def, 0);
        }
        if (step)
        {
            g_value_init(step, G_TYPE_INT);
            g_value_set_int(step, 1);
        }
        if (type)
        {
            g_value_init(type, G_TYPE_STRING);
            g_value_set_string(type,gst_tcamautoexposure_get_property_type(prop, name));
        }
        if (category)
        {
            g_value_init(category, G_TYPE_STRING);
            g_value_set_string(category, "");
        }
        if (group)
        {
            g_value_init(group, G_TYPE_STRING);
            g_value_set_string(group, "");
        }
        return TRUE;
    }

    return 0;

    return FALSE;
}


static gboolean gst_tcamautoexposure_set_tcam_property (TcamProp* self,
                                                        gchar* name,
                                                        const GValue* value)
{
    guint id = tcamautoexposure_string_to_property_id(name);
    if (id == 0)
    {
        return FALSE;
    }
    gst_tcamautoexposure_set_property(G_OBJECT(self), id, value, NULL);
    return TRUE;
}

static GSList* gst_tcamautoexposure_get_tcam_menu_entries (TcamProp* self,
                                                           const gchar* name)
{
    return nullptr;
}


static GSList* gst_tcamautoexposure_get_device_serials (TcamProp* self)
{
    return nullptr;
}


static gboolean gst_tcamautoexposure_get_device_info (TcamProp* self,
                                                      const char* serial,
                                                      char** name,
                                                      char** identifier,
                                                      char** connection_type)
{
    return FALSE;
}
/* pad templates */

static GstStaticPadTemplate gst_tcamautoexposure_sink_template =
    GST_STATIC_PAD_TEMPLATE ("sink",
                             GST_PAD_SINK,
                             GST_PAD_ALWAYS,
                             GST_STATIC_CAPS ("ANY"));

static GstStaticPadTemplate gst_tcamautoexposure_src_template =
    GST_STATIC_PAD_TEMPLATE ("src",
                             GST_PAD_SRC,
                             GST_PAD_ALWAYS,
                             GST_STATIC_CAPS ("ANY"));


/* class initialization */

static void gst_tcamautoexposure_class_init (GstTcamautoexposureClass* klass)
{
    GObjectClass* gobject_class = G_OBJECT_CLASS (klass);
    GstBaseTransformClass* base_transform_class = GST_BASE_TRANSFORM_CLASS (klass);

    gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
                                        gst_static_pad_template_get(&gst_tcamautoexposure_src_template));
    gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
                                        gst_static_pad_template_get(&gst_tcamautoexposure_sink_template));

    gst_element_class_set_details_simple (GST_ELEMENT_CLASS(klass),
                                          "The Imaging Source Brightness Balance Element",
                                          "Generic",
                                          "Adjusts the image brightness by setting camera properties.",
                                          "The Imaging Source Europe GmbH <support@theimagingsource.com>");

    GST_DEBUG_CATEGORY_INIT(gst_tcamautoexposure_debug_category, "tcamautoexposure", 0, "tcam autoexposure");


    gobject_class->set_property = gst_tcamautoexposure_set_property;
    gobject_class->get_property = gst_tcamautoexposure_get_property;
    gobject_class->finalize = gst_tcamautoexposure_finalize;
    base_transform_class->transform_ip = gst_tcamautoexposure_transform_ip;
    // base_transform_class->transform_caps = gst_tcamautoexposure_transform_caps;
    //base_transform_class->fixate_caps = gst_tcamautoexposure_fixate_caps;

    g_object_class_install_property (gobject_class,
                                     PROP_AUTO_EXPOSURE,
                                     g_param_spec_boolean ("auto-exposure",
                                                           "Auto Exposure",
                                                           "Automatically adjust exposure",
                                                           TRUE,
                                                           G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    g_object_class_install_property (gobject_class,
                                     PROP_AUTO_GAIN,
                                     g_param_spec_boolean ("auto-gain",
                                                           "Auto Gain",
                                                           "Automatically adjust gain",
                                                           TRUE,
                                                           G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    g_object_class_install_property (gobject_class,
                                     PROP_EXPOSURE_MAX,
                                     g_param_spec_double ("exposure-max",
                                                          "Exposure Maximum",
                                                          "Maximum value exposure can take",
                                                          0.0, G_MAXDOUBLE, 0.0,
                                                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    g_object_class_install_property (gobject_class,
                                     PROP_GAIN_MAX,
                                     g_param_spec_double ("gain-max",
                                                          "Gain Maximum",
                                                          "Maximum value gain can take",
                                                          0.0, G_MAXDOUBLE, 0.0,
                                                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    g_object_class_install_property (gobject_class,
                                     PROP_BRIGHTNESS_REFERENCE,
                                     g_param_spec_int("brightness-reference",
                                                      "Brightness Reference",
                                                      "Ideal average brightness of buffer",
                                                      0, 255, 128,
                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    g_object_class_install_property(gobject_class,
                                    PROP_X0,
                                    g_param_spec_uint("region-x0",
                                                      "Upper, left boundary",
                                                      "",
                                                      0, G_MAXUINT, 0,
                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT)) ;

    g_object_class_install_property(gobject_class,
                                    PROP_Y0,
                                    g_param_spec_uint("region-y0",
                                                      "Upper, left boundary",
                                                      "",
                                                      0, G_MAXUINT, 0,
                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT)) ;

    g_object_class_install_property(gobject_class,
                                    PROP_X1,
                                    g_param_spec_uint("region-x1",
                                                      "Lower, right boundary",
                                                      "",
                                                      0, G_MAXUINT, 0,
                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT)) ;

    g_object_class_install_property(gobject_class,
                                    PROP_Y1,
                                    g_param_spec_uint("region-y1",
                                                      "Lower, right boundary",
                                                      "",
                                                      0, G_MAXUINT, 0,
                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT)) ;

    g_object_class_install_property (gobject_class,
                                     PROP_CAMERA,
                                     g_param_spec_object ("camera",
                                                          "camera gst element",
                                                          "Gstreamer element that shall be manipulated",
                                                          GST_TYPE_ELEMENT,
                                                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}

static void gst_tcamautoexposure_init (GstTcamautoexposure *self)
{
    self->auto_exposure = TRUE;
    self->auto_gain = TRUE;

    self->frame_counter = 0;
    self->camera_src = NULL;
}

void gst_tcamautoexposure_set_property (GObject* object,
                                        guint property_id,
                                        const GValue* value,
                                        GParamSpec* pspec)
{
    GstTcamautoexposure* tcamautoexposure = GST_TCAMAUTOEXPOSURE (object);

    switch (property_id)
    {
        case PROP_AUTO_EXPOSURE:
            tcamautoexposure->auto_exposure = g_value_get_boolean(value);
            break;
        case PROP_AUTO_GAIN:
            tcamautoexposure->auto_gain = g_value_get_boolean(value);
            break;
        case PROP_CAMERA:
            tcamautoexposure->camera_src = (GstElement*)g_value_get_object(value);
            break;
        case PROP_EXPOSURE_MAX:
            tcamautoexposure->exposure.max = g_value_get_double(value);
            if (tcamautoexposure->exposure.max == 0.0)
                tcamautoexposure->exposure = tcamautoexposure->default_exposure_values;
            break;
        case PROP_GAIN_MAX:
            tcamautoexposure->gain.max = g_value_get_double(value);
            if (tcamautoexposure->gain.max == 0.0)
                tcamautoexposure->gain = tcamautoexposure->default_gain_values;
            break;
        case PROP_BRIGHTNESS_REFERENCE:
            tcamautoexposure->brightness_reference = g_value_get_int(value);
            break;
        case PROP_X0:
            tcamautoexposure->image_region.x0 = g_value_get_uint(value);
            break;
        case PROP_Y0:
            tcamautoexposure->image_region.y0 = g_value_get_uint(value);
            break;
        case PROP_X1:
            tcamautoexposure->image_region.x1 = g_value_get_uint(value);
            break;
        case PROP_Y1:
            tcamautoexposure->image_region.y1 = g_value_get_uint(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

void gst_tcamautoexposure_get_property (GObject* object,
                                        guint property_id,
                                        GValue* value,
                                        GParamSpec* pspec)
{
    GstTcamautoexposure* tcamautoexposure = GST_TCAMAUTOEXPOSURE(object);

    switch (property_id)
    {
        case PROP_AUTO_EXPOSURE:
            g_value_set_boolean (value, tcamautoexposure->auto_exposure);
            break;
        case PROP_AUTO_GAIN:
            g_value_set_boolean (value, tcamautoexposure->auto_gain);
            break;
        case PROP_CAMERA:
            g_value_set_object (value, tcamautoexposure->camera_src);
            break;
        case PROP_EXPOSURE_MAX:
            g_value_set_double(value, tcamautoexposure->exposure.max);
            break;
        case PROP_GAIN_MAX:
            g_value_set_double(value, tcamautoexposure->gain.max);
            break;
        case PROP_BRIGHTNESS_REFERENCE:
            g_value_set_int(value, tcamautoexposure->brightness_reference);
            break;
        case PROP_X0:
            g_value_set_uint(value, tcamautoexposure->image_region.x0);
            break;
        case PROP_Y0:
            g_value_set_uint(value, tcamautoexposure->image_region.y0);
            break;
        case PROP_X1:
            g_value_set_uint(value, tcamautoexposure->image_region.x1);
            break;
        case PROP_Y1:
            g_value_set_uint(value, tcamautoexposure->image_region.y1);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

void gst_tcamautoexposure_finalize (GObject* object)
{
    G_OBJECT_CLASS (gst_tcamautoexposure_parent_class)->finalize (object);
}


static void init_camera_resources (GstTcamautoexposure* self)
{
    /* retrieve the element name e.g. GstAravis or GstV4l2Src*/
    const char* element_name = g_type_name(gst_element_factory_get_element_type (gst_element_get_factory(self->camera_src)));

    tcam::CaptureDevice* dev = NULL;

    g_object_get(G_OBJECT(self->camera_src), "camera", &dev, NULL);

    struct tcam_device_property p = {0};

    tcam::Property* prop = dev->get_property(TCAM_PROPERTY_EXPOSURE);

    if (prop == nullptr)
    {
        gst_debug_log (gst_tcamautoexposure_debug_category,
                       GST_LEVEL_ERROR,
                       "tcamautoexposure",
                       "init_camera_resources",
                       __LINE__,
                       NULL,
                       "Exposure could not be found!");
    }
    else
    {
        p = prop->get_struct();
        self->exposure.min = p.value.i.min;
        self->exposure.max = p.value.i.max;
        self->exposure.value = p.value.i.value;

        self->default_exposure_values.max = 1000000 / (self->framerate_numerator / self->framerate_denominator);
    }

    p = {0};
    prop = dev->get_property(TCAM_PROPERTY_GAIN);

    if (prop == nullptr)
    {
        gst_debug_log (gst_tcamautoexposure_debug_category,
                       GST_LEVEL_ERROR,
                       "tcamautoexposure",
                       "init_camera_resources",
                       __LINE__,
                       NULL,
                       "Gain could not be found!");
    }
    else
    {
        p = prop->get_struct();
        self->gain.min = p.value.i.min;
        self->gain.max = p.value.i.max;
        self->gain.value = p.value.i.value;
    }


    self->exposure.max = self->default_exposure_values.max;

    gst_debug_log (gst_tcamautoexposure_debug_category,
                   GST_LEVEL_INFO,
                   "tcamautoexposure",
                   "init_camera_resources",
                   __LINE__,
                   NULL,
                   "Exposure boundaries are %f %f", self->exposure.min, self->exposure.max);

    gst_debug_log (gst_tcamautoexposure_debug_category,
                   GST_LEVEL_INFO,
                   "tcamautoexposure",
                   "init_camera_resources",
                   __LINE__,
                   NULL,
                   "Gain boundaries are %f %f", self->gain.min, self->gain.max);
}


static unsigned int calc_dist (unsigned int reference_val, unsigned int brightness_val)
{
    if ( brightness_val == 0 )
        brightness_val = 1;
    return (reference_val * dist_mid) / brightness_val;
}


static void set_exposure (GstTcamautoexposure* self, gdouble exposure)
{
    gst_debug_log (gst_tcamautoexposure_debug_category,
                   GST_LEVEL_INFO,
                   "tcamautoexposure",
                   "set_exposure",
                   __LINE__,
                   NULL,
                   "Setting exposure to %f", exposure);

    tcam::CaptureDevice* dev;
    g_object_get(G_OBJECT(self->camera_src), "camera", &dev, NULL);

    dev->set_property(TCAM_PROPERTY_EXPOSURE, (int64_t)exposure);
}


static gdouble calc_exposure (GstTcamautoexposure* self, guint dist, gdouble exposure)
{
    gdouble ddist = ( dist + (dist_mid * 2) ) / 3;
    exposure = ( exposure * ddist ) / dist_mid;

    int granularity = 1;

    /* If we do not want a significant change (on the sensor-scale), don't change anything */
    /* This should avoid pumping caused by an abrupt brightness change caused by a small value change */
    if ( abs(exposure - self->exposure.value) < (granularity / 2) )
        return self->exposure.value;

    return CLIP( exposure, self->exposure.min, self->exposure.max );
}


static void set_gain (GstTcamautoexposure* self, gdouble gain)
{
    gst_debug_log (gst_tcamautoexposure_debug_category,
                   GST_LEVEL_INFO,
                   "",
                   "",
                   __LINE__,
                   NULL,
                   "Setting gain to %f", gain);

    tcam::CaptureDevice* dev;
    g_object_get(G_OBJECT(self->camera_src), "camera", &dev, NULL);

    dev->set_property(TCAM_PROPERTY_GAIN, (int64_t)gain);
}


static gdouble calc_gain (GstTcamautoexposure* self, guint dist)
{

    gst_debug_log (gst_tcamautoexposure_debug_category,
                   GST_LEVEL_DEBUG,
                   "calc_gain",
                   "",
                   __LINE__,
                   NULL,
                   "Calculating gain: cur: %f min: %f max: %f",
                   self->gain.value, self->gain.min, self->gain.max);

    gdouble gain = self->gain.value;

    /* when we have to reduce, we reduce it faster */
    if ( dist >= dist_mid )
    {
        /* this dampens the change in dist factor */
        dist = ( dist + (dist_mid * 2) ) / 3;
    }
    double val = log( dist / (double)dist_mid ) / log( 2.0f );

    gain += (val * steps_to_double_brightness);


    gst_debug_log (gst_tcamautoexposure_debug_category,
                   GST_LEVEL_DEBUG,
                   "calc_gain",
                   "",
                   __LINE__,
                   NULL,
                   "new gain: %f",
                   gain);


    return CLIP( gain, self->gain.min, self->gain.max );
}


void retrieve_current_values (GstTcamautoexposure* self)
{
    tcam::CaptureDevice* dev = NULL;
    g_object_get(G_OBJECT(self->camera_src), "camera", &dev, NULL);

    struct tcam_device_property p = {};

    if (dev == NULL)
    {
        gst_debug_log (gst_tcamautoexposure_debug_category,
                       GST_LEVEL_ERROR,
                       "",
                       "",
                       __LINE__,
                       NULL,
                       "Tcam did not return a valid device");
    }

    tcam::Property* prop =  dev->get_property(TCAM_PROPERTY_EXPOSURE);

    if (prop == nullptr)
    {
        gst_debug_log (gst_tcamautoexposure_debug_category,
                       GST_LEVEL_ERROR,
                       "",
                       "",
                       __LINE__,
                       NULL,
                       "Tcam did not return exposure");
    }
    else
    {
        p = prop->get_struct();
        gst_debug_log (gst_tcamautoexposure_debug_category,
                       GST_LEVEL_DEBUG,
                       "",
                       "",
                       __LINE__,
                       NULL,
                       "Current exposure is %ld", p.value.i.value);
        self->exposure.value = p.value.i.value;
    }

    prop =  dev->get_property(TCAM_PROPERTY_GAIN);

    if (prop == nullptr)
    {
        gst_debug_log (gst_tcamautoexposure_debug_category,
                       GST_LEVEL_ERROR,
                       "",
                       "",
                       __LINE__,
                       NULL,
                       "Tcam did not return gain");
    }
    else
    {
        p = prop->get_struct();
        gst_debug_log (gst_tcamautoexposure_debug_category,
                       GST_LEVEL_DEBUG,
                       "",
                       "",
                       __LINE__,
                       NULL,
                       "Current gain is %ld", p.value.i.value);
        self->gain.value = p.value.i.value;
    }

}


static tBY8Pattern calculate_pattern_from_offset (GstTcamautoexposure* self)
{
    tBY8Pattern ret = self->pattern;
    // check if we need to switch between pattern lines
    if ((self->image_region.y0 % 2) != 0)
    {
        ret = next_line(self->pattern);
    }

    if ((self->image_region.x0 % 2) != 0)
    {
        ret = next_pixel(ret);
    }

    return ret;
}


static image_buffer retrieve_image_region (GstTcamautoexposure* self, GstBuffer* buf)
{
    GstMapInfo info;

    gst_buffer_map(buf, &info, GST_MAP_READ);

    if (self->image_region.x0 == 0
        && self->image_region.x1 == 0
        && self->image_region.y0 == 0
        && self->image_region.y1 == 0)
    {
        self->image_region.x1 = self->image_size.width;
        self->image_region.y1 = self->image_size.height;
    }

    const int bytes_per_pixel = 1;

    image_buffer new_buf;

    new_buf.image = info.data + (self->image_region.x0 * bytes_per_pixel * self->image_region.y0);

    new_buf.width = self->image_region.x1 - self->image_region.x0;
    new_buf.height = self->image_region.y1 - self->image_region.y0;

    new_buf.pattern = calculate_pattern_from_offset(self);

    gst_debug_log (gst_tcamautoexposure_debug_category,
                   GST_LEVEL_INFO,
                   "tis_auto_exposure",
                   "init_camera_resources",
                   __LINE__,
                   NULL,
                   "Region is from %d %d to %d %d", self->image_region.x0, self->image_region.y0,
                   self->image_region.x1, self->image_region.y1);

    gst_buffer_unmap(buf, &info);

    return new_buf;
}


static void correct_brightness (GstTcamautoexposure* self, GstBuffer* buf)
{
    image_buffer buffer = retrieve_image_region(self, buf);
    guint brightness = 0;

    if (self->color_format == BAYER)
    {

        gst_debug_log (gst_tcamautoexposure_debug_category,
                       GST_LEVEL_DEBUG,
                       "correct_brightness",
                       "",
                       __LINE__,
                       NULL,
                       "Calculating brightness");
        brightness = image_brightness_bayer(&buffer);
    }
    else
    {
        gst_debug_log (gst_tcamautoexposure_debug_category,
                       GST_LEVEL_DEBUG,
                       "correct_brightness",
                       "",
                       __LINE__,
                       NULL,
                       "Calculating brightness for gray");
        brightness = buffer_brightness_gray(&buffer);
    }
    /* assure we have the current values */
    retrieve_current_values (self);

    /* get distance from optimum */
    guint dist = calc_dist(self->brightness_reference, brightness);

    gst_debug_log (gst_tcamautoexposure_debug_category,
                   GST_LEVEL_DEBUG,
                   "correct_brightness",
                   "",
                   __LINE__,
                   NULL,
                   "Distance is %u", dist);

    if (dist < 98 || dist > 102)
    {
        gdouble new_gain = 0.0;

        if (self->auto_gain == TRUE)
        {
            /* set_gain */
            new_gain = calc_gain(self, dist);

            gst_debug_log (gst_tcamautoexposure_debug_category,
                           GST_LEVEL_DEBUG,
                           "correct_brightness",
                           "",
                           __LINE__,
                           NULL,
                           "Comparing gain: %f(new) < %f(old)", new_gain, self->gain.value);
            if (new_gain < self->gain.value)
            {
                set_gain(self, new_gain);
                return;
            }
        }

        if (self->auto_exposure == TRUE)
        {
            /* exposure */
            gdouble tmp_exposure = calc_exposure(self, dist, self->exposure.value);

            if (tmp_exposure != self->exposure.value)
            {
                set_exposure(self, tmp_exposure);
                return;
            }
        }

        if (self->auto_gain == TRUE)
        {
            if (self->auto_exposure == TRUE)
            {
                /* when exposure is in a sweet spot, or cannot be increased anymore */
                if (new_gain != self->gain.value && self->exposure.value >= self->exposure.max)
                {
                    set_gain(self, new_gain);
                    return;
                }

                // we can reduce gain, because we can increase exposure
                if ( self->gain.value > self->gain.min && self->exposure.value < self->exposure.max)
                {
                    /* increase exposure by 5% */
                    set_exposure(self, CLIP(((self->exposure.value * 105) / 100), self->exposure.min, self->exposure.max ));
                }

            }
            else
                set_gain(self,new_gain);
        }
    }

}


static gboolean find_camera_src (GstBaseTransform* trans)
{

    GstElement* e = GST_ELEMENT( gst_object_get_parent(GST_OBJECT(trans)));

    GList* l = GST_BIN(e)->children;

    while (1==1)
    {
        const char* name = g_type_name(gst_element_factory_get_element_type (gst_element_get_factory(l->data)));

        if (g_strcmp0(name, "GstTcamSrc") == 0)
        {

            GST_TCAMAUTOEXPOSURE(trans)->camera_src = l->data;
            break;
        }

        if (g_list_next(l) == NULL)
            break;

        l = g_list_next(l);
    }

    if (GST_TCAMAUTOEXPOSURE(trans)->camera_src == NULL)
    {
        gst_debug_log (gst_tcamautoexposure_debug_category,
                       GST_LEVEL_ERROR,
                       "",
                       "",
                       __LINE__,
                       NULL,
                       "Camera source not set!");
        return FALSE;
    }
    else
        return TRUE;
}


gboolean find_image_values (GstTcamautoexposure* self)
{
    GstPad* pad = GST_BASE_TRANSFORM_SINK_PAD(self);
    GstCaps* caps = gst_pad_get_current_caps(pad);
    GstStructure *structure = gst_caps_get_structure (caps, 0);

    g_return_val_if_fail (gst_structure_get_int (structure, "width", &self->image_size.width), FALSE);
    g_return_val_if_fail (gst_structure_get_int (structure, "height", &self->image_size.height), FALSE);

    gst_structure_get_fraction(structure, "framerate", &self->framerate_numerator, &self->framerate_denominator);

    return TRUE;
}


/*
  Entry point for actual transformation
*/
static GstFlowReturn gst_tcamautoexposure_transform_ip (GstBaseTransform* trans, GstBuffer* buf)
{
    GstTcamautoexposure* self = GST_TCAMAUTOEXPOSURE (trans);


    if (self->image_size.width == 0 || self->image_size.height == 0)
    {
        if (!find_image_values(self))
        {
            return GST_FLOW_ERROR;
        }
    }

    if (self->camera_src == NULL)
    {
        if (!find_camera_src(trans))
        {
            return GST_FLOW_ERROR;
        }
        else
        {
            init_camera_resources(self);
        }
    }

    if (self->auto_exposure == FALSE && self->auto_gain == FALSE)
    {
        return GST_FLOW_OK;
    }

    if (self->frame_counter > 3)
    {
        // validity checks
        GstMapInfo info;

        gst_buffer_map(buf, &info, GST_MAP_READ);

        guint* data = (guint*)info.data;
        guint length = info.size;

        gst_buffer_unmap(buf, &info);


        if (data == NULL || length == 0)
        {
            gst_debug_log (gst_tcamautoexposure_debug_category,
                           GST_LEVEL_ERROR,
                           "gst_tcamautoexposure",
                           "gst_tcamautoexposure",
                           __LINE__,
                           NULL,
                           "Buffer is not valid! Ignoring buffer and trying to continue...");
            return GST_FLOW_OK;
        }

        correct_brightness(self, buf);
        self->frame_counter = 0;
    }
    self->frame_counter++;

    return GST_FLOW_OK;
}


static gboolean plugin_init (GstPlugin * plugin)
{
    return gst_element_register (plugin,
                                 "tcamautoexposure",
                                 GST_RANK_NONE,
                                 GST_TYPE_TCAMAUTOEXPOSURE);
}

#ifndef VERSION
#define VERSION "0.0.1"
#endif
#ifndef PACKAGE
#define PACKAGE "tcamautoexposure"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "tcamautoexposure"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "https://github.com/TheImagingSource/tiscamera"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
                   GST_VERSION_MINOR,
                   tcamautoexposure,
                   "The Imaging Source auto exposure plugin",
                   plugin_init,
                   VERSION,
                   "Proprietary",
                   PACKAGE_NAME, GST_PACKAGE_ORIGIN)
