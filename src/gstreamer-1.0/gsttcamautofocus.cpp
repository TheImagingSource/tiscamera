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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include <stdbool.h>
#include <ctype.h>
#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include "gsttcamautofocus.h"

#include "tcam.h"
#include "tcamgstbase.h"
#include "tcamprop.h"

GST_DEBUG_CATEGORY_STATIC (gst_tcamautofocus_debug_category);
#define GST_CAT_DEFAULT gst_tcamautofocus_debug_category



enum
{
    PROP_0,
    PROP_AUTO,
    PROP_LEFT,
    PROP_TOP,
    PROP_WIDTH,
    PROP_HEIGHT,
};



/* prototypes */

static void gst_tcamautofocus_set_property (GObject* object,
                                            guint property_id,
                                            const GValue* value,
                                            GParamSpec* pspec);
static void gst_tcamautofocus_get_property (GObject* object,
                                            guint property_id,
                                            GValue* value,
                                            GParamSpec* pspec);
static void gst_tcamautofocus_finalize (GObject* object);

static GstFlowReturn gst_tcamautofocus_transform_ip (GstBaseTransform* trans,
                                                     GstBuffer* buf);
static GstCaps* gst_tcamautofocus_transform_caps (GstBaseTransform* trans,
                                                  GstPadDirection direction,
                                                  GstCaps* caps);

static void gst_tcamautofocus_fixate_caps (GstBaseTransform* base,
                                           GstPadDirection direction,
                                           GstCaps* caps,
                                           GstCaps* othercaps);



/* tcamprop interface*/

static GSList* gst_tcamautofocus_get_property_names (TcamProp* self);

static gchar *gst_tcamautofocus_get_property_type (TcamProp* self, gchar* name);

static gboolean gst_tcamautofocus_get_tcam_property (TcamProp* self,
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

static gboolean gst_tcamautofocus_set_tcam_property (TcamProp* self,
                                                     gchar* name,
                                                     const GValue* value);

static GSList* gst_tcamautofocus_get_tcam_menu_entries (TcamProp* self,
                                                        const gchar* name);

static GSList* gst_tcamautofocus_get_device_serials (TcamProp* self);

static gboolean gst_tcamautofocus_get_device_info (TcamProp* self,
                                                   const char* serial,
                                                   char** name,
                                                   char** identifier,
                                                   char** connection_type);

static void gst_tcamautofocus_prop_init (TcamPropInterface* iface)
{
    iface->get_property_names = gst_tcamautofocus_get_property_names;
    iface->get_property_type = gst_tcamautofocus_get_property_type;
    iface->get_property = gst_tcamautofocus_get_tcam_property;
    iface->get_menu_entries = gst_tcamautofocus_get_tcam_menu_entries;
    iface->set_property = gst_tcamautofocus_set_tcam_property;
    iface->get_device_serials = gst_tcamautofocus_get_device_serials;
    iface->get_device_info = gst_tcamautofocus_get_device_info;
}



G_DEFINE_TYPE_WITH_CODE (GstTcamAutoFocus,
                         gst_tcamautofocus,
                         GST_TYPE_BASE_TRANSFORM,
                         G_IMPLEMENT_INTERFACE(TCAM_TYPE_PROP, gst_tcamautofocus_prop_init));


static const char* tcamautofocus_property_id_to_string (guint id)
{
    switch (id)
    {
        case PROP_AUTO:
            return "Focus Auto";
        case PROP_LEFT:
            return "Focus ROI Left";
        case PROP_TOP:
            return "Focus ROI Top";
        case PROP_WIDTH:
            return "Focus ROI Width";
        case PROP_HEIGHT:
            return "Focus ROI Height";
        default:
            return 0;
    }
}


static guint tcamautofocus_string_to_property_id (const char* name)
{
    if (g_strcmp0(name, tcamautofocus_property_id_to_string(PROP_AUTO)) == 0)
    {
        return PROP_AUTO;
    }
    else if (g_strcmp0(name, tcamautofocus_property_id_to_string(PROP_LEFT)) == 0)
    {
        return PROP_LEFT;
    }
    else if (g_strcmp0(name, tcamautofocus_property_id_to_string(PROP_TOP)) == 0)
    {
        return PROP_TOP;
    }
    else if (g_strcmp0(name, tcamautofocus_property_id_to_string(PROP_WIDTH)) == 0)
    {
        return PROP_WIDTH;
    }
    else if (g_strcmp0(name, tcamautofocus_property_id_to_string(PROP_HEIGHT)) == 0)
    {
        return PROP_HEIGHT;
    }

    return 0;
}


static GSList* gst_tcamautofocus_get_property_names (TcamProp* self)
{
    GSList* names = nullptr;

    names = g_slist_append(names,
        g_strdup(tcamautofocus_property_id_to_string(PROP_AUTO)));
    names = g_slist_append(names,
        g_strdup(tcamautofocus_property_id_to_string(PROP_LEFT)));
    names = g_slist_append(names,
        g_strdup(tcamautofocus_property_id_to_string(PROP_TOP)));
    names = g_slist_append(names,
        g_strdup(tcamautofocus_property_id_to_string(PROP_WIDTH)));
    names = g_slist_append(names,
        g_strdup(tcamautofocus_property_id_to_string(PROP_HEIGHT)));

    return names;
}


static gchar* gst_tcamautofocus_get_property_type (TcamProp* self, gchar* name)
{
    if (g_strcmp0(name, tcamautofocus_property_id_to_string(PROP_AUTO)) == 0)
    {
        return g_strdup("button");
    }
    else if (g_strcmp0(name, tcamautofocus_property_id_to_string(PROP_LEFT)) == 0)
    {
        return g_strdup("integer");
    }
    else if (g_strcmp0(name, tcamautofocus_property_id_to_string(PROP_TOP)) == 0)
    {
        return g_strdup("integer");
    }
    else if (g_strcmp0(name, tcamautofocus_property_id_to_string(PROP_WIDTH)) == 0)
    {
        return g_strdup("integer");
    }
    else if (g_strcmp0(name, tcamautofocus_property_id_to_string(PROP_HEIGHT)) == 0)
    {
        return g_strdup("integer");
    }

    return nullptr;
}


static gboolean gst_tcamautofocus_get_tcam_property (TcamProp* prop,
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
    GstTcamAutoFocus* self = GST_TCAMAUTOFOCUS(prop);

    if (category)
    {
        g_value_init(category, G_TYPE_STRING);
        g_value_set_string(category, "Special");
    }
    if (group)
    {
        g_value_init(group, G_TYPE_STRING);
        g_value_set_string(group, "Focus");
    }

    if (g_strcmp0(name, tcamautofocus_property_id_to_string(PROP_AUTO)) == 0)
    {
        if (value)
        {
            g_value_init(value, G_TYPE_BOOLEAN);
            g_value_set_boolean(value, self->focus_active);
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
            g_value_init(step, G_TYPE_BOOLEAN);
        }
        if (flags)
        {
            g_value_init(flags, G_TYPE_INT);
            g_value_set_int(flags, 0);
        }
        if (type)
        {
            g_value_init(type, G_TYPE_STRING);
            g_value_set_string(type, gst_tcamautofocus_get_property_type(prop, name));
        }
        return TRUE;
    }
    else if (g_strcmp0(name, tcamautofocus_property_id_to_string(PROP_LEFT)) == 0)
    {
        if (value)
        {
            g_value_init(value, G_TYPE_INT);
            g_value_set_int(value, self->roi_left);
        }
        if (min)
        {
            g_value_init(min, G_TYPE_INT);
            g_value_set_int(min, 0);
        }
        if (max)
        {
            g_value_init(max, G_TYPE_INT);
            g_value_set_int(max, self->image_width);
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
            g_value_set_string(type, gst_tcamautofocus_get_property_type(prop, name));
        }
        return TRUE;
    }
    else if (g_strcmp0(name, tcamautofocus_property_id_to_string(PROP_TOP)) == 0)
    {
        if (value)
        {
            g_value_init(value, G_TYPE_INT);
            g_value_set_int(value, self->roi_top);
        }
        if (min)
        {
            g_value_init(min, G_TYPE_INT);
            g_value_set_int(min, 0);
        }
        if (max)
        {
            g_value_init(max, G_TYPE_INT);
            g_value_set_int(max, self->image_height);
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
            g_value_set_string(type, gst_tcamautofocus_get_property_type(prop, name));
        }
        return TRUE;
    }
    else if (g_strcmp0(name, tcamautofocus_property_id_to_string(PROP_WIDTH)) == 0)
    {
        if (value)
        {
            g_value_init(value, G_TYPE_INT);
            g_value_set_int(value, self->roi_width);
        }
        if (min)
        {
            g_value_init(min, G_TYPE_INT);
            g_value_set_int(min, 0);
        }
        if (max)
        {
            g_value_init(max, G_TYPE_INT);
            g_value_set_int(max, self->image_width);
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
            g_value_set_string(type, gst_tcamautofocus_get_property_type(prop, name));
        }
        return TRUE;
    }
    else if (g_strcmp0(name, tcamautofocus_property_id_to_string(PROP_HEIGHT)) == 0)
    {
        if (value)
        {
            g_value_init(value, G_TYPE_INT);
            g_value_set_int(value, self->roi_height);
        }
        if (min)
        {
            g_value_init(min, G_TYPE_INT);
            g_value_set_int(min, 0);
        }
        if (max)
        {
            g_value_init(max, G_TYPE_INT);
            g_value_set_int(max, self->image_height);
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
            g_value_set_string(type, gst_tcamautofocus_get_property_type(prop, name));
        }
        return TRUE;
    }
    return FALSE;
}


static gboolean gst_tcamautofocus_set_tcam_property (TcamProp* self,
                                                     gchar* name,
                                                     const GValue* value)
{
    guint id = tcamautofocus_string_to_property_id(name);

    if (id == 0)
    {
        return FALSE;
    }

    gst_tcamautofocus_set_property(G_OBJECT(self),
                                   tcamautofocus_string_to_property_id(name),
                                   value, NULL);
    return TRUE;
}


static GSList* gst_tcamautofocus_get_tcam_menu_entries (TcamProp* self,
                                                        const gchar* name)
{
    return nullptr;
}


static GSList* gst_tcamautofocus_get_device_serials (TcamProp* self)
{
    return FALSE;
}


static gboolean gst_tcamautofocus_get_device_info (TcamProp* self,
                                                   const char* serial,
                                                   char** name,
                                                   char** identifier,
                                                   char** connection_type)
{
    return FALSE;
}


static GstStaticPadTemplate gst_tcamautofocus_sink_template =
    GST_STATIC_PAD_TEMPLATE ("sink",
                             GST_PAD_SINK,
                             GST_PAD_ALWAYS,
                             GST_STATIC_CAPS ("ANY"));

static GstStaticPadTemplate gst_tcamautofocus_src_template =
    GST_STATIC_PAD_TEMPLATE ("src",
                             GST_PAD_SRC,
                             GST_PAD_ALWAYS,
                             GST_STATIC_CAPS ("ANY"));


static void focus_run_tcam (GstTcamAutoFocus* self)
{
    if (self->camera_src == NULL)
    {
        GST_ERROR("Source empty! Aborting.");
        return;
    }

    if (autofocus_is_running(self->focus))
    {
        return;
    }

    tcam::CaptureDevice* dev = nullptr;
    g_object_get (G_OBJECT (self->camera_src), "camera", &dev, NULL);

    if (dev == nullptr)
    {
        GST_ERROR("Unable to retrieve camera! Aborting.");
        return;
    }

    RECT r = {0, 0, 0, 0};

    /* user defined rectangle */
    if (self->roi_width != 0 && self->roi_height != 0)
    {
        r.left = self->roi_left;
        r.right = self->roi_left + self->roi_width;
        r.top = self->roi_left;
        r.bottom = self->roi_left + self->roi_height;
    }

    tcam::Property* p = dev->get_property(TCAM_PROPERTY_FOCUS);

    if (p == nullptr)
    {
        GST_ERROR("Unable to retrieve focus property! Aborting.");
        return;
    }
    struct tcam_device_property prop = p->get_struct();

    self->cur_focus = prop.value.i.value;
    int min = prop.value.i.min;
    int max = prop.value.i.max;

    /* magic number */
    int focus_auto_step_divisor = 4;

    GST_INFO("Callig autofocus_run with: Focus %d Min %d Max %d Divisor %d ",
             self->cur_focus,
             min,
             max,
             focus_auto_step_divisor);

    autofocus_run(self->focus,
                  self->cur_focus,
                  min,
                  max,
                  r,
                  500,
                  focus_auto_step_divisor,
                  false);
}


static void focus_run (GstTcamAutoFocus* self)
{
    focus_run_tcam(self);
}


static void gst_tcamautofocus_class_init (GstTcamAutoFocusClass* klass)
{
    GObjectClass* gobject_class = G_OBJECT_CLASS (klass);
    GstBaseTransformClass* base_transform_class = GST_BASE_TRANSFORM_CLASS (klass);

    gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
                                        gst_static_pad_template_get(&gst_tcamautofocus_src_template));
    gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
                                        gst_static_pad_template_get(&gst_tcamautofocus_sink_template));

    gst_element_class_set_details_simple (GST_ELEMENT_CLASS(klass),
                                          "The Imaging Source auto focus Element",
                                          "Generic",
                                          "Adjusts the image focus by setting camera properties.",
                                          "The Imaging Source Europe GmbH <support@theimagingsource.com>");

    gobject_class->set_property = gst_tcamautofocus_set_property;
    gobject_class->get_property = gst_tcamautofocus_get_property;
    gobject_class->finalize = gst_tcamautofocus_finalize;
    base_transform_class->transform_ip = GST_DEBUG_FUNCPTR (gst_tcamautofocus_transform_ip);

    g_object_class_install_property(gobject_class,
                                    PROP_AUTO,
                                    g_param_spec_boolean("auto",
                                                         "Activate auto focus run",
                                                         "Automatically adjust camera focus",
                                                         FALSE,
                                                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    g_object_class_install_property(gobject_class,
                                    PROP_LEFT,
                                    g_param_spec_int("left",
                                                     "Left border of the focus region",
                                                     "Coordinate for focus region.",
                                                     0, G_MAXINT, 0,
                                                     G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    g_object_class_install_property(gobject_class,
                                    PROP_TOP,
                                    g_param_spec_int("top",
                                                     "Top border of the focus region",
                                                     "Coordinate for focus region.",
                                                     0, G_MAXINT, 0,
                                                     G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    g_object_class_install_property(gobject_class,
                                    PROP_WIDTH,
                                    g_param_spec_int("width",
                                                     "Width of focus region",
                                                     "Width of the focus region beginning at 'left'",
                                                     0, G_MAXINT, 0,
                                                     G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    g_object_class_install_property(gobject_class,
                                    PROP_HEIGHT,
                                    g_param_spec_int("height",
                                                     "Height of focus region",
                                                     "Height of the focus region beginning at 'top'.",
                                                     0, G_MAXINT, 0,
                                                     G_PARAM_READWRITE | G_PARAM_CONSTRUCT));


    GST_DEBUG_CATEGORY_INIT (gst_tcamautofocus_debug_category,
                             "tcamautofocus",
                             0,
                             "debug category for tcamautofocus element");

}


static void gst_tcamautofocus_init (GstTcamAutoFocus *self)
{
    self->focus = autofocus_create();
    self->cur_focus = 0;
    self->roi_left = 0;
    self->roi_left = 0;
    self->roi_width = 0;
    self->roi_height = 0;
    self->image_width = 0;
    self->image_height = 0;
    self->camera_src = NULL;
}


void gst_tcamautofocus_set_property (GObject* object,
                                     guint property_id,
                                     const GValue* value,
                                     GParamSpec* pspec)
{
    GstTcamAutoFocus* self = GST_TCAMAUTOFOCUS (object);

    switch (property_id)
    {
        case PROP_AUTO:
            self->focus_active = g_value_get_boolean (value);
            if (self->focus_active == TRUE)
            {
                focus_run(self);
            }
            break;
        case PROP_LEFT:
            self->roi_left = g_value_get_int(value);
            break;
        case PROP_TOP:
            self->roi_top = g_value_get_int(value);
            break;
        case PROP_WIDTH:
            self->roi_width = g_value_get_int(value);
            break;
        case PROP_HEIGHT:
            self->roi_height = g_value_get_int(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}


void gst_tcamautofocus_get_property (GObject* object,
                                     guint property_id,
                                     GValue* value,
                                     GParamSpec* pspec)
{
    GstTcamAutoFocus* self = GST_TCAMAUTOFOCUS (object);

    switch (property_id)
    {
        case PROP_AUTO:
            g_value_set_boolean(value, self->focus_active);
            break;
        case PROP_LEFT:
            g_value_set_int(value, self->roi_left);
            break;
        case PROP_TOP:
            g_value_set_int(value, self->roi_left);
            break;
        case PROP_WIDTH:
            g_value_set_int(value, self->roi_width);
            break;
        case PROP_HEIGHT:
            g_value_set_int(value, self->roi_height);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}


void gst_tcamautofocus_finalize (GObject* object)
{
    GstTcamAutoFocus* self = GST_TCAMAUTOFOCUS (object);

    autofocus_destroy(self->focus);
    G_OBJECT_CLASS (gst_tcamautofocus_parent_class)->finalize (object);
}


static GstCaps* gst_tcamautofocus_transform_caps (GstBaseTransform* trans,
                                                  GstPadDirection direction,
                                                  GstCaps* caps)
{
    GstTcamAutoFocus* self = GST_TCAMAUTOFOCUS(trans);
    GstCaps *outcaps = gst_caps_copy (caps);

    if (self->camera_src != NULL)
    {
        return outcaps;
    }
    /* if camera_src is not set we assume that the first default camera src found shall be used */

    return outcaps;
}


static void gst_tcamautofocus_fixate_caps (GstBaseTransform* base,
                                           GstPadDirection direction,
                                           GstCaps* incoming,
                                           GstCaps* outgoing)
{
    GstTcamAutoFocus* self = GST_TCAMAUTOFOCUS (base);

    GstStructure* ins;
    GstStructure* outs;
    gint width, height;
    uint32_t fourcc;
    g_return_if_fail (gst_caps_is_fixed (incoming));

    GST_DEBUG_OBJECT (base, "trying to fixate outgoing %" GST_PTR_FORMAT
                      " based on caps %" GST_PTR_FORMAT, outgoing, incoming);

    ins = gst_caps_get_structure (incoming, 0);
    outs = gst_caps_get_structure (outgoing, 0);

    if (gst_structure_get_int (ins, "width", &width))
    {
        if (gst_structure_has_field (outs, "width"))
        {
            gst_structure_fixate_field_nearest_int (outs, "width", width);
        }
        self->image_width = width;
    }

    if (gst_structure_get_int (ins, "height", &height))
    {
        if (gst_structure_has_field (outs, "height"))
        {
            gst_structure_fixate_field_nearest_int (outs, "height", height);
        }
        self->image_height = height;
    }

    if (gst_structure_get_field_type (ins, "format") == G_TYPE_STRING)
    {
        const char* string;

        string = gst_structure_get_string (ins, "format");
        fourcc = GST_STR_FOURCC (string);
    }
    else
        fourcc = 0;

    if (fourcc == 0)
    {
        GST_WARNING("Unable to determine fourcc. Using Y800");
        fourcc = FOURCC_BY8;
    }
    else
    {
        unsigned char bytes[4];

        bytes[0] = (fourcc >> 24) & 0xFF;
        bytes[1] = (fourcc >> 16) & 0xFF;
        bytes[2] = (fourcc >> 8) & 0xFF;
        bytes[3] = fourcc & 0xFF;

        fourcc = GST_MAKE_FOURCC (toupper(bytes[0]),
                                  toupper(bytes[1]),
                                  toupper(bytes[2]),
                                  toupper(bytes[3]));

    }
}


static int clip (int min, int value, int max)
{
    if (min > value)
        return min;
    if (max < value)
        return max;
    return value;
}


static void transform_tcam (GstTcamAutoFocus* self, GstBuffer* buf)
{
    if (self->camera_src == nullptr)
    {
        self->camera_src = tcam_gst_find_camera_src(GST_ELEMENT(self));
    }

    tcam::CaptureDevice* dev;
    g_object_get (G_OBJECT (self->camera_src), "camera", &dev, NULL);

    gint64 min = 0;
    gint64 max = 0;

    tcam::Property* focus_prop = dev->get_property(TCAM_PROPERTY_FOCUS);
    struct tcam_device_property prop = focus_prop->get_struct();

    int focus_auto_min = prop.value.i.min;
    max = prop.value.i.max;

    /* assure we use the current focus value */
    autofocus_update_focus(self->focus, clip(focus_auto_min, self->cur_focus, max));

    GstMapInfo info = {};
    gst_buffer_make_writable(buf);

    gst_buffer_map(buf, &info, GST_MAP_WRITE);

    img_descriptor img =
        {
            info.data,
            info.size,
            FOURCC_GRBG, /* TODO: DYNAMICALLY FIND FORMAT */
            self->image_width,
            self->image_height,
            self->image_width
        };

    int new_focus_value;
    POINT p = {0, 0};

    bool ret = autofocus_analyze_frame(self->focus,
                                       img,
                                       p,
                                       500,
                                       &new_focus_value);

    if (ret)
    {
        GST_DEBUG("Setting focus %d", new_focus_value);

        focus_prop->set_value((int64_t)new_focus_value);
        self->cur_focus = new_focus_value;
    }

    gst_buffer_unmap(buf, &info);
}


gboolean find_image_values (GstTcamAutoFocus* self)
{
    GstPad* pad  = GST_BASE_TRANSFORM_SINK_PAD(self);
    GstCaps* caps = gst_pad_get_current_caps(pad);
    GstStructure *structure = gst_caps_get_structure (caps, 0);

    g_return_val_if_fail (gst_structure_get_int (structure, "width", &self->image_width), FALSE);
    g_return_val_if_fail (gst_structure_get_int (structure, "height", &self->image_height), FALSE);

    gst_structure_get_fraction(structure, "framerate", &self->framerate_numerator, &self->framerate_denominator);

    return TRUE;
}


static GstFlowReturn gst_tcamautofocus_analyze_buffer (GstTcamAutoFocus* self, GstBuffer* buf)
{
    // validity checks
    GstMapInfo info;

    gst_buffer_map(buf, &info, GST_MAP_READ);

    guint* data = (guint*)info.data;
    guint length = info.size;

    if (data == NULL || length == 0)
    {
        GST_ERROR("Buffer is not valid! Ignoring buffer and trying to continue...");
        return GST_FLOW_OK;
    }

    GST_DEBUG("transform_tcam");
    transform_tcam(self, buf);

    gst_buffer_unmap(buf, &info);

    return GST_FLOW_OK;
}


static GstFlowReturn gst_tcamautofocus_transform_ip (GstBaseTransform* trans, GstBuffer* buf)
{
    GstTcamAutoFocus* self = GST_TCAMAUTOFOCUS (trans);

    if (self->image_width == 0 || self->image_height == 0)
    {
        if (!find_image_values(self))
        {
            return GST_FLOW_ERROR;
        }
    }

    if (autofocus_is_running(self->focus))
    {
        if (self->camera_src == NULL)
        {
            self->camera_src = tcam_gst_find_camera_src(GST_ELEMENT(self));
        }
        find_image_values(self);

        return gst_tcamautofocus_analyze_buffer(self, buf);
    }
    else if (self->focus_active) // entered if set to true with gst-launch
    {
        find_image_values(self);
        self->focus_active = FALSE;
        focus_run(self);

        return gst_tcamautofocus_analyze_buffer(self, buf);
    }

    autofocus_end(self->focus);
    self->focus_active = FALSE;
    return GST_FLOW_OK;
}


static gboolean plugin_init (GstPlugin* plugin)
{
    return gst_element_register(plugin,
                                "tcamautofocus",
                                GST_RANK_NONE,
                                GST_TYPE_TCAMAUTOFOCUS);
}


#ifndef VERSION
#define VERSION "0.0.1"
#endif
#ifndef PACKAGE
#define PACKAGE "tcamautofocus"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME tcamautofocus
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "https://github.com/TheImagingSource/tiscamera"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
                   GST_VERSION_MINOR,
                   PACKAGE_NAME,
                   "The Imaging Source auto exposure plugin",
                   plugin_init,
                   VERSION,
                   "Proprietary",
                   PACKAGE, GST_PACKAGE_ORIGIN)
