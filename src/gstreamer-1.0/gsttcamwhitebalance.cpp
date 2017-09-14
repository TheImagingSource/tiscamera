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
 * SECTION:element-gsttiswhitebalance
 *
 * The tiswhitebalance element analyzes the color temperatures of the incomming buffers and applies a whitebalancing.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v fakesrc ! tiswhitebalance ! bayer ! fakesink
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>

#include <gst/base/gstbasetransform.h>
#include "gsttcamwhitebalance.h"
#include "tcamgstbase.h"
#include "tcamprop.h"
#include <stdlib.h>
#include <cstring>

#include "tcam.h"
#include <glib-object.h>

GST_DEBUG_CATEGORY_STATIC (gst_tcamwhitebalance_debug_category);
#define GST_CAT_DEFAULT gst_tcamwhitebalance_debug_category

enum
{
    PROP_0,
    PROP_GAIN_RED,
    PROP_GAIN_GREEN,
    PROP_GAIN_BLUE,
    PROP_AUTO_ENABLED,
    PROP_WHITEBALANCE_ENABLED,
    PROP_CAMERA_WB,
};


/* prototypes */

static void gst_tcamwhitebalance_set_property (GObject* object,
                                               guint property_id,
                                               const GValue* value,
                                               GParamSpec* pspec);
static void gst_tcamwhitebalance_get_property (GObject* object,
                                               guint property_id,
                                               GValue* value,
                                               GParamSpec* pspec);
static void gst_tcamwhitebalance_finalize (GObject* object);

static GstFlowReturn gst_tcamwhitebalance_transform_ip (GstBaseTransform* trans, GstBuffer* buf);
static GstCaps* gst_tcamwhitebalance_transform_caps (GstBaseTransform* trans,
                                                     GstPadDirection direction,
                                                     GstCaps* caps);

static void gst_tcamwhitebalance_fixate_caps (GstBaseTransform* base,
                                              GstPadDirection direction,
                                              GstCaps* caps,
                                              GstCaps* othercaps);

static GSList* gst_tcamwhitebalance_get_property_names(TcamProp* self);

static const gchar *gst_tcamwhitebalance_get_property_type (TcamProp* self, gchar* name);

static gboolean gst_tcamwhitebalance_get_tcam_property (TcamProp* self,
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

static gboolean gst_tcamwhitebalance_set_tcam_property (TcamProp* self,
                                                        gchar* name,
                                                        const GValue* value);

static GSList* gst_tcamwhitebalance_get_tcam_menu_entries (TcamProp* self,
                                                           const gchar* name);

static GSList* gst_tcamwhitebalance_get_device_serials (TcamProp* self);

static gboolean gst_tcamwhitebalance_get_device_info (TcamProp* self,
                                                      const char* serial,
                                                      char** name,
                                                      char** identifier,
                                                      char** connection_type);

static void gst_tcamwhitebalance_prop_init (TcamPropInterface* iface)
{
    iface->get_property_names = gst_tcamwhitebalance_get_property_names;
    iface->get_property_type = gst_tcamwhitebalance_get_property_type;
    iface->get_property = gst_tcamwhitebalance_get_tcam_property;
    iface->get_menu_entries = gst_tcamwhitebalance_get_tcam_menu_entries;
    iface->set_property = gst_tcamwhitebalance_set_tcam_property;
    iface->get_device_serials = gst_tcamwhitebalance_get_device_serials;
    iface->get_device_info = gst_tcamwhitebalance_get_device_info;
}



G_DEFINE_TYPE_WITH_CODE (GstTcamWhitebalance, gst_tcamwhitebalance, GST_TYPE_BASE_TRANSFORM,
                         G_IMPLEMENT_INTERFACE (TCAM_TYPE_PROP,
                                                gst_tcamwhitebalance_prop_init));


static const char* tcamwhitebalance_property_id_to_string (guint id)
{
    switch (id)
    {
        case PROP_GAIN_RED:
            return "whitebalance-red";
        case PROP_GAIN_GREEN:
            return "whitebalance-green";
        case PROP_GAIN_BLUE:
            return "whitebalance-blue";
        case PROP_AUTO_ENABLED:
            return "whitebalance-auto";
        case PROP_WHITEBALANCE_ENABLED:
            return "camera-whitebalance";
        case PROP_CAMERA_WB:
            return "whitebalance-module-enabled";
        default:
            return "";
    }
}


static guint tcamwhitebalance_string_to_property_id (const char* name)
{
    if (strcmp(name, "whitebalance-red") == 0)
    {
        return PROP_GAIN_RED;
    }
    else if (strcmp(name, "whitebalance-green") == 0)
    {
        return PROP_GAIN_GREEN;
    }
    else if (strcmp(name, "whitebalance-blue") == 0)
    {
        return PROP_GAIN_BLUE;
    }
    else if (strcmp(name, "whitebalance-auto") == 0)
    {
        return PROP_AUTO_ENABLED;
    }
    else if (strcmp(name, "camera-whitebalance") == 0)
    {
        return PROP_CAMERA_WB;
    }
    else if (strcmp(name, "whitebalance-module-enabled") == 0)
    {
        return PROP_WHITEBALANCE_ENABLED;
    }
    else
    {
        return PROP_0;
    }
}



static GSList* gst_tcamwhitebalance_get_property_names (TcamProp* self)
{
    GSList* names = nullptr;

    names = g_slist_append(names,
        g_strdup(tcamwhitebalance_property_id_to_string(PROP_GAIN_RED)));
    names = g_slist_append(names,
        g_strdup(tcamwhitebalance_property_id_to_string(PROP_GAIN_GREEN)));
    names = g_slist_append(names,
        g_strdup(tcamwhitebalance_property_id_to_string(PROP_GAIN_BLUE)));
    names = g_slist_append(names,
        g_strdup(tcamwhitebalance_property_id_to_string(PROP_AUTO_ENABLED)));
    names = g_slist_append(names,
        g_strdup(tcamwhitebalance_property_id_to_string(PROP_CAMERA_WB)));
    names = g_slist_append(names,
        g_strdup(tcamwhitebalance_property_id_to_string(PROP_WHITEBALANCE_ENABLED)));

    return names;
}


static const gchar* gst_tcamwhitebalance_get_property_type (TcamProp* self, gchar* name)
{
    if (strcmp(name, "whitebalance-red") == 0)
    {
        return strdup("integer");
    }
    else if (strcmp(name, "whitebalance-green") == 0)
    {
        return strdup("integer");
    }
    else if (strcmp(name, "whitebalance-blue") == 0)
    {
        return strdup("integer");
    }
    else if (strcmp(name, "whitebalance-auto") == 0)
    {
        return strdup("boolean");
    }
    else if (strcmp(name, "camera-whitebalance") == 0)
    {
        return strdup("boolean");
    }
    else if (strcmp(name, "whitebalance-module-enabled") == 0)
    {
        return strdup("boolean");
    }
    else
    {
        return nullptr;
    }
}


static gboolean gst_tcamwhitebalance_get_tcam_property (TcamProp* prop,
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
    GstTcamWhitebalance* self = GST_TCAMWHITEBALANCE(prop);

    if (strcmp(name, "whitebalance-red") == 0)
    {
        if (value)
        {
            g_value_init(value, G_TYPE_INT);
            g_value_set_int(value, self->red);
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
            g_value_set_int(def, 64);
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
            g_value_set_string(type,gst_tcamwhitebalance_get_property_type(prop, name));
        }
        if (category)
        {
            g_value_init(category, G_TYPE_STRING);
            g_value_set_string(category, "Color");
        }
        if (group)
        {
            g_value_init(group, G_TYPE_STRING);
            g_value_set_string(group, "Whitebalance");
        }

        return TRUE;
    }
    else if (strcmp(name, "whitebalance-green") == 0)
    {
        if (value)
        {
            g_value_init(value, G_TYPE_INT);
            g_value_set_int(value, self->green);
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
            g_value_set_int(def, 64);
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
            g_value_set_string(type,gst_tcamwhitebalance_get_property_type(prop, name));
        }
        if (category)
        {
            g_value_init(category, G_TYPE_STRING);
            g_value_set_string(category, "Color");
        }
        if (group)
        {
            g_value_init(group, G_TYPE_STRING);
            g_value_set_string(group, "Whitebalance");
        }
        return TRUE;
    }
    else if (strcmp(name, "whitebalance-blue") == 0)
    {
        if (value)
        {
            g_value_init(value, G_TYPE_INT);
            g_value_set_int(value, self->blue);
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
            g_value_set_int(def, 64);
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
            g_value_set_string(type,gst_tcamwhitebalance_get_property_type(prop, name));
        }
        if (category)
        {
            g_value_init(category, G_TYPE_STRING);
            g_value_set_string(category, "Color");
        }
        if (group)
        {
            g_value_init(group, G_TYPE_STRING);
            g_value_set_string(group, "Whitebalance");
        }
        return TRUE;
    }
    else if (strcmp(name, "whitebalance-auto") == 0)
    {
        if (value)
        {
            g_value_init(value, G_TYPE_BOOLEAN);
            g_value_set_boolean(value, self->auto_enabled);
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
            g_value_set_string(type,gst_tcamwhitebalance_get_property_type(prop, name));
        }
        if (category)
        {
            g_value_init(category, G_TYPE_STRING);
            g_value_set_string(category, "Color");
        }
        if (group)
        {
            g_value_init(group, G_TYPE_STRING);
            g_value_set_string(group, "Whitebalance");
        }
        return TRUE;
    }
    else if (strcmp(name, "camera-whitebalance") == 0)
    {
        if (value)
        {
            g_value_init(value, G_TYPE_BOOLEAN);
            g_value_set_boolean(value, self->auto_enabled);
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
            g_value_set_boolean(def, FALSE);
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
            g_value_set_string(type,gst_tcamwhitebalance_get_property_type(prop, name));
        }
        if (category)
        {
            g_value_init(category, G_TYPE_STRING);
            g_value_set_string(category, "Color");
        }
        if (group)
        {
            g_value_init(group, G_TYPE_STRING);
            g_value_set_string(group, "Whitebalance");
        }        return TRUE;
    }
    else if (strcmp(name, "whitebalance-module-enabled") == 0)
    {
        if (value)
        {
            g_value_init(value, G_TYPE_BOOLEAN);
            g_value_set_boolean(value, self->auto_enabled);
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
            g_value_set_string(type,gst_tcamwhitebalance_get_property_type(prop, name));
        }
        if (category)
        {
            g_value_init(category, G_TYPE_STRING);
            g_value_set_string(category, "Color");
        }
        if (group)
        {
            g_value_init(group, G_TYPE_STRING);
            g_value_set_string(group, "Whitebalance");
        }
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


static gboolean gst_tcamwhitebalance_set_tcam_property (TcamProp* self,
                                                        gchar* name,
                                                        const GValue* value)
{

    guint id = tcamwhitebalance_string_to_property_id(name);

    if (id == PROP_0)
    {
        return FALSE;
    }

    gst_tcamwhitebalance_set_property(G_OBJECT(self), id, value, NULL);

    return TRUE;
}


static GSList* gst_tcamwhitebalance_get_tcam_menu_entries (TcamProp* self,
                                                           const gchar* name)
{
    return nullptr;
}


static GSList* gst_tcamwhitebalance_get_device_serials (TcamProp* self)
{
    return nullptr;
}


static gboolean gst_tcamwhitebalance_get_device_info (TcamProp* self,
                                                      const char* serial,
                                                      char** name,
                                                      char** identifier,
                                                      char** connection_type)
{
    return FALSE;
}

/* actual gstreamer element */


/* pad templates */

static GstStaticPadTemplate gst_tcamwhitebalance_sink_template =
    GST_STATIC_PAD_TEMPLATE ("sink",
                             GST_PAD_SINK,
                             GST_PAD_ALWAYS,
                             GST_STATIC_CAPS ("video/x-bayer,format=(string){bggr,grbg,gbrg,rggb},framerate=(fraction)[0/1,MAX],width=[1,MAX],height=[1,MAX]")
        );

static GstStaticPadTemplate gst_tcamwhitebalance_src_template =
    GST_STATIC_PAD_TEMPLATE ("src",
                             GST_PAD_SRC,
                             GST_PAD_ALWAYS,
                             GST_STATIC_CAPS ("video/x-bayer,format=(string){bggr,grbg,gbrg,rggb},framerate=(fraction)[0/1,MAX],width=[1,MAX],height=[1,MAX]")
        );


/* class initialization */

static void gst_tcamwhitebalance_class_init (GstTcamWhitebalanceClass* klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    GstBaseTransformClass *base_transform_class = GST_BASE_TRANSFORM_CLASS(klass);

    gst_element_class_add_pad_template(GST_ELEMENT_CLASS(klass),
                                       gst_static_pad_template_get(&gst_tcamwhitebalance_src_template));
    gst_element_class_add_pad_template(GST_ELEMENT_CLASS(klass),
                                       gst_static_pad_template_get(&gst_tcamwhitebalance_sink_template));

    gst_element_class_set_details_simple(GST_ELEMENT_CLASS(klass),
                                         "The Imaging Source White Balance Element",
                                         "Generic",
                                         "Adjusts white balancing of video data buffers",
                                         "The Imaging Source Europe GmbH <support@theimagingsource.com>");

    gobject_class->set_property = gst_tcamwhitebalance_set_property;
    gobject_class->get_property = gst_tcamwhitebalance_get_property;
    gobject_class->finalize = gst_tcamwhitebalance_finalize;
    base_transform_class->transform_ip = GST_DEBUG_FUNCPTR(gst_tcamwhitebalance_transform_ip);

    GST_DEBUG_CATEGORY_INIT(gst_tcamwhitebalance_debug_category, "tcamwhitebalance", 0, "tcam whitebalance");

    g_object_class_install_property(gobject_class,
                                    PROP_GAIN_RED,
                                    g_param_spec_int("red",
                                                     "Red",
                                                     "Value for red",
                                                     0, 255, 0,
                                                     G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    g_object_class_install_property(gobject_class,
                                    PROP_GAIN_GREEN,
                                    g_param_spec_int("green",
                                                     "Green",
                                                     "Value for red",
                                                     0, 255, 0,
                                                     G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    g_object_class_install_property(gobject_class,
                                    PROP_GAIN_BLUE,
                                    g_param_spec_int("blue",
                                                     "Blue",
                                                     "Value for blue",
                                                     0, 255, 0,
                                                     G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    g_object_class_install_property(gobject_class,
                                    PROP_AUTO_ENABLED,
                                    g_param_spec_boolean("auto",
                                                         "Auto Value Adjustment",
                                                         "Automatically adjust white balance values",
                                                         TRUE,
                                                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    g_object_class_install_property(gobject_class,
                                    PROP_CAMERA_WB,
                                    g_param_spec_boolean("camera-whitebalance",
                                                         "Device whitebalance settings",
                                                         "Adjust whitebalance values in the camera",
                                                         FALSE,
                                                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    g_object_class_install_property(gobject_class,
                                    PROP_WHITEBALANCE_ENABLED,
                                    g_param_spec_boolean("module-enabled",
                                                         "Enable/Disable White Balance Module",
                                                         "Disable entire module",
                                                         TRUE,
                                                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}


static void init_wb_values (GstTcamWhitebalance* self)
{
    self->rgb = (rgb_tripel){WB_IDENTITY, WB_IDENTITY, WB_IDENTITY};
    self->red = WB_IDENTITY;
    self->green = WB_IDENTITY;
    self->blue = WB_IDENTITY;
}


static void gst_tcamwhitebalance_init (GstTcamWhitebalance* self)
{
    gst_base_transform_set_in_place(GST_BASE_TRANSFORM(self), TRUE);

    init_wb_values(self);
    self->auto_wb = TRUE;

    self->image_size.width = 0;
    self->image_size.height = 0;
}


void gst_tcamwhitebalance_set_property (GObject* object,
                                        guint property_id,
                                        const GValue* value,
                                        GParamSpec* pspec)
{
    GstTcamWhitebalance* tcamwhitebalance = GST_TCAMWHITEBALANCE(object);

    switch (property_id)
    {
        case PROP_GAIN_RED:
            tcamwhitebalance->red = g_value_get_int(value);
            break;
        case PROP_GAIN_GREEN:
            tcamwhitebalance->green = g_value_get_int(value);
            break;
        case PROP_GAIN_BLUE:
            tcamwhitebalance->blue = g_value_get_int(value);
            break;
        case PROP_AUTO_ENABLED:
            tcamwhitebalance->auto_wb = g_value_get_boolean(value);
            break;
        case PROP_WHITEBALANCE_ENABLED:
            tcamwhitebalance->auto_enabled = g_value_get_boolean(value);
            break;
        case PROP_CAMERA_WB:
            tcamwhitebalance->force_hardware_wb = g_value_get_boolean(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}


void gst_tcamwhitebalance_get_property (GObject* object,
                                        guint property_id,
                                        GValue* value,
                                        GParamSpec* pspec)
{
    GstTcamWhitebalance *tcamwhitebalance = GST_TCAMWHITEBALANCE(object);

    switch (property_id)
    {
        case PROP_GAIN_RED:
            g_value_set_int(value, tcamwhitebalance->red);
            break;
        case PROP_GAIN_GREEN:
            g_value_set_int(value, tcamwhitebalance->green);
            break;
        case PROP_GAIN_BLUE:
            g_value_set_int(value, tcamwhitebalance->blue);
            break;
        case PROP_AUTO_ENABLED:
            g_value_set_boolean(value, tcamwhitebalance->auto_wb);
            break;
        case PROP_WHITEBALANCE_ENABLED:
            g_value_set_boolean(value, tcamwhitebalance->auto_enabled);
            break;
        case PROP_CAMERA_WB:
            g_value_set_boolean(value, tcamwhitebalance->force_hardware_wb);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}


void gst_tcamwhitebalance_finalize (GObject* object)
{
    G_OBJECT_CLASS(gst_tcamwhitebalance_parent_class)->finalize (object);
}


static gboolean gst_tcamwhitebalance_device_set_whiteblance (GstTcamWhitebalance* self)
{
    GST_INFO("Applying white balance to device with values: R:%d G:%d B:%d",
             self->res.color.rgb.R,
             self->res.color.rgb.G,
             self->res.color.rgb.B);

    tcam::CaptureDevice* dev;
    g_object_get(G_OBJECT(self->res.source_element), "camera", &dev, NULL);


    tcam::Property* p = dev->get_property(TCAM_PROPERTY_GAIN_RED);

    if (p == nullptr)
    {
        GST_ERROR("Unable to retrieve gain red property");
    }

    if (p->set_value((int64_t)self->res.color.rgb.R))
    {
        return FALSE;
    }

    p = dev->get_property(TCAM_PROPERTY_GAIN_GREEN);

    if (p == nullptr)
    {
        GST_ERROR("Unable to retrieve gain green property");
    }

    if (p->set_value((int64_t)self->res.color.rgb.G))
    {
        return FALSE;
    }

    p = dev->get_property(TCAM_PROPERTY_GAIN_BLUE);

    if (p == nullptr)
    {
        GST_ERROR("Unable to retrieve gain blue property");
    }

    if (p->set_value((int64_t)self->res.color.rgb.B))
    {
        return FALSE;
    }

    return TRUE;

    /* return device_set_rgb(&self->res); */
}


static guint clip (guint x, guint max)
{
    if ( x > max )
        return max;
    return x;
}


guint calc_brightness_from_clr_avg (guint r, guint g, guint b)
{
    return (r * r_factor + g * g_factor + b * b_factor) >> 8;
}


gboolean is_near_gray (guint r, guint g, guint b)
{
    guint brightness = calc_brightness_from_clr_avg( r, g, b );
    if ( brightness < NEARGRAY_MIN_BRIGHTNESS ) return FALSE;
    if ( brightness > NEARGRAY_MAX_BRIGHTNESS ) return FALSE;

    guint deltaR = abs( (gint)r - (gint)brightness );
    guint deltaG = abs( (gint)g - (gint)brightness );
    guint deltaB = abs( (gint)b - (gint)brightness );

    float devR = deltaR / (float)brightness;
    float devG = deltaG / (float)brightness;
    float devB = deltaB / (float)brightness;

    return ((devR < NEARGRAY_MAX_COLOR_DEVIATION) &&
            (devG < NEARGRAY_MAX_COLOR_DEVIATION) &&
            (devB < NEARGRAY_MAX_COLOR_DEVIATION));
}


rgb_tripel simulate_whitebalance (const auto_sample_points* data,
                                  const rgb_tripel* wb,
                                  gboolean enable_near_gray)
{
    rgb_tripel result = { 0, 0, 0 };
    rgb_tripel result_near_gray = { 0, 0, 0 };
    unsigned int count_near_gray = 0;

    guint i;
    for (i = 0; i < data->cnt; ++i)
    {
        unsigned int r = clip( data->samples[i].r * wb->R / WB_IDENTITY, WB_MAX );
        unsigned int g = clip( data->samples[i].g * wb->G / WB_IDENTITY, WB_MAX );
        unsigned int b = clip( data->samples[i].b * wb->B / WB_IDENTITY, WB_MAX );

        result.R += r;
        result.G += g;
        result.B += b;

        if ( is_near_gray( r, g, b ) )
        {
            result_near_gray.R += r;
            result_near_gray.G += g;
            result_near_gray.B += b;
            count_near_gray += 1;
        }
    }

    float near_gray_amount = count_near_gray / (float)data->cnt;

    if ((near_gray_amount < NEARGRAY_REQUIRED_AMOUNT) || !enable_near_gray)
    {
        result.R /= data->cnt;
        result.G /= data->cnt;
        result.B /= data->cnt;
        return result;
    }
    else
    {
        result_near_gray.R /= count_near_gray;
        result_near_gray.G /= count_near_gray;
        result_near_gray.B /= count_near_gray;
        return result_near_gray;
    }
}


static rgb_tripel average_color_cam (const auto_sample_points* data)
{
    rgb_tripel result = { 0, 0, 0 };

	guint i;
    for (i = 0; i < data->cnt; ++i)
    {
        unsigned int r =  data->samples[i].r ;
        unsigned int g = data->samples[i].g ;
        unsigned int b = data->samples[i].b ;

        result.R += r;
        result.G += g;
        result.B += b;
    }

	result.R /= data->cnt;
	result.G /= data->cnt;
	result.B /= data->cnt;
	return result;
}


gboolean wb_auto_step (rgb_tripel* clr, rgb_tripel* wb )
{
    unsigned int avg = ((clr->R + clr->G + clr->B) / 3);
    int dr = (int)avg - clr->R;
    int dg = (int)avg - clr->G;
    int db = (int)avg - clr->B;

    if (abs(dr) < BREAK_DIFF && abs(dg) < BREAK_DIFF && abs(db) < BREAK_DIFF)
    {
        wb->R = clip( wb->R, WB_MAX );
        wb->G = clip( wb->G, WB_MAX );
        wb->B = clip( wb->B, WB_MAX );

        return TRUE;
    }

    if ((clr->R > avg) && (wb->R > WB_IDENTITY))
    {
        wb->R -= 1;
    }

    if ((clr->G > avg) && (wb->G > WB_IDENTITY))
    {
        wb->G -= 1;
    }

    if ((clr->B > avg) && (wb->B > WB_IDENTITY))
    {
        wb->B -= 1;
    }

    if ((clr->R < avg) && (wb->R < WB_MAX))
    {
        wb->R += 1;
    }

    if ((clr->G < avg) && (wb->G < WB_MAX))
    {
        wb->G += 1;
    }

    if ((clr->B < avg) && (wb->B < WB_MAX))
    {
        wb->B += 1;
    }

    if ((wb->R > WB_IDENTITY) && (wb->G > WB_IDENTITY) && (wb->B > WB_IDENTITY))
    {
        wb->R -= 1;
        wb->G -= 1;
        wb->B -= 1;
    }

    return FALSE;
}


gboolean auto_whitebalance (const auto_sample_points* data, rgb_tripel* wb, guint* resulting_brightness)
{
    rgb_tripel old_wb = *wb;
    if (wb->R < WB_IDENTITY)
        wb->R = WB_IDENTITY;
    if (wb->G < WB_IDENTITY)
        wb->G = WB_IDENTITY;
    if (wb->B < WB_IDENTITY)
        wb->B = WB_IDENTITY;
    if (old_wb.R != wb->R || old_wb.G != wb->G || old_wb.B != wb->B)
        return FALSE;

    while ((wb->R > WB_IDENTITY) && (wb->G > WB_IDENTITY) && (wb->B > WB_IDENTITY))
    {
        wb->R -= 1;
        wb->G -= 1;
        wb->B -= 1;
    }

    unsigned int steps = 0;
    while (steps++ < MAX_STEPS)
    {
        rgb_tripel tmp = simulate_whitebalance( data, wb, TRUE );

        // Simulate white balance once more, this time always on the whole image
        rgb_tripel tmp2 = simulate_whitebalance(data, wb, FALSE);
        *resulting_brightness = calc_brightness_from_clr_avg(tmp2.R, tmp2.G, tmp2.B);

        if (wb_auto_step(&tmp, wb))
        {
            return TRUE;
        }
    }
    wb->R = clip( wb->R, WB_MAX );
    wb->G = clip( wb->G, WB_MAX );
    wb->B = clip( wb->B, WB_MAX );

    return FALSE;
}


gboolean auto_whitebalance_cam (const auto_sample_points* data, rgb_tripel* wb )
{
    rgb_tripel old_wb = *wb;

    if (wb->R < WB_IDENTITY)
        wb->R = WB_IDENTITY;
    if (wb->G < WB_IDENTITY)
        wb->G = WB_IDENTITY;
    if (wb->B < WB_IDENTITY)
        wb->B = WB_IDENTITY;
    if (old_wb.R != wb->R || old_wb.G != wb->G || old_wb.B != wb->B)
        return FALSE;

    while ((wb->R > WB_IDENTITY) && (wb->G > WB_IDENTITY) && (wb->B > WB_IDENTITY))
    {
        wb->R -= 1;
        wb->G -= 1;
        wb->B -= 1;
    }

    rgb_tripel averageColor = average_color_cam( data);
    if (wb_auto_step(&averageColor, wb))
    {
        return TRUE;
    }

    wb->R = clip( wb->R, WB_MAX );
    wb->G = clip( wb->G, WB_MAX );
    wb->B = clip( wb->B, WB_MAX );

    GST_INFO("Calculated white balance R:%d G:%d B:%d", wb->R, wb->G, wb->B);

    return FALSE;
}


byte wb_pixel_c (byte pixel, byte wb_r, byte wb_g, byte wb_b, tBY8Pattern pattern)
{
    unsigned int val = pixel;
    switch (pattern)
    {
        case BG:
            val = (val * wb_b) / 64;
            break;
        case GB:
            val = (val * wb_g) / 64;
            break;
        case GR:
            val = (val * wb_g) / 64;
            break;
        case RG:
            val = (val * wb_r) / 64;
            break;
    };
    return ( val > 0xFF ? 0xFF : (byte)(val));
}


static void wb_line_c (byte* dest_line,
                       byte* src_line,
                       unsigned int dim_x,
                       byte wb_r, byte wb_g, byte wb_b,
                       tBY8Pattern pattern)
{
    const tBY8Pattern even_pattern = pattern;
    const tBY8Pattern odd_pattern = next_pixel(pattern);
    guint x;
    for (x = 0; x < dim_x; x += 2)
    {
        unsigned int v0 = wb_pixel_c( src_line[x], wb_r, wb_g, wb_b,even_pattern );
        unsigned int v1 = wb_pixel_c( src_line[x+1], wb_r, wb_g, wb_b, odd_pattern );
        *((guint16*)(dest_line + x)) = (guint16)(v1 << 8 | v0);
    }

    if (x == (dim_x - 1))
    {
        dest_line[x] = wb_pixel_c( src_line[x], wb_r, wb_g, wb_b, even_pattern );
    }
}


static void	wb_image_c (GstTcamWhitebalance* self, GstBuffer* buf, byte wb_r, byte wb_g, byte wb_b)
{
    GstMapInfo info;
    gst_buffer_make_writable(buf);

    gst_buffer_map(buf, &info, GST_MAP_WRITE);

    guint* data = (guint*)info.data;

    unsigned int dim_x = self->image_size.width;
    unsigned int dim_y = self->image_size.height;

    guint pitch = 8 * dim_x / 8;

    tBY8Pattern odd = next_line(self->pattern);

    guint y;
    for (y = 0 ; y < (dim_y - 1); y += 2)
    {
        byte* line0 = (byte*)data + y * pitch;
        byte* line1 = (byte*)data + (y + 1) * pitch;

        wb_line_c(line0, line0, dim_x, wb_r, wb_g, wb_b, self->pattern);
        wb_line_c(line1, line1, dim_x, wb_r, wb_g, wb_b, odd);
    }

    if (y == (dim_y - 1))
    {
        byte* line = (byte*)data + y * pitch;
        wb_line_c(line, line, dim_x, wb_r, wb_g, wb_b, self->pattern);
    }

    gst_buffer_unmap(buf, &info);
}


void apply_wb_by8_c ( GstTcamWhitebalance* self, GstBuffer* buf, byte wb_r, byte wb_g, byte wb_b)
{
    GST_DEBUG("Applying white balance with values: R:%d G:%d B:%d", wb_r, wb_g, wb_b);

    wb_image_c( self, buf, wb_r, wb_g, wb_b);
}


static void whitebalance_buffer (GstTcamWhitebalance* self, GstBuffer* buf)
{
    rgb_tripel rgb = self->rgb;

    /* we prefer to set our own values */
    if (self->auto_wb == FALSE)
    {
        rgb.R = self->red;
        rgb.G = self->green;
        rgb.B = self->blue;
    }
    else /* update the permanent values to represent the current adjustments */
    {
        auto_sample_points points = {};

        GstMapInfo info;

        gst_buffer_map(buf, &info, GST_MAP_READ);

        unsigned char* data = (unsigned char*)info.data;

        get_sampling_points(data, &points, self->pattern, self->image_size);
        gst_buffer_unmap(buf, &info);

        guint resulting_brightness = 0;
        auto_whitebalance(&points, &rgb, &resulting_brightness);

        self->red = rgb.R;
        self->green = rgb.G;
        self->blue = rgb.B;

        self->rgb = rgb;
    }

    if (self->res.color.has_whitebalance)
    {
        self->res.color.rgb = rgb;
        gst_tcamwhitebalance_device_set_whiteblance(self);
    }
    else
    {
        apply_wb_by8_c(self, buf, rgb.R, rgb.G, rgb.B);
    }
}


static void update_device_resources (struct device_resources* res)
{
    tcam::CaptureDevice* dev = nullptr;

    g_object_get(G_OBJECT(res->source_element), "camera", &dev, NULL);

    if (dev == nullptr)
    {
        GST_ERROR("Could not retrieve device. Aborting");
        return;
    }

    tcam::Property* prop = dev->get_property(TCAM_PROPERTY_EXPOSURE);

    if (prop == nullptr)
    {
        GST_ERROR("Exposure could not be found!");
    }
    else
    {
        struct tcam_device_property p = prop->get_struct();

        res->exposure.min = p.value.i.min;
        res->exposure.max = p.value.i.max;
        res->exposure.value = p.value.i.value;
    }

    prop = dev->get_property(TCAM_PROPERTY_GAIN);

    if (prop == nullptr)
    {
        GST_ERROR("Gain could not be found!");
    }
    else
    {
        struct tcam_device_property p = prop->get_struct();

        res->gain.min = p.value.i.min;
        res->gain.max = p.value.i.max;
        res->gain.value = p.value.i.value;
    }

    prop = dev->get_property(TCAM_PROPERTY_GAIN_RED);

    if (prop == nullptr)
    {
        GST_INFO("Gain Red could not be found!");
    }
    else
    {
        struct tcam_device_property p = prop->get_struct();
        res->color.rgb.R = p.value.i.value;
        res->exposure.max = p.value.i.max;
        // hardcoded from dfk72
        res->color.default_value = 36;
    }
    prop = dev->get_property(TCAM_PROPERTY_GAIN_GREEN);

    if (prop == nullptr)
    {
        GST_INFO("Gain Green could not be found!");
    }
    else
    {
        struct tcam_device_property p = prop->get_struct();
        res->color.rgb.G = p.value.i.value;
        res->exposure.max = p.value.i.max;
        // hardcoded from dfk72
        res->color.default_value = 36;
    }

    prop = dev->get_property(TCAM_PROPERTY_GAIN_BLUE);

    if (prop == nullptr)
    {
        GST_INFO("Gain Blue could not be found!");
    }
    else
    {
        struct tcam_device_property p = prop->get_struct();
        res->color.rgb.B = p.value.i.value;
        res->exposure.max = p.value.i.max;
        // hardcoded from dfk72
        res->color.default_value = 36;
    }


}


static struct device_resources find_source (GstElement* self)
{

    struct device_resources res = {};
    res.color.max = 255;

    /* if camera_src is not set we assume that the first default camera src found shall be used */

    res.source_element = tcam_gst_find_camera_src(self);

    if (res.source_element == nullptr)
    {
        GST_ERROR("Could not find source element");
    }

    update_device_resources(&res);

    return res;
}


static gboolean extract_resolution (GstTcamWhitebalance* self)
{

    GstPad* pad  = GST_BASE_TRANSFORM_SINK_PAD(self);
    GstCaps* caps = gst_pad_get_current_caps(pad);
    GstStructure *structure = gst_caps_get_structure (caps, 0);

    g_return_val_if_fail(gst_structure_get_int(structure, "width", &self->image_size.width), FALSE);
    g_return_val_if_fail(gst_structure_get_int(structure, "height", &self->image_size.height), FALSE);

    guint fourcc;

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
    else
    {
        GST_ERROR("Unable to determine bayer pattern.");
        return FALSE;
    }

    // we only handle bayer 8 bit -> 1 byte
    int bytes_per_pixel = 1;
    self->expected_buffer_size = self->image_size.height * self->image_size.width * bytes_per_pixel;

    self->res = find_source(GST_ELEMENT(self));

    return TRUE;
}


/* Entry point */
static GstFlowReturn gst_tcamwhitebalance_transform_ip (GstBaseTransform* trans, GstBuffer* buf)
{
    GstTcamWhitebalance* self = GST_TCAMWHITEBALANCE (trans);

    if (self->image_size.width == 0 || self->image_size.height == 0)
    {
        if (!extract_resolution(self))
        {
            GST_ERROR("Received format is not usable. Aborting");
            return GST_FLOW_ERROR;
        }

        if (self->force_hardware_wb)
        {
            self->res.color.has_whitebalance = TRUE;
        }

		if (self->res.color.has_whitebalance)
		{
			WB_MAX = self->res.color.max;
			WB_IDENTITY = self->res.color.default_value;

			init_wb_values(self);
		}
    }

    /* auto is completely disabled */
    if (!self->auto_enabled)
    {
        return GST_FLOW_OK;
    }

    // validity checks
    GstMapInfo info;

    gst_buffer_map(buf, &info, GST_MAP_READ);

    guint* data = (guint*)info.data;
    guint length = info.size;

    gst_buffer_unmap(buf, &info);


    if (data == NULL || length != self->expected_buffer_size)
    {
        GST_ERROR("Buffer is not valid! Ignoring buffer and trying to continue...");
        return GST_FLOW_OK;
    }

    whitebalance_buffer(self, buf);

    return GST_FLOW_OK;
}


static gboolean plugin_init (GstPlugin* plugin)
{
    return gst_element_register (plugin, "tcamwhitebalance", GST_RANK_NONE, GST_TYPE_TCAMWHITEBALANCE);
}

#ifndef VERSION
#define VERSION "0.0.1"
#endif
#ifndef PACKAGE
#define PACKAGE "tcamwhitebalance"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME tcamwhitebalance
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "https://github.com/TheImagingSource/tcamcamera"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
                   GST_VERSION_MINOR,
                   PACKAGE_NAME,
                   "The Imaging Source white balance plugin",
                   plugin_init, VERSION, "Proprietary", PACKAGE, GST_PACKAGE_ORIGIN)
