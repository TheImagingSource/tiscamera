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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tcam.h"
#include "base_types.h"
#include "tcamgstbase.h"
#include "tcamprop.h"

#include <cmath>
#include <stdlib.h>
#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include "gsttcamautoexposure.h"

GST_DEBUG_CATEGORY_STATIC (gst_tcamautoexposure_debug_category);
#define GST_CAT_DEFAULT gst_tcamautoexposure_debug_category

/* Constants */
static const gdouble K_CONTROL  = 0.5 / 255;
static const gdouble K_DEADBAND = 5.0;
static const gdouble K_GAIN     = 3.0;
static const gdouble K_ADJUST   = 4;
static const gdouble K_SMALL    = 1e-9;


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
    PROP_ROI_LEFT,
    PROP_ROI_TOP,
    PROP_ROI_WIDTH,
    PROP_ROI_HEIGHT,
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

static const gchar* gst_tcamautoexposure_get_property_type (TcamProp* self, gchar* name);

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
        case PROP_ROI_LEFT:
        {
            return "Exposure ROI Left";
        }
        case PROP_ROI_WIDTH:
        {
            return "Exposure ROI Width";
        }
        case PROP_ROI_TOP:
        {
            return "Exposure ROI Top";
        }
        case PROP_ROI_HEIGHT:
        {
            return "Exposure ROI Height";
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
    else if (g_strcmp0(str, "Exposure ROI Left") == 0)
    {
        return PROP_ROI_LEFT;
    }
    else if (g_strcmp0(str, "Exposure ROI Width") == 0)
    {
        return PROP_ROI_WIDTH;
    }
    else if (g_strcmp0(str, "Exposure ROI Top") == 0)
    {
        return PROP_ROI_TOP;
    }
    else if (g_strcmp0(str, "Exposure ROI Height") == 0)
    {
        return PROP_ROI_HEIGHT;
    }
    return 0;
}



static GSList* gst_tcamautoexposure_get_property_names (TcamProp* self)
{
    GSList* names = nullptr;

    names = g_slist_append(names,
        g_strdup(tcamautoexposure_property_id_to_string(PROP_AUTO_EXPOSURE)));
    names = g_slist_append(names,
        g_strdup(tcamautoexposure_property_id_to_string(PROP_AUTO_GAIN)));
    names = g_slist_append(names,
        g_strdup(tcamautoexposure_property_id_to_string(PROP_BRIGHTNESS_REFERENCE)));
    names = g_slist_append(names,
        g_strdup(tcamautoexposure_property_id_to_string(PROP_EXPOSURE_MAX)));
    names = g_slist_append(names,
        g_strdup(tcamautoexposure_property_id_to_string(PROP_GAIN_MAX)));
    names = g_slist_append(names,
        g_strdup(tcamautoexposure_property_id_to_string(PROP_ROI_LEFT)));
    names = g_slist_append(names,
        g_strdup(tcamautoexposure_property_id_to_string(PROP_ROI_WIDTH)));
    names = g_slist_append(names,
        g_strdup(tcamautoexposure_property_id_to_string(PROP_ROI_TOP)));
    names = g_slist_append(names,
        g_strdup(tcamautoexposure_property_id_to_string(PROP_ROI_HEIGHT)));

    return names;
}


static const gchar* gst_tcamautoexposure_get_property_type (TcamProp* self, gchar* name)
{
    if (name == nullptr)
    {
        return 0;
    }

    if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_AUTO_EXPOSURE)) == 0)
    {
        return strdup("boolean");
    }
    else if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_AUTO_GAIN)) == 0)
    {
        return strdup("boolean");
    }
    else if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_BRIGHTNESS_REFERENCE)) == 0)
    {
        return strdup("integer");
    }
    else if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_EXPOSURE_MAX)) == 0)
    {
        return strdup("integer");
    }
    else if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_GAIN_MAX)) == 0)
    {
        return strdup("double");
    }
    else if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_ROI_LEFT)) == 0)
    {
        return strdup("integer");
    }
    else if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_ROI_WIDTH)) == 0)
    {
        return strdup("integer");
    }
    else if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_ROI_TOP)) == 0)
    {
        return strdup("integer");
    }
    else if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_ROI_HEIGHT)) == 0)
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

    if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_AUTO_EXPOSURE)) == 0)
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
        if (flags)
        {
            g_value_init(flags, G_TYPE_INT);
            g_value_set_int(flags, 0);
        }
        if (type)
        {
            g_value_init(type, G_TYPE_STRING);
            g_value_set_string(type, gst_tcamautoexposure_get_property_type(prop, name));
        }
        if (category)
        {
            g_value_init(category, G_TYPE_STRING);
            g_value_set_string(category, "Exposure");
        }
        if (group)
        {
            g_value_init(group, G_TYPE_STRING);
            g_value_set_string(group, "Exposure");
        }
        return TRUE;
    }
    else if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_AUTO_GAIN)) == 0)
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
        if (flags)
        {
            g_value_init(flags, G_TYPE_INT);
            g_value_set_int(flags, 0);
        }
        if (type)
        {
            g_value_init(type, G_TYPE_STRING);
            g_value_set_string(type, gst_tcamautoexposure_get_property_type(prop, name));
        }
        if (category)
        {
            g_value_init(category, G_TYPE_STRING);
            g_value_set_string(category, "Exposure");
        }
        if (group)
        {
            g_value_init(group, G_TYPE_STRING);
            g_value_set_string(group, "Gain");
        }
        return TRUE;
    }
    else if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_BRIGHTNESS_REFERENCE)) == 0)
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
        if (flags)
        {
            g_value_init(flags, G_TYPE_INT);
            g_value_set_int(flags, 0);
        }
        if (type)
        {
            g_value_init(type, G_TYPE_STRING);
            g_value_set_string(type,gst_tcamautoexposure_get_property_type(prop, name));
        }
        if (category)
        {
            g_value_init(category, G_TYPE_STRING);
            g_value_set_string(category, "Exposure");
        }
        if (group)
        {
            g_value_init(group, G_TYPE_STRING);
            g_value_set_string(group, "Exposure");
        }
        return TRUE;
    }
    else if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_EXPOSURE_MAX)) == 0)
    {
        if (value)
        {
            g_value_init(value, G_TYPE_INT);
            g_value_set_int(value, self->exposure_max);
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
        if (flags)
        {
            g_value_init(flags, G_TYPE_INT);
            g_value_set_int(flags, 0);
        }
        if (type)
        {
            g_value_init(type, G_TYPE_STRING);
            g_value_set_string(type,gst_tcamautoexposure_get_property_type(prop, name));
        }
        if (category)
        {
            g_value_init(category, G_TYPE_STRING);
            g_value_set_string(category, "Exposure");
        }
        if (group)
        {
            g_value_init(group, G_TYPE_STRING);
            g_value_set_string(group, "Exposure");
        }
        return TRUE;
    }
    else if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_GAIN_MAX)) == 0)
    {
        if (value)
        {
            g_value_init(value, G_TYPE_DOUBLE);
            g_value_set_double(value, self->gain_max);
        }
        if (min)
        {
            g_value_init(min, G_TYPE_DOUBLE);
            g_value_set_double(min, self->gain.min);
        }
        if (max)
        {
            g_value_init(max, G_TYPE_DOUBLE);
            g_value_set_double(max, self->gain.max);
        }
        if (def)
        {
            g_value_init(def, G_TYPE_DOUBLE);
            g_value_set_double(def, 0);
        }
        if (step)
        {
            g_value_init(step, G_TYPE_DOUBLE);
            g_value_set_double(step, 1);
        }
        if (flags)
        {
            g_value_init(flags, G_TYPE_INT);
            g_value_set_int(flags, 0);
        }
        if (type)
        {
            g_value_init(type, G_TYPE_STRING);
            g_value_set_string(type,gst_tcamautoexposure_get_property_type(prop, name));
        }
        if (category)
        {
            g_value_init(category, G_TYPE_STRING);
            g_value_set_string(category, "Exposure");
        }
        if (group)
        {
            g_value_init(group, G_TYPE_STRING);
            g_value_set_string(group, "Gain");
        }

        return TRUE;
    }
    else if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_ROI_LEFT)) == 0)
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
            g_value_set_int(max, self->image_size.width - (SAMPLING_MIN_WIDTH+1));
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
        if (flags)
        {
            g_value_init(flags, G_TYPE_INT);
            g_value_set_int(flags, 0);
        }
        if (type)
        {
            g_value_init(type, G_TYPE_STRING);
            g_value_set_string(type,gst_tcamautoexposure_get_property_type(prop, name));
        }
        if (category)
        {
            g_value_init(category, G_TYPE_STRING);
            g_value_set_string(category, "Exposure");
        }
        if (group)
        {
            g_value_init(group, G_TYPE_STRING);
            g_value_set_string(group, "ROI");
        }
        return TRUE;
    }
    else if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_ROI_WIDTH)) == 0)
    {
        if (value)
        {
            g_value_init(value, G_TYPE_INT);
            g_value_set_int(value, self->image_region.x1);
        }
        if (min)
        {
            g_value_init(min, G_TYPE_INT);
            g_value_set_int(min, SAMPLING_MIN_WIDTH);
        }
        if (max)
        {
            g_value_init(max, G_TYPE_INT);
            g_value_set_int(max, self->image_size.width);
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
        if (flags)
        {
            g_value_init(flags, G_TYPE_INT);
            g_value_set_int(flags, 0);
        }
        if (type)
        {
            g_value_init(type, G_TYPE_STRING);
            g_value_set_string(type,gst_tcamautoexposure_get_property_type(prop, name));
        }
        if (category)
        {
            g_value_init(category, G_TYPE_STRING);
            g_value_set_string(category, "Exposure");
        }
        if (group)
        {
            g_value_init(group, G_TYPE_STRING);
            g_value_set_string(group, "ROI");
        }
        return TRUE;
    }
    else if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_ROI_TOP)) == 0)
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
            g_value_set_int(max, self->image_size.height - (SAMPLING_MIN_HEIGHT + 1));
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
        if (flags)
        {
            g_value_init(flags, G_TYPE_INT);
            g_value_set_int(flags, 0);
        }
        if (type)
        {
            g_value_init(type, G_TYPE_STRING);
            g_value_set_string(type,gst_tcamautoexposure_get_property_type(prop, name));
        }
        if (category)
        {
            g_value_init(category, G_TYPE_STRING);
            g_value_set_string(category, "Exposure");
        }
        if (group)
        {
            g_value_init(group, G_TYPE_STRING);
            g_value_set_string(group, "ROI");
        }
        return TRUE;
    }
    else if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_ROI_HEIGHT)) == 0)
    {
        if (value)
        {
            g_value_init(value, G_TYPE_INT);
            g_value_set_int(value, self->image_region.y1);
        }
        if (min)
        {
            g_value_init(min, G_TYPE_INT);
            g_value_set_int(min, SAMPLING_MIN_HEIGHT);
        }
        if (max)
        {
            g_value_init(max, G_TYPE_INT);
            g_value_set_int(max, self->image_size.height);
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
        if (flags)
        {
            g_value_init(flags, G_TYPE_INT);
            g_value_set_int(flags, 0);
        }
        if (type)
        {
            g_value_init(type, G_TYPE_STRING);
            g_value_set_string(type,gst_tcamautoexposure_get_property_type(prop, name));
        }
        if (category)
        {
            g_value_init(category, G_TYPE_STRING);
            g_value_set_string(category, "Exposure");
        }
        if (group)
        {
            g_value_init(group, G_TYPE_STRING);
            g_value_set_string(group, "ROI");
        }
        return TRUE;
    }

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
                                     g_param_spec_int ("exposure-max",
                                                       "Exposure Maximum",
                                                       "Maximum value exposure can take",
                                                       0, G_MAXINT, 0,
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
                                    PROP_ROI_LEFT,
                                    g_param_spec_int("left",
                                                      "Left boundary of ROI",
                                                      "Left boundary of the region of interest",
                                                      0, G_MAXINT, 0,
                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT)) ;

    g_object_class_install_property(gobject_class,
                                    PROP_ROI_TOP,
                                    g_param_spec_int("top",
                                                      "Top boundary of ROI",
                                                      "Top boundary of the region of interest",
                                                      0, G_MAXINT, 0,
                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT)) ;

    g_object_class_install_property(gobject_class,
                                    PROP_ROI_WIDTH,
                                    g_param_spec_int("width",
                                                      "Width of ROI starting at 'left'",
                                                      "Width of the region of interest",
                                                      0, G_MAXINT, 0,
                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT)) ;

    g_object_class_install_property(gobject_class,
                                    PROP_ROI_HEIGHT,
                                    g_param_spec_int("height",
                                                      "Lower, right boundary starting at 'top'",
                                                      "Height of the region of interest",
                                                      0, G_MAXINT, 0,
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
    self->gain_is_double = FALSE;
    self->exposure_max = 0;
    self->gain_max = 0.0;
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
            tcamautoexposure->exposure_max = g_value_get_int(value);
            if (tcamautoexposure->exposure_max == 0.0)
            {
                tcamautoexposure->exposure = tcamautoexposure->default_exposure_values;
            }
            break;
        case PROP_GAIN_MAX:
            GST_DEBUG("Setting gain max to : %f", g_value_get_double(value));

            tcamautoexposure->gain_max = g_value_get_double(value);
            if (tcamautoexposure->gain_max == 0.0)
            {
                tcamautoexposure->gain = tcamautoexposure->default_gain_values;
            }
            break;
        case PROP_BRIGHTNESS_REFERENCE:
            tcamautoexposure->brightness_reference = g_value_get_int(value);
            break;
        case PROP_ROI_LEFT:
            tcamautoexposure->image_region.x0 = g_value_get_int(value);
            break;
        case PROP_ROI_TOP:
            tcamautoexposure->image_region.y0 = g_value_get_int(value);
            break;
        case PROP_ROI_WIDTH:
            tcamautoexposure->image_region.x1 = g_value_get_int(value);
            break;
        case PROP_ROI_HEIGHT:
            tcamautoexposure->image_region.y1 = g_value_get_int(value);
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
            g_value_set_int(value, tcamautoexposure->exposure_max);
            break;
        case PROP_GAIN_MAX:
            g_value_set_double(value, tcamautoexposure->gain_max);
            break;
        case PROP_BRIGHTNESS_REFERENCE:
            g_value_set_int(value, tcamautoexposure->brightness_reference);
            break;
        case PROP_ROI_LEFT:
            g_value_set_int(value, tcamautoexposure->image_region.x0);
            break;
        case PROP_ROI_TOP:
            g_value_set_int(value, tcamautoexposure->image_region.y0);
            break;
        case PROP_ROI_WIDTH:
            g_value_set_int(value, tcamautoexposure->image_region.x1);
            break;
        case PROP_ROI_HEIGHT:
            g_value_set_int(value, tcamautoexposure->image_region.y1);
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
        GST_ERROR("Exposure could not be found!");
    }
    else
    {
        p = prop->get_struct();
        self->exposure.min = p.value.i.min;
        // do not set exposure.max
        // we default to 0
        // if 0 -> use max setting according to framerate
        // if max is set by user we use that
        self->exposure.max = p.value.i.max;
        self->exposure.value = p.value.i.value;

        self->default_exposure_values.max = 1000000 / (self->framerate_numerator / self->framerate_denominator);
    }

    p = {0};
    prop = dev->get_property(TCAM_PROPERTY_GAIN);

    if (prop == nullptr)
    {
        GST_ERROR("Gain could not be found!");
    }
    else
    {
        p = prop->get_struct();
        if (p.type == TCAM_PROPERTY_TYPE_INTEGER)
        {
            self->gain.min = p.value.i.min;
            self->gain.max = p.value.i.max;
            self->gain.value = p.value.i.value;
        }
        else
        {
            self->gain.min = p.value.d.min;
            self->gain.max = p.value.d.max;
            self->gain.value = p.value.d.value;
        }
        self->gain_max = self->gain.max;
    }

    if (self->exposure_max == 0)
    {
        self->exposure_max = self->default_exposure_values.max;
    }
    GST_INFO("Exposure boundaries are %f %f", self->exposure.min, self->exposure_max);

    GST_INFO("Gain boundaries are %f %f", self->gain.min, self->gain.max);
}


static void set_exposure (GstTcamautoexposure* self, gdouble exposure)
{
    GST_INFO("Setting exposure to %f", exposure);

    tcam::CaptureDevice* dev;
    g_object_get(G_OBJECT(self->camera_src), "camera", &dev, NULL);

    dev->set_property(TCAM_PROPERTY_EXPOSURE, (int64_t)exposure);
}


static void set_gain (GstTcamautoexposure* self, gdouble gain)
{
    GST_INFO("Setting gain to %f", gain);

    tcam::CaptureDevice* dev;
    g_object_get(G_OBJECT(self->camera_src), "camera", &dev, NULL);

    if (!self->gain_is_double)
    {
        dev->set_property(TCAM_PROPERTY_GAIN, (int64_t)std::lround(gain));
    }
    else
    {
        dev->set_property(TCAM_PROPERTY_GAIN, gain);
    }
}


static gdouble calc_offset (unsigned int reference_val, unsigned int brightness_val)
{
    const gdouble r = (gdouble) reference_val;
    const gdouble y = (gdouble) brightness_val;

    return r - y;
}


static gdouble modify_gain (GstTcamautoexposure* self, gdouble diff)
{
    if (fabs(diff) < K_SMALL)
    {
        return 0.0;
    }

    if (self->auto_gain)
    {
        gdouble K_GAIN_FAST = 30;
        gdouble K_GAIN_SLOW = 0.1;

        gdouble g_ref;

        if (diff <= 40.0)
        {
            g_ref = self->gain.value + K_GAIN_FAST * diff;
        }
        else
        {
            g_ref = self->gain.value + K_GAIN_SLOW * diff;
        }

        gdouble new_gain = fmax(fmin(g_ref, self->gain_max), self->gain.min);
        double percentage_new = (new_gain / self->gain_max * 100) - (self->gain.value / self->gain_max * 100);

        if (fabs(self->gain.value - new_gain) > K_SMALL)
        {
            GST_DEBUG("fabs(self->gain.value - new_gain) > K_SMALL == %f - %f =%f (g_ref %f diff %f)",
                      self->gain.value, new_gain, fabs(self->gain.value - new_gain), g_ref, diff);

            double percentage = self->gain_max / 100;
            double setter = new_gain;
            GST_DEBUG("Comparing percentage_new %f > percentage %f", percentage_new, percentage);
            if (fabs(percentage_new) > percentage)
            {
                if (percentage_new > 0.0)
                {
                    setter = self->gain.value + percentage;
                }
                else
                {
                    setter = self->gain.value - percentage;
                }
            }
            set_gain(self, setter);
        }
        GST_DEBUG("NEW GAIN: g_ref %f - new_gain %f / K_GAIN %f = %f",
                  g_ref, new_gain, K_GAIN, (g_ref - new_gain) / K_GAIN);

        return (g_ref - new_gain) / K_GAIN;
    }
    else
    {
        return diff;
    }
}


static double modify_exposure (GstTcamautoexposure* self, gdouble diff)
{
    if (fabs(diff) < K_SMALL)
    {
        return 0;
    }

    if (self->auto_exposure)
    {
        const double e_ref = self->exposure.value * pow(2, diff);
        const double new_exposure = fmax(fmin(e_ref, self->exposure_max), self->exposure.min);

        if (fabs(self->exposure.value - new_exposure) > K_SMALL)
        {
            set_exposure(self, new_exposure);
        }
        GST_DEBUG("Returning (e_ref - new_exposure = diff) %f - %f = %f",
                  e_ref, new_exposure, (e_ref - new_exposure));

        return e_ref - new_exposure;
    }
    else
    {
        return diff;
    }
}


void retrieve_current_values (GstTcamautoexposure* self)
{
    tcam::CaptureDevice* dev = NULL;
    g_object_get(G_OBJECT(self->camera_src), "camera", &dev, NULL);

    struct tcam_device_property p = {};

    if (dev == NULL)
    {
        GST_ERROR("Tcam did not return a valid device");
    }

    tcam::Property* prop =  dev->get_property(TCAM_PROPERTY_EXPOSURE);

    if (prop == nullptr)
    {
        GST_ERROR("Tcam did not return exposure");
    }
    else
    {
        p = prop->get_struct();
        GST_DEBUG("Current exposure is %ld", p.value.i.value);
        self->exposure.value = p.value.i.value;
    }

    prop =  dev->get_property(TCAM_PROPERTY_GAIN);

    if (prop == nullptr)
    {
        GST_ERROR("Tcam did not return gain");
    }
    else
    {
        p = prop->get_struct();

        if (prop->get_type() == TCAM_PROPERTY_TYPE_INTEGER)
        {
            self->gain.value = p.value.i.value;
            if (self->gain.value < self->gain.min)
            {
                self->gain.value = self->gain.min;
            }
            GST_DEBUG("Current gain is %f", self->gain.value);

        }
        else
        {
            GST_DEBUG("Current gain is %f", p.value.d.value);
            self->gain.value = p.value.d.value;
        }
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

    if (self->image_region.x1 == 0)
    {
        self->image_region.x1 = self->image_size.width;
    }
    else if (self->image_region.x1 < SAMPLING_MIN_WIDTH)
    {
        self->image_region.x1 = SAMPLING_MIN_WIDTH;
    }

    if (self->image_region.y1 == 0)
    {
        self->image_region.y1 = self->image_size.height;
    }
    else if (self->image_region.y1 < SAMPLING_MIN_HEIGHT)
    {
        self->image_region.y1 = SAMPLING_MIN_HEIGHT;
    }

    if (self->image_region.x0 >= (self->image_size.width -
				  self->image_region.x1))
    {
        self->image_region.x0 = self->image_size.width -
	   ( self->image_region.x1);
    }

    if (self->image_region.y0 >= (self->image_size.height -
				  self->image_region.y1))
    {
        self->image_region.y0 = self->image_size.height -
	   ( self->image_region.y1);
    }

    const int bytes_per_pixel = 1;

    image_buffer new_buf;

    new_buf.rowstride = self->image_size.width * bytes_per_pixel;

    new_buf.image = info.data + (self->image_region.x0 * bytes_per_pixel
				 + self->image_region.y0 * new_buf.rowstride * bytes_per_pixel);

    new_buf.width = self->image_region.x1;
    new_buf.height = self->image_region.y1;

    new_buf.pattern = calculate_pattern_from_offset(self);

    GST_INFO("Region is from %d %d to %d %d",
             self->image_region.x0, self->image_region.y0,
             self->image_region.x0 + self->image_region.x1, self->image_region.y0 + self->image_region.y1);

    gst_buffer_unmap(buf, &info);

    return new_buf;
}


static void correct_brightness (GstTcamautoexposure* self, GstBuffer* buf)
{
    image_buffer buffer = retrieve_image_region(self, buf);
    guint brightness = 0;

    if (self->color_format == BAYER)
    {
        GST_DEBUG("Calculating brightness");
        brightness = image_brightness_bayer(&buffer);
    }
    else
    {
        GST_DEBUG("Calculating brightness for gray");
        brightness = buffer_brightness_gray(&buffer);
    }
    /* assure we have the current values */
    retrieve_current_values (self);

    /* get offset and control difference from reference */
    const gdouble offset = calc_offset(self->brightness_reference, brightness);

    GST_INFO("offset = %f, gain = %f, exposure = %f", offset, self->gain.value, self->exposure.value);

    /* Check if outside deadband */
    if (fabs(offset) > K_DEADBAND)
    {
        /* Compute control change from offset */
        const gdouble change = K_CONTROL * offset;

        /* Check if too bright */
        if (change < 0.0)
        {
            GST_DEBUG("exposure");
            /* Reduce gain first, then exposure */
            modify_exposure(self, modify_gain(self, change));
        }
        else
        {
            GST_DEBUG("gain");
            /* Increase exposure first, then gain */
            modify_gain(self, modify_exposure(self, change));
        }
    }
    else
    {
        /* Try swapping gain for exposure */
        if (self->gain.value > self->gain.min && self->exposure.value < self->exposure_max)
        {
            GST_DEBUG("reducing gain");
            modify_exposure(self, -modify_gain(self, -K_ADJUST));
        }
    }
}


gboolean find_image_values (GstTcamautoexposure* self)
{
    GstPad* pad = GST_BASE_TRANSFORM_SINK_PAD(self);
    GstCaps* caps = gst_pad_get_current_caps(pad);
    GstStructure *structure = gst_caps_get_structure (caps, 0);

    g_return_val_if_fail (gst_structure_get_int (structure, "width", &self->image_size.width), FALSE);
    g_return_val_if_fail (gst_structure_get_int (structure, "height", &self->image_size.height), FALSE);

    gst_structure_get_fraction(structure, "framerate",
                               &self->framerate_numerator, &self->framerate_denominator);

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
        self->camera_src = tcam_gst_find_camera_src(GST_ELEMENT(self));
        if (self->camera_src == nullptr)
        {
            GST_ERROR("Could not find source element");
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
        find_image_values(self);
        // validity checks
        GstMapInfo info;

        gst_buffer_map(buf, &info, GST_MAP_READ);

        guint* data = (guint*)info.data;
        guint length = info.size;

        gst_buffer_unmap(buf, &info);


        if (data == NULL || length == 0)
        {
            GST_WARNING("Buffer is not valid! Ignoring buffer and trying to continue...");
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
