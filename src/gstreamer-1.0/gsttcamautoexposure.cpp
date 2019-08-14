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
#include <gst/video/video.h>
#include <gst/base/gstbasetransform.h>
#include "gsttcamautoexposure.h"


GST_DEBUG_CATEGORY_STATIC (gst_tcamautoexposure_debug_category);
#define GST_CAT_DEFAULT gst_tcamautoexposure_debug_category

// The algorithm works with integers
// floating point gain values will be multiplied
// to allow handling of this
static const gdouble GAIN_FLOAT_MULTIPLIER = 1000;

/* prototypes */

static void set_exposure (GstTcamautoexposure* self, gdouble exposure);

static void set_gain (GstTcamautoexposure* self, gdouble gain);

static void set_iris (GstTcamautoexposure* self, int iris);

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
static gboolean gst_tcamautoexposure_set_caps (GstBaseTransform * trans,
                                               GstCaps * incaps, GstCaps * outcaps);

static void init_camera_resources (GstTcamautoexposure* self);


enum
{
    PROP_0,
    PROP_AUTO_EXPOSURE,
    PROP_AUTO_GAIN,
    PROP_AUTO_IRIS,
    PROP_CAMERA,
    PROP_BRIGHTNESS_REFERENCE,
    PROP_EXPOSURE_MIN,
    PROP_EXPOSURE_MAX,
    PROP_GAIN_MIN,
    PROP_GAIN_MAX,
    PROP_IRIS_MIN,
    PROP_IRIS_MAX,
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

static GSList* gst_tcamautoexposure_get_property_names(TcamProp* self);

static gchar* gst_tcamautoexposure_get_property_type (TcamProp* self,
                                                      const gchar* name);

static gboolean gst_tcamautoexposure_get_tcam_property (TcamProp* self,
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

static gboolean gst_tcamautoexposure_set_tcam_property (TcamProp* self,
                                                        const gchar* name,
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
                                                gst_tcamautoexposure_prop_init))


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
        case PROP_AUTO_IRIS:
        {
            return "Iris Auto";
        }
        case PROP_BRIGHTNESS_REFERENCE:
        {
            return "Brightness Reference";
        }
        case PROP_EXPOSURE_MIN:
        {
            return "Exposure Min";
        }
        case PROP_EXPOSURE_MAX:
        {
            return "Exposure Max";
        }
        case PROP_GAIN_MIN:
        {
            return "Gain Min";
        }
        case PROP_GAIN_MAX:
        {
            return "Gain Max";
        }
        case PROP_IRIS_MIN:
        {
            return "Iris Min";
        }
        case PROP_IRIS_MAX:
        {
            return "Iris Max";
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
    else if (g_strcmp0(str, "Iris Auto") == 0)
    {
        return PROP_AUTO_IRIS;
    }
    else if (g_strcmp0(str, "Brightness Reference") == 0)
    {
        return PROP_BRIGHTNESS_REFERENCE;
    }
    else if (g_strcmp0(str, "Exposure Min") == 0)
    {
        return PROP_EXPOSURE_MIN;
    }
    else if (g_strcmp0(str, "Exposure Max") == 0)
    {
        return PROP_EXPOSURE_MAX;
    }
    else if (g_strcmp0(str, "Gain Min") == 0)
    {
        return PROP_GAIN_MIN;
    }
    else if (g_strcmp0(str, "Gain Max") == 0)
    {
        return PROP_GAIN_MAX;
    }
    else if (g_strcmp0(str, "Iris Min") == 0)
    {
        return PROP_IRIS_MIN;
    }
    else if (g_strcmp0(str, "Iris Max") == 0)
    {
        return PROP_IRIS_MAX;
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
    GstTcamautoexposure* element = GST_TCAMAUTOEXPOSURE(self);

    GSList* names = nullptr;

    names = g_slist_append(names,
                           g_strdup(tcamautoexposure_property_id_to_string(PROP_AUTO_EXPOSURE)));
    names = g_slist_append(names,
                           g_strdup(tcamautoexposure_property_id_to_string(PROP_AUTO_GAIN)));
    names = g_slist_append(names,
                           g_strdup(tcamautoexposure_property_id_to_string(PROP_BRIGHTNESS_REFERENCE)));
    names = g_slist_append(names,
                           g_strdup(tcamautoexposure_property_id_to_string(PROP_EXPOSURE_MIN)));
    names = g_slist_append(names,
                           g_strdup(tcamautoexposure_property_id_to_string(PROP_EXPOSURE_MAX)));
    names = g_slist_append(names,
                           g_strdup(tcamautoexposure_property_id_to_string(PROP_GAIN_MIN)));
    names = g_slist_append(names,
                           g_strdup(tcamautoexposure_property_id_to_string(PROP_GAIN_MAX)));
    if (element->has_iris)
    {
        names = g_slist_append(names,
                               g_strdup(tcamautoexposure_property_id_to_string(PROP_AUTO_IRIS)));
        names = g_slist_append(names,
                               g_strdup(tcamautoexposure_property_id_to_string(PROP_IRIS_MIN)));
        names = g_slist_append(names,
                               g_strdup(tcamautoexposure_property_id_to_string(PROP_IRIS_MAX)));
    }
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


static gchar* gst_tcamautoexposure_get_property_type (TcamProp* self __attribute__((unused)),
                                                      const gchar* name)
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
    else if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_AUTO_IRIS)) == 0)
    {
        return strdup("boolean");
    }
    else if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_BRIGHTNESS_REFERENCE)) == 0)
    {
        return strdup("integer");
    }
    else if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_EXPOSURE_MIN)) == 0)
    {
        return strdup("integer");
    }
    else if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_EXPOSURE_MAX)) == 0)
    {
        return strdup("integer");
    }
    else if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_GAIN_MIN)) == 0)
    {
        return strdup("double");
    }
    else if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_GAIN_MAX)) == 0)
    {
        return strdup("double");
    }
    else if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_IRIS_MIN)) == 0)
    {
        return strdup("integer");
    }
    else if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_IRIS_MAX)) == 0)
    {
        return strdup("integer");
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
    if (name == nullptr)
    {
        return FALSE;
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
    else if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_AUTO_IRIS)) == 0)
    {
        if (!self->has_iris)
        {
            return FALSE;
        }

        if (value)
        {
            g_value_init(value, G_TYPE_BOOLEAN);
            g_value_set_boolean(value, self->auto_iris);
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
            g_value_set_string(category, "Lens");
        }
        if (group)
        {
            g_value_init(group, G_TYPE_STRING);
            g_value_set_string(group, "Iris");
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
    else if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_EXPOSURE_MIN)) == 0)
    {
        if (value)
        {
            g_value_init(value, G_TYPE_INT);
            g_value_set_int(value, self->exposure_min);
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
            g_value_set_int(def, self->exposure.min);
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
            g_value_set_int(def, self->default_exposure_values.max);
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
    else if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_GAIN_MIN)) == 0)
    {
        if (value)
        {
            g_value_init(value, G_TYPE_DOUBLE);
            if (self->gain_is_double)
            {
                g_value_set_double(value, self->gain_min / GAIN_FLOAT_MULTIPLIER);
            }
            else
            {
                g_value_set_double(value, self->gain_min);
            }
        }
        if (min)
        {
            g_value_init(min, G_TYPE_DOUBLE);
            if (self->gain_is_double)
            {
                g_value_set_double(min, self->gain.min / GAIN_FLOAT_MULTIPLIER);
            }
            else
            {
                g_value_set_double(min, self->gain.min);
            }
        }
        if (max)
        {
            g_value_init(max, G_TYPE_DOUBLE);
            if (self->gain_is_double)
            {
                g_value_set_double(max, self->gain.max / GAIN_FLOAT_MULTIPLIER);
            }
            else
            {
                g_value_set_double(max, self->gain.max);
            }
        }
        if (def)
        {
            g_value_init(def, G_TYPE_DOUBLE);
            if (self->gain_is_double)
            {
                g_value_set_double(def, self->gain.min / GAIN_FLOAT_MULTIPLIER);
            }
            else
            {
                g_value_set_double(def, self->gain.min);
            }
        }
        if (step)
        {
            g_value_init(step, G_TYPE_DOUBLE);
            if (self->gain_is_double)
            {
                g_value_set_double(step, 0.1);
            }
            else
            {
                g_value_set_double(step, 1);
            }
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
    else if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_GAIN_MAX)) == 0)
    {
        if (value)
        {
            g_value_init(value, G_TYPE_DOUBLE);
            if (self->gain_is_double)
            {
                g_value_set_double(value,
                                   self->gain_max / GAIN_FLOAT_MULTIPLIER);
            }
            else
            {
                g_value_set_double(value, self->gain_max);
            }
        }
        if (min)
        {
            g_value_init(min, G_TYPE_DOUBLE);
            if (self->gain_is_double)
            {
                g_value_set_double(min,
                                   self->gain.min / GAIN_FLOAT_MULTIPLIER);
            }
            else
            {
                g_value_set_double(min, self->gain.min);
            }
        }
        if (max)
        {
            g_value_init(max, G_TYPE_DOUBLE);
            if (self->gain_is_double)
            {
                g_value_set_double(max,
                                   self->gain.max / GAIN_FLOAT_MULTIPLIER);
            }
            else
            {
                g_value_set_double(max, self->gain.max);
            }
        }
        if (def)
        {
            g_value_init(def, G_TYPE_DOUBLE);
            if (self->gain_is_double)
            {
                g_value_set_double(def,
                                   self->gain.max / GAIN_FLOAT_MULTIPLIER);
            }
            else
            {
                g_value_set_double(def, self->gain.max);
            }
        }
        if (step)
        {
            g_value_init(step, G_TYPE_DOUBLE);
            if (self->gain_is_double)
            {
                g_value_set_double(step, 0.1);
            }
            else
            {
                g_value_set_double(step, 1);
            }
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
    else if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_IRIS_MIN)) == 0)
    {
        if (!self->has_iris)
        {
            return FALSE;
        }

        if (value)
        {
            g_value_init(value, G_TYPE_INT);
            g_value_set_int(value, self->iris_min);
        }
        if (min)
        {
            g_value_init(min, G_TYPE_INT);
            g_value_set_int(min, self->iris.min);
        }
        if (max)
        {
            g_value_init(max, G_TYPE_INT);
            g_value_set_int(max, self->iris.max);
        }
        if (def)
        {
            g_value_init(def, G_TYPE_INT);
            g_value_set_int(def, self->iris.min);
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
            g_value_set_string(category, "Lens");
        }
        if (group)
        {
            g_value_init(group, G_TYPE_STRING);
            g_value_set_string(group, "Iris");
        }
        return TRUE;
    }
    else if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_IRIS_MAX)) == 0)
    {
        if (!self->has_iris)
        {
            return FALSE;
        }

        if (value)
        {
            g_value_init(value, G_TYPE_INT);
            g_value_set_int(value, self->iris_max);
        }
        if (min)
        {
            g_value_init(min, G_TYPE_INT);
            g_value_set_int(min, self->iris.min);
        }
        if (max)
        {
            g_value_init(max, G_TYPE_INT);
            g_value_set_int(max, self->iris.max);
        }
        if (def)
        {
            g_value_init(def, G_TYPE_INT);
            g_value_set_int(def, self->iris.min);
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
            g_value_set_string(category, "Lens");
        }
        if (group)
        {
            g_value_init(group, G_TYPE_STRING);
            g_value_set_string(group, "Iris");
        }
        return TRUE;
    }
    else if (g_strcmp0(name, tcamautoexposure_property_id_to_string(PROP_ROI_LEFT)) == 0)
    {
        if (value)
        {
            g_value_init(value, G_TYPE_INT);
            g_value_set_int(value, roi_left(self->roi));
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
            g_value_set_int(value, roi_width(self->roi));
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
            g_value_set_int(def, self->image_size.width);
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
            g_value_set_string(type,
                               gst_tcamautoexposure_get_property_type(prop, name));
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
            g_value_set_int(value, roi_top(self->roi));
        }
        if (min)
        {
            g_value_init(min, G_TYPE_INT);
            g_value_set_int(min, 0);
        }
        if (max)
        {
            g_value_init(max, G_TYPE_INT);
            g_value_set_int(max,
                            self->image_size.height - (SAMPLING_MIN_HEIGHT + 1));
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
            g_value_set_string(type,
                               gst_tcamautoexposure_get_property_type(prop, name));
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
            g_value_set_int(value, roi_height(self->roi));
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
            g_value_set_int(def, self->image_size.height);
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
            g_value_set_string(type,
                               gst_tcamautoexposure_get_property_type(prop, name));
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
                                                        const gchar* name,
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

static GSList* gst_tcamautoexposure_get_tcam_menu_entries (TcamProp* self __attribute__((unused)),
                                                           const gchar* name __attribute__((unused)))
{
    GSList* ret = nullptr;

    return ret;
}


static GSList* gst_tcamautoexposure_get_device_serials (TcamProp* self __attribute__((unused)))
{
    return nullptr;
}


static gboolean gst_tcamautoexposure_get_device_info (TcamProp* self __attribute__((unused)),
                                                      const char* serial __attribute__((unused)),
                                                      char** name __attribute__((unused)),
                                                      char** identifier __attribute__((unused)),
                                                      char** connection_type __attribute__((unused)))
{
    return FALSE;
}
/* pad templates */

#define VIDEO_CAPS \
    GST_VIDEO_CAPS_MAKE("{GRAY8, GRAY16_LE}") ";" \
        "video/x-bayer,format={rggb,bggr,gbrg,grbg}," \
        "width=" GST_VIDEO_SIZE_RANGE ",height=" GST_VIDEO_SIZE_RANGE \
        ",framerate=" GST_VIDEO_FPS_RANGE


/* class initialization */

static void gst_tcamautoexposure_class_init (GstTcamautoexposureClass* klass)
{
    GObjectClass* gobject_class = G_OBJECT_CLASS (klass);
    GstBaseTransformClass* base_transform_class = GST_BASE_TRANSFORM_CLASS (klass);

    gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
        gst_pad_template_new ("src", GST_PAD_SRC, GST_PAD_ALWAYS,
            gst_caps_from_string (VIDEO_CAPS)));
    gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
        gst_pad_template_new ("sink", GST_PAD_SINK, GST_PAD_ALWAYS,
            gst_caps_from_string (VIDEO_CAPS)));

    gst_element_class_set_details_simple (GST_ELEMENT_CLASS(klass),
                                          "The Imaging Source Auto Exposure Element",
                                          "Generic",
                                          "Adjusts the image brightness by setting camera properties.",
                                          "The Imaging Source Europe GmbH <support@theimagingsource.com>");

    GST_DEBUG_CATEGORY_INIT(gst_tcamautoexposure_debug_category, "tcamautoexposure", 0, "tcam autoexposure");


    gobject_class->set_property = gst_tcamautoexposure_set_property;
    gobject_class->get_property = gst_tcamautoexposure_get_property;
    gobject_class->finalize = gst_tcamautoexposure_finalize;
    base_transform_class->transform_ip = gst_tcamautoexposure_transform_ip;
    base_transform_class->set_caps = GST_DEBUG_FUNCPTR (gst_tcamautoexposure_set_caps);

    g_object_class_install_property (gobject_class,
                                     PROP_AUTO_EXPOSURE,
                                     g_param_spec_boolean ("auto-exposure",
                                                           "Auto Exposure",
                                                           "Automatically adjust exposure",
                                                           TRUE,
                                                           static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));
    g_object_class_install_property (gobject_class,
                                     PROP_AUTO_GAIN,
                                     g_param_spec_boolean ("auto-gain",
                                                           "Auto Gain",
                                                           "Automatically adjust gain",
                                                           TRUE,
                                                           static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));
    g_object_class_install_property (gobject_class,
                                     PROP_AUTO_GAIN,
                                     g_param_spec_boolean ("auto-iris",
                                                           "Auto Iris",
                                                           "Automatically adjust the iris, if camera allows adjustments",
                                                           TRUE,
                                                           static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));
    g_object_class_install_property (gobject_class,
                                     PROP_EXPOSURE_MAX,
                                     g_param_spec_int ("exposure-max",
                                                       "Exposure Maximum",
                                                       "Maximum value exposure can take",
                                                       0, G_MAXINT, G_MAXINT,
                                                       static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));
    g_object_class_install_property (gobject_class,
                                     PROP_EXPOSURE_MIN,
                                     g_param_spec_int ("exposure-min",
                                                       "Exposure Minimum",
                                                       "Minimum value exposure can take",
                                                       0, G_MAXINT, 0,
                                                       static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));

    g_object_class_install_property (gobject_class,
                                     PROP_GAIN_MAX,
                                     g_param_spec_double ("gain-max",
                                                          "Gain Maximum",
                                                          "Maximum value gain can take",
                                                          0.0, G_MAXDOUBLE, G_MAXDOUBLE,
                                                          static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));
    g_object_class_install_property (gobject_class,
                                     PROP_GAIN_MIN,
                                     g_param_spec_double ("gain-min",
                                                          "Gain Minimum",
                                                          "Minimum value gain can take",
                                                          0.0, G_MAXDOUBLE, 0.0,
                                                          static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));

    g_object_class_install_property (gobject_class,
                                     PROP_IRIS_MAX,
                                     g_param_spec_int ("iris-max",
                                                       "Iris Maximum",
                                                       "Maximum value the iris can take",
                                                       0, G_MAXINT, G_MAXINT,
                                                       static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));
    g_object_class_install_property (gobject_class,
                                     PROP_IRIS_MIN,
                                     g_param_spec_int ("iris-min",
                                                       "Iris Minimum",
                                                       "Minimum value the iris can take",
                                                       0, G_MAXINT, 0,
                                                       static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));

    g_object_class_install_property (gobject_class,
                                     PROP_BRIGHTNESS_REFERENCE,
                                     g_param_spec_int("brightness-reference",
                                                      "Brightness Reference",
                                                      "Ideal average brightness of buffer",
                                                      0, 255, 128,
                                                      static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));
    g_object_class_install_property(gobject_class,
                                    PROP_ROI_LEFT,
                                    g_param_spec_int("left",
                                                      "Left boundary of ROI",
                                                      "Left boundary of the region of interest",
                                                      0, G_MAXINT, 0,
                                                      static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));

    g_object_class_install_property(gobject_class,
                                    PROP_ROI_TOP,
                                    g_param_spec_int("top",
                                                      "Top boundary of ROI",
                                                      "Top boundary of the region of interest",
                                                      0, G_MAXINT, 0,
                                                      static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));

    g_object_class_install_property(gobject_class,
                                    PROP_ROI_WIDTH,
                                    g_param_spec_int("width",
                                                      "Width of ROI starting at 'left'",
                                                      "Width of the region of interest",
                                                      0, G_MAXINT, 0,
                                                      static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));

    g_object_class_install_property(gobject_class,
                                    PROP_ROI_HEIGHT,
                                    g_param_spec_int("height",
                                                      "Lower, right boundary starting at 'top'",
                                                      "Height of the region of interest",
                                                      0, G_MAXINT, 0,
                                                      static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));

    g_object_class_install_property (gobject_class,
                                     PROP_CAMERA,
                                     g_param_spec_object ("camera",
                                                          "camera gst element",
                                                          "Gstreamer element that shall be manipulated",
                                                          GST_TYPE_ELEMENT,
                                                          static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));
}

static void gst_tcamautoexposure_init (GstTcamautoexposure *self)
{
    self->auto_exposure = TRUE;
    self->auto_gain = TRUE;
    self->auto_iris = TRUE;
    self->gain_is_double = FALSE;
    self->exposure_min = 0;
    self->exposure_max = G_MAXINT;
    self->gain_min = 0.0;
    self->gain_max = G_MAXDOUBLE;
    self->frame_counter = 0;
    self->has_iris = FALSE;
    self->camera_src = NULL;
    self->module_is_disabled = FALSE;

    tcam_image_size s = {SAMPLING_MIN_WIDTH, SAMPLING_MIN_HEIGHT};

    self->roi = create_roi(&s, &s);

    // explicitly set properties
    roi_set_preset(self->roi, ROI_PRESET_FULL_SENSOR);

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
        {
            tcamautoexposure->auto_exposure = g_value_get_boolean(value);
            break;
        }
        case PROP_AUTO_GAIN:
        {
            tcamautoexposure->auto_gain = g_value_get_boolean(value);
            break;
        }
        case PROP_AUTO_IRIS:
        {
            tcamautoexposure->auto_iris = g_value_get_boolean(value);
            break;
        }
        case PROP_CAMERA:
        {
            tcamautoexposure->camera_src = (GstElement*)g_value_get_object(value);
            break;
        }
        case PROP_EXPOSURE_MIN:
        {
            if (g_value_get_int(value) > tcamautoexposure->exposure_max)
            {
                GST_ERROR("New user value for exposure min is greater or equal to exposure max. Ignoring request.");
                break;
            }

            tcamautoexposure->exposure_min = g_value_get_int(value);

            if (tcamautoexposure->exposure.value < tcamautoexposure->exposure_min)
            {
                tcamautoexposure->exposure.value = tcamautoexposure->exposure_min;
                set_exposure(tcamautoexposure, tcamautoexposure->exposure.value);
            }

            if (tcamautoexposure->exposure_min == 0)
            {
                tcamautoexposure->exposure_min = tcamautoexposure->default_exposure_values.min;
            }
            break;
        }
        case PROP_EXPOSURE_MAX:
        {
            if (g_value_get_int(value) < tcamautoexposure->exposure_min)
            {
                GST_ERROR("New user value for exposure max is smaller or equal to exposure min. Ignoring request.");
                break;
            }

            if (tcamautoexposure->exposure.value > tcamautoexposure->exposure_max)
            {
                tcamautoexposure->exposure.value = tcamautoexposure->exposure_max;
                set_exposure(tcamautoexposure, tcamautoexposure->exposure.value);
            }

            tcamautoexposure->exposure_max = g_value_get_int(value);
            if (tcamautoexposure->exposure_max == 0.0)
            {
                tcamautoexposure->exposure_max = tcamautoexposure->default_exposure_values.max;
            }
            break;
        }
        case PROP_GAIN_MIN:
        {
            GST_DEBUG("Setting gain min to : %f", g_value_get_double(value));
            if (tcamautoexposure->gain_max && (g_value_get_double(value) > tcamautoexposure->gain_max))
            {
                GST_ERROR("New user value for gain min is greater or equal to gain max. Ignoring request.");
                break;
            }

            if (tcamautoexposure->default_gain_values.min && (g_value_get_double(value) > tcamautoexposure->default_gain_values.min))
            {
                GST_WARNING("New user value for gain min is greater than device gain min.");
                tcamautoexposure->gain_min = tcamautoexposure->default_gain_values.min;
                break;
            }

            tcamautoexposure->gain_min = g_value_get_double(value);

            if (tcamautoexposure->gain.value < tcamautoexposure->gain_min)
            {
                tcamautoexposure->gain.value = tcamautoexposure->gain_min;
                set_gain(tcamautoexposure, tcamautoexposure->gain.value);
            }

            if (tcamautoexposure->gain_min == 0.0)
            {
                tcamautoexposure->gain_min = tcamautoexposure->default_gain_values.min;
            }
            break;
        }
        case PROP_GAIN_MAX:
        {
            GST_DEBUG("Setting gain max to : %f", g_value_get_double(value));
            if (g_value_get_double(value) < tcamautoexposure->gain_min)
            {
                GST_ERROR("New user value for gain max is smaller or equal to gain min. Ignoring request.");
                break;
            }

            if (tcamautoexposure->gain.max && (g_value_get_double(value) > tcamautoexposure->gain.max))
            {
                GST_WARNING("New user value for gain max is bigger that device gain max. Ignoring request.");
                tcamautoexposure->gain_max = tcamautoexposure->default_gain_values.max;
                break;
            }

            tcamautoexposure->gain_max = g_value_get_double(value);

            if (tcamautoexposure->gain.value > tcamautoexposure->gain_max)
            {
                tcamautoexposure->gain.value = tcamautoexposure->gain_max;
                set_gain(tcamautoexposure, tcamautoexposure->gain.value);
            }

            if (tcamautoexposure->gain_max == 0.0)
            {
                tcamautoexposure->gain_max = tcamautoexposure->default_gain_values.max;
            }
            break;
        }
        case PROP_IRIS_MIN:
        {
            if (g_value_get_int(value) > tcamautoexposure->iris_max)
            {
                GST_ERROR("New user value for iris min is greater or equal to iris max. Ignoring request.");
                break;
            }

            tcamautoexposure->iris_min = g_value_get_int(value);

            if (tcamautoexposure->iris.value < tcamautoexposure->iris_min)
            {
                tcamautoexposure->iris.value = tcamautoexposure->iris_min;
                set_iris(tcamautoexposure, tcamautoexposure->iris.value);
            }

            if (tcamautoexposure->iris_min == 0)
            {
                tcamautoexposure->iris_min = tcamautoexposure->iris.min;
            }
            break;
        }
        case PROP_IRIS_MAX:
        {
            if (g_value_get_int(value) < tcamautoexposure->iris_min)
            {
                GST_ERROR("New user value for iris max is smaller or equal to iris min. Ignoring request.");
                break;
            }

            if (tcamautoexposure->iris.value > tcamautoexposure->iris_max)
            {
                tcamautoexposure->iris.value = tcamautoexposure->iris_max;
                set_iris(tcamautoexposure, tcamautoexposure->iris.value);
            }

            tcamautoexposure->iris_max = g_value_get_int(value);
            if (tcamautoexposure->iris_max == 0.0)
            {
                tcamautoexposure->iris_max = tcamautoexposure->iris.max;
            }
            break;
        }
        case PROP_BRIGHTNESS_REFERENCE:
        {
            tcamautoexposure->brightness_reference = g_value_get_int(value);
            break;
        }
        case PROP_ROI_LEFT:
        {
            roi_set_left(tcamautoexposure->roi, g_value_get_int(value));
            break;
        }
        case PROP_ROI_TOP:
        {
            roi_set_top(tcamautoexposure->roi, g_value_get_int(value));
            break;
        }
        case PROP_ROI_WIDTH:
        {
            roi_set_width(tcamautoexposure->roi, g_value_get_int(value));
            break;
        }
        case PROP_ROI_HEIGHT:
        {
            roi_set_height(tcamautoexposure->roi,
                           g_value_get_int(value));
            break;
        }
        default:
        {
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
        }
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
        case PROP_AUTO_IRIS:
            g_value_set_boolean (value, tcamautoexposure->auto_iris);
            break;
        case PROP_CAMERA:
            g_value_set_object (value, tcamautoexposure->camera_src);
            break;
        case PROP_EXPOSURE_MIN:
            g_value_set_int(value, tcamautoexposure->exposure_min);
            break;
        case PROP_EXPOSURE_MAX:
            g_value_set_int(value, tcamautoexposure->exposure_max);
            break;
        case PROP_GAIN_MIN:
            g_value_set_double(value, tcamautoexposure->gain_min);
            break;
        case PROP_GAIN_MAX:
            g_value_set_double(value, tcamautoexposure->gain_max);
            break;
        case PROP_IRIS_MIN:
            g_value_set_int(value, tcamautoexposure->iris_min);
            break;
        case PROP_IRIS_MAX:
            g_value_set_int(value, tcamautoexposure->iris_max);
            break;
         case PROP_BRIGHTNESS_REFERENCE:
            g_value_set_int(value, tcamautoexposure->brightness_reference);
            break;
        case PROP_ROI_LEFT:
            g_value_set_int(value, roi_left(tcamautoexposure->roi));
            break;
        case PROP_ROI_TOP:
            g_value_set_int(value, roi_top(tcamautoexposure->roi));
            break;
        case PROP_ROI_WIDTH:
            g_value_set_int(value, roi_width(tcamautoexposure->roi));
            break;
        case PROP_ROI_HEIGHT:
            g_value_set_int(value, roi_height(tcamautoexposure->roi));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

void gst_tcamautoexposure_finalize (GObject* object)
{
    auto self = GST_TCAMAUTOEXPOSURE(object);
    destroy_roi(self->roi);

    G_OBJECT_CLASS (gst_tcamautoexposure_parent_class)->finalize (object);
}


static void init_camera_resources (GstTcamautoexposure* self)
{
    tcam::CaptureDevice* dev = NULL;

    g_object_get(G_OBJECT(self->camera_src), "camera", &dev, NULL);

    struct tcam_device_property p = {};

    if (dev->get_property(TCAM_PROPERTY_GAIN_AUTO) != nullptr ||
        dev->get_property(TCAM_PROPERTY_EXPOSURE_AUTO) != nullptr)
    {
        GST_INFO("Device has auto properties. Disabling module.");
        self->module_is_disabled = TRUE;
        return;
    }

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

        self->default_exposure_values.max = 1000000 / double(self->framerate_numerator / self->framerate_denominator);
    }

    p = {};
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
            self->gain.step = p.value.i.step;
        }
        else
        {
            self->gain_is_double = TRUE;
            if (p.value.d.min == 0.0)
            {
                self->gain.min = GAIN_FLOAT_MULTIPLIER;
            }
            else
            {
                self->gain.min = p.value.d.min * GAIN_FLOAT_MULTIPLIER;
            }
            // self->gain.min = p.value.d.min * GAIN_FLOAT_MULTIPLIER;
            self->gain.max = p.value.d.max * GAIN_FLOAT_MULTIPLIER;
            self->gain.value = p.value.d.value * GAIN_FLOAT_MULTIPLIER;
            self->gain.step = p.value.d.step * GAIN_FLOAT_MULTIPLIER;
        }
        if (self->gain_max == 0
            || self->gain_max == G_MAXDOUBLE
            || self->gain_max > self->gain.max)
        {
            self->gain_max = self->gain.max;
        }
        if (self->gain_min == 0
            || self->gain_min < self->gain.min)
        {
            self->gain_min = self->gain.min;
        }
    }

    if (self->exposure_min == 0
        || self->exposure_min < self->default_exposure_values.min)
    {
        self->exposure_min = self->default_exposure_values.min;
    }
    if (self->exposure_max == 0
        || self->exposure_max == G_MAXINT
        || self->exposure_max > self->default_exposure_values.max)
    {
        self->exposure_max = self->default_exposure_values.max;
    }

    p = {};
    prop = dev->get_property(TCAM_PROPERTY_IRIS);

    if (prop == nullptr)
    {
        GST_INFO("Iris could not be found");
    }
    else
    {
        self->has_iris = true;
        p = prop->get_struct();
        //self->iris.min = p.value.i.min;
        // most iris modules are the same
        // with a range from 0 - 1023
        // 100 is a good base number.
        // GigE cameras offer IrisAutoMin that suggests what this should be
        self->iris.min = 100;
        self->iris.max = p.value.i.max;
        self->iris.value = p.value.i.value;

        if (self->iris_min == 0)
        {
            self->iris_min = self->iris.min;
        }
        if (self->iris_max == 0)
        {
            self->iris_max = self->iris.max;
        }
    }

    GST_INFO("Exposure boundaries are %f %d", self->exposure.min, self->exposure_max);

    GST_INFO("Gain boundaries are %f %f", self->gain.min, self->gain.max);

    if (self->has_iris)
    {
        GST_INFO("Camera has an iris.");
        GST_INFO("Iris boundaries are %d %d", self->iris.min, self->iris.max);
    }
    else
    {
        GST_INFO("Camera does not have an iris. Disabling functionality");
    }
}


static void set_exposure (GstTcamautoexposure* self, gdouble exposure)
{
    if (!G_IS_OBJECT(self->camera_src))
    {
        GST_WARNING("Have no camera source to set exposure.");
        return;
    }

    //GST_INFO("Setting exposure to %f", exposure);

    tcam::CaptureDevice* dev;
    g_object_get(G_OBJECT(self->camera_src), "camera", &dev, NULL);

    dev->set_property(TCAM_PROPERTY_EXPOSURE, (int64_t)exposure);
}


static void set_gain (GstTcamautoexposure* self, gdouble gain)
{
    if (!G_IS_OBJECT(self->camera_src))
    {
        GST_WARNING("Have no camera source to set gain.");
        return;
    }

    GST_INFO("Setting gain to %f", gain);

    tcam::CaptureDevice* dev;
    g_object_get(G_OBJECT(self->camera_src), "camera", &dev, NULL);

    if (!self->gain_is_double)
    {
        dev->set_property(TCAM_PROPERTY_GAIN, (int64_t)std::lround(gain));
    }
    else
    {
        dev->set_property(TCAM_PROPERTY_GAIN, (float)gain / GAIN_FLOAT_MULTIPLIER);
        GST_INFO("Setting gain to %f", (float)gain /  GAIN_FLOAT_MULTIPLIER);

    }
}


static void set_iris (GstTcamautoexposure* self, int iris)
{
    if (!G_IS_OBJECT(self->camera_src))
    {
        GST_WARNING("Have no camera source to set iris.");
        return;
    }

    GST_INFO("Setting iris to %d", iris);

    tcam::CaptureDevice* dev;
    g_object_get(G_OBJECT(self->camera_src), "camera", &dev, NULL);

    dev->set_property(TCAM_PROPERTY_IRIS, (int64_t)iris);
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
        //GST_DEBUG("Current exposure is %ld", p.value.i.value);
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
            self->gain.value = p.value.d.value * GAIN_FLOAT_MULTIPLIER;
        }
    }

    if (self->has_iris)
    {
        prop =  dev->get_property(TCAM_PROPERTY_IRIS);

        if (prop == nullptr)
        {
            GST_ERROR("Tcam did not return iris");
        }
        else
        {
            p = prop->get_struct();
            //GST_DEBUG("Current iris is %ld", p.value.i.value);
            self->iris.value = p.value.i.value;
        }
    }
    else
    {
        self->auto_iris = false;
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


uint32_t get_pitch (unsigned int width, uint32_t fourcc)
{
    return width * (img::get_bits_per_pixel(fourcc) / 8);
}


static image_buffer retrieve_image_region (GstTcamautoexposure* self, GstBuffer* buf)
{
    GstMapInfo info;

    gst_buffer_map(buf, &info, GST_MAP_READ);

    tcam_image_buffer image = {};


    gst_buffer_to_tcam_image_buffer(buf, &image);
    image.format = self->active_format;

    tcam_image_buffer roi_buffer = {};

    if (!roi_extract_view(self->roi, &image, &roi_buffer))
    {
        GST_ERROR("Unable to extract ROI");
        return {};
    }

    roi_buffer.length = 1228800;

    // roi_buffer.pitch = get_pitch(roi_buffer.format.width,
    //                              roi_buffer.format.fourcc);


    image_buffer new_buf = {};

    new_buf.image = roi_buffer.pData;
    new_buf.width = roi_buffer.format.width;
    new_buf.height = roi_buffer.format.height;

    new_buf.pattern = calculate_pattern_from_offset(self);
    new_buf.rowstride = roi_buffer.pitch;

    // GST_INFO("Region is from %d %d to %d %d",
    //          reg.x0, reg.y0,
    //          reg.x0 + reg.x1,
    //          reg.y0 + reg.y1);



    gst_buffer_unmap(buf, &info);

    return new_buf;
}


static void new_exposure (GstTcamautoexposure* self, unsigned int brightness)
{
    algorithms::property_cont_gain gain = {};
    gain.is_db_gain = false;
    gain.min = self->gain_min;
    gain.max = self->gain_max;
    gain.val = self->gain.value;
    gain.do_auto = self->auto_gain;

    algorithms::property_cont_exposure exposure = {};
    exposure.min = self->exposure_min;
    exposure.max = self->exposure_max;
    exposure.val = self->exposure.value;
    exposure.do_auto = self->auto_exposure;
    // exposure.granularity = self->exposure.step;

    algorithms::property_cont_iris iris = {};
    if (self->has_iris)
    {
        iris.camera_fps = self->framerate_numerator / self->framerate_denominator;
        iris.do_auto = self->auto_iris;
        iris.min = self->iris_min;
        iris.max = self->iris_max;
        iris.val = self->iris.value;
        iris.is_pwm_iris = false;
    }
    else
    {
        iris.do_auto = false;
    }

    auto res = algorithms::calc_auto_gain_exposure_iris(brightness,
                                                        self->brightness_reference,
                                                        gain,
                                                        exposure,
                                                        iris);

    if (self->auto_exposure)
    {
        set_exposure(self, res.exposure);
    }

    if (self->auto_gain)
    {
        set_gain(self, res.gain);
    }

    if (self->auto_iris)
    {
        set_iris(self, res.iris);
    }
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
        if (self->bit_depth == 8)
        {
            GST_DEBUG("Calculating brightness for gray");
            brightness = buffer_brightness_gray(&buffer);
        }
        else
        {
            // the algorithm is designed for 8-bit brightness
            // we thus need to convert the 16-bit brightness
            // back to something that makes sense
            brightness = buffer_brightness_gray16(&buffer) / 256;
        }
    }

    GST_INFO("Calculated brightness: %u", brightness);

    /* assure we have the current values */
    retrieve_current_values (self);

    new_exposure(self, brightness);

    return;
}


gboolean find_image_values (GstTcamautoexposure* self)
{
    GstPad* pad = GST_BASE_TRANSFORM_SINK_PAD(self);
    GstCaps* caps = gst_pad_get_current_caps(pad);

    gst_caps_to_tcam_video_format(caps, &self->active_format);

    GstStructure *structure = gst_caps_get_structure (caps, 0);

    gint tmp_w, tmp_h;
    g_return_val_if_fail (gst_structure_get_int (structure, "width", &tmp_w), FALSE);
    g_return_val_if_fail (gst_structure_get_int (structure, "height", &tmp_h), FALSE);
    self->image_size.width  = tmp_w < 0 ? 0 : tmp_w;
    self->image_size.height = tmp_h < 0 ? 0 : tmp_h;

    if (self->image_region.x1 == 0)
    {
        self->image_region.x1 = self->image_size.width;
    }

    if (self->image_region.y1 == 0)
    {
        self->image_region.y1 = self->image_size.height;
    }

    gst_structure_get_fraction(structure, "framerate",
                               &self->framerate_numerator, &self->framerate_denominator);

    if (strcmp(gst_structure_get_name(structure), "video/x-bayer") == 0)
    {
        self->color_format = BAYER;

        guint fourcc = 0;

        if (gst_structure_get_field_type(structure, "format") == G_TYPE_STRING)
        {
            const char *string;
            string = gst_structure_get_string (structure, "format");
            fourcc = GST_STR_FOURCC (string);
        }

        if (fourcc == MAKE_FOURCC ('g','r','b','g'))
        {
            self->pattern = GR;
        }
        else if (fourcc == MAKE_FOURCC ('r', 'g', 'g', 'b'))
        {
            self->pattern = RG;
        }
        else if (fourcc == MAKE_FOURCC ('g', 'b', 'r', 'g'))
        {
            self->pattern = GB;
        }
        else if (fourcc == MAKE_FOURCC ('b', 'g', 'g', 'r'))
        {
            self->pattern = BG;
        }
        // else
        // {
        //     GST_ERROR("Unable to determine bayer pattern.");
        //     return FALSE;
        // }
    }
    else
    {
        self->color_format = GRAY;
        // will not be used, but still explicitly define something
        self->pattern = BG;
    }



    return TRUE;
}


/*
  Entry point for actual transformation
*/
static GstFlowReturn gst_tcamautoexposure_transform_ip (GstBaseTransform* trans, GstBuffer* buf)
{
    GstTcamautoexposure* self = GST_TCAMAUTOEXPOSURE (trans);

    // module is disabled when device has any auto property
    if (self->module_is_disabled)
    {
        return GST_FLOW_OK;
    }

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

    if (self->auto_exposure == FALSE
        && self->auto_gain == FALSE
        && self->auto_iris == FALSE)
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

        if (data == NULL || length == 0)
        {
            gst_buffer_unmap(buf, &info);
            GST_WARNING("Buffer is not valid! Ignoring buffer and trying to continue...");
            return GST_FLOW_OK;
        }
        gst_buffer_unmap(buf, &info);

        correct_brightness(self, buf);
        self->frame_counter = 0;
    }
    self->frame_counter++;

    return GST_FLOW_OK;
}


static gboolean gst_tcamautoexposure_set_caps (GstBaseTransform* trans,
                                               GstCaps* incaps,
                                               GstCaps* outcaps)
{
    GstTcamautoexposure* self = GST_TCAMAUTOEXPOSURE(trans);
    GstStructure* structure = nullptr;

    GST_DEBUG("in caps %" GST_PTR_FORMAT " out caps %" GST_PTR_FORMAT,
              (void*)incaps,
              (void*)outcaps);
    structure = gst_caps_get_structure(incaps, 0);

    self->bit_depth = 8;

    if (g_str_equal(gst_structure_get_name(structure), "video/x-bayer"))
    {
        const char *format;
        format = gst_structure_get_string (structure, "format");
        self->color_format = BAYER;
        if (g_str_equal (format, "bggr"))
        {
            self->pattern = BG;
        }
        else if (g_str_equal (format, "gbrg"))
        {
            self->pattern = GB;
        }
        else if (g_str_equal (format, "grbg"))
        {
            self->pattern = GR;
        }
        else if (g_str_equal (format, "rggb"))
        {
            self->pattern = RG;
        }
        else
        {
            g_critical("Format '%s' not handled by this element", format);
            g_return_val_if_reached(false);
        }
    }
    else
    {
        self->pattern = UNDEFINED_PATTERN;
        self->color_format = GRAY;

        if (g_str_equal(gst_structure_get_string(structure, "format"), "GRAY16_LE"))
        {
            self->bit_depth = 16;
        }
    }

    unsigned int width;
    gst_structure_get_int(structure, "width", (int*)&width);

    unsigned int height;
    gst_structure_get_int(structure, "height", (int*)&height);

    roi_set_image_size(self->roi, {width, height});

    return true;
}


static gboolean plugin_init (GstPlugin* plugin)
{
    return gst_element_register(plugin,
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
