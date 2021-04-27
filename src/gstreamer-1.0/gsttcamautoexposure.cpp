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

#include "base_types.h"
#include "gsttcamautoexposure.h"
#include "tcam.h"
#include "tcamgstbase.h"
#include "tcamprop.h"
#include "tcamprop_impl_helper.h"

#include <cmath>
#include <gst/base/gstbasetransform.h>
#include <gst/gst.h>
#include <gst/video/video.h>
#include <stdlib.h>

using namespace tcam;
using namespace tcam::algorithms::roi;


GST_DEBUG_CATEGORY_STATIC(gst_tcamautoexposure_debug_category);
#define GST_CAT_DEFAULT gst_tcamautoexposure_debug_category

// The algorithm works with integers
// floating point gain values will be multiplied
// to allow handling of this
static const gdouble GAIN_FLOAT_MULTIPLIER = 1000;

/* prototypes */

static void set_exposure(GstTcamautoexposure* self, gdouble exposure);

static void set_gain(GstTcamautoexposure* self, gdouble gain);

static void set_iris(GstTcamautoexposure* self, int iris);

static gboolean gst_tcamautoexposure_set_caps(GstBaseTransform* trans,
                                              GstCaps* incaps,
                                              GstCaps* outcaps);

static void init_camera_resources(GstTcamautoexposure* self);
static gboolean find_image_values(GstTcamautoexposure* self);


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

static void gst_tcamautoexposure_set_property(GObject* object,
                                              guint property_id,
                                              const GValue* value,
                                              GParamSpec* pspec);
static void gst_tcamautoexposure_get_property(GObject* object,
                                              guint property_id,
                                              GValue* value,
                                              GParamSpec* pspec);
static void gst_tcamautoexposure_finalize(GObject* object);


static GstStateChangeReturn gst_tcamautoexposure_change_state(GstElement* element,
                                                              GstStateChange trans);

static GstFlowReturn gst_tcamautoexposure_transform_ip(GstBaseTransform* trans, GstBuffer* buf);

static GSList* gst_tcamautoexposure_get_property_names(TcamProp* self);

static gchar* gst_tcamautoexposure_get_property_type(TcamProp* self, const gchar* name);

static gboolean gst_tcamautoexposure_get_tcam_property(TcamProp* self,
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

static gboolean gst_tcamautoexposure_set_tcam_property(TcamProp* self,
                                                       const gchar* name,
                                                       const GValue* value);

static GSList* gst_tcamautoexposure_get_tcam_menu_entries(TcamProp* self, const gchar* name);

static GSList* gst_tcamautoexposure_get_device_serials(TcamProp* self);

static gboolean gst_tcamautoexposure_get_device_info(TcamProp* self,
                                                     const char* serial,
                                                     char** name,
                                                     char** identifier,
                                                     char** connection_type);

static void gst_tcamautoexposure_prop_init(TcamPropInterface* iface)
{
    iface->get_tcam_property_names = gst_tcamautoexposure_get_property_names;
    iface->get_tcam_property_type = gst_tcamautoexposure_get_property_type;
    iface->get_tcam_property = gst_tcamautoexposure_get_tcam_property;
    iface->get_tcam_menu_entries = gst_tcamautoexposure_get_tcam_menu_entries;
    iface->set_tcam_property = gst_tcamautoexposure_set_tcam_property;
    iface->get_tcam_device_serials = gst_tcamautoexposure_get_device_serials;
    iface->get_tcam_device_info = gst_tcamautoexposure_get_device_info;
}


G_DEFINE_TYPE_WITH_CODE(GstTcamautoexposure,
                        gst_tcamautoexposure,
                        GST_TYPE_BASE_TRANSFORM,
                        G_IMPLEMENT_INTERFACE(TCAM_TYPE_PROP, gst_tcamautoexposure_prop_init))


using namespace tcamprop_impl_helper;

static const prop_entry tcamautoexposure_properties[] = {
    { PROP_AUTO_EXPOSURE, "Exposure Auto", prop_types::boolean, "Exposure", "Exposure" },
    { PROP_AUTO_GAIN, "Gain Auto", prop_types::boolean, "Exposure", "Gain" },
    { PROP_AUTO_IRIS, "Iris Auto", prop_types::boolean, "Lens", "Iris" },
    { PROP_BRIGHTNESS_REFERENCE,
      "Brightness Reference",
      prop_types::integer,
      "Exposure",
      "Exposure" },
    { PROP_EXPOSURE_MIN, "Exposure Min", prop_types::integer, "Exposure", "Exposure" },
    { PROP_EXPOSURE_MAX, "Exposure Max", prop_types::integer, "Exposure", "Exposure" },
    { PROP_GAIN_MIN, "Gain Min", prop_types::real, "Exposure", "Gain" },
    { PROP_GAIN_MAX, "Gain Max", prop_types::real, "Exposure", "Gain" },
    { PROP_IRIS_MIN, "Iris Min", prop_types::integer, "Lens", "Iris" },
    { PROP_IRIS_MAX, "Iris Max", prop_types::integer, "Lens", "Iris" },
    { PROP_ROI_LEFT, "Exposure ROI Left", prop_types::integer, "Exposure", "ROI" },
    { PROP_ROI_WIDTH, "Exposure ROI Width", prop_types::integer, "Exposure", "ROI" },
    { PROP_ROI_TOP, "Exposure ROI Top", prop_types::integer, "Exposure", "ROI" },
    { PROP_ROI_HEIGHT, "Exposure ROI Height", prop_types::integer, "Exposure", "ROI" },
};


static const prop_entry* find_tcamautoexposure_property_entry(guint id)
{
    for (const auto& e : tcamautoexposure_properties)
    {
        if (e.prop_id == id)
        {
            return &e;
        }
    }
    return nullptr;
}

static const prop_entry* find_tcamautoexposure_property_entry(const char* str)
{
    for (const auto& e : tcamautoexposure_properties)
    {
        if (g_strcmp0(e.prop_name, str) == 0)
        {
            return &e;
        }
    }
    return nullptr;
}


static gboolean gst_tcamautoexposure_is_active_property(GstTcamautoexposure* self,
                                                        const std::string& name)
{
    if (name == "Exposure Auto" || name == "Exposure Min" || name == "Exposure Max")
    {
        return !self->exposure_name.empty();
    }

    if (name == "Gain Auto" || name == "Gain Min" || name == "Gain Max")
    {
        return !self->gain_name.empty();
    }

    if (name == "Iris Auto" || name == "Iris Min" || name == "Iris Max")
    {
        return !self->iris_name.empty();
    }

    return !self->exposure_name.empty() || !self->gain_name.empty();
}


static GSList* gst_tcamautoexposure_get_property_names(TcamProp* self)
{
    GstTcamautoexposure* element = GST_TCAMAUTOEXPOSURE(self);

    if (element->exposure_name.empty() && element->gain_name.empty())
    {
        return nullptr;
    }

    auto append = [](GSList* list, guint id) {
        auto prop = find_tcamautoexposure_property_entry(id);
        if (prop)
        {
            return g_slist_append(list, g_strdup(prop->prop_name));
        }
        else
        {
            return list;
        }
    };

    GSList* names = nullptr;
    names = append(names, PROP_BRIGHTNESS_REFERENCE);
    if (!element->exposure_name.empty())
    {
        names = append(names, PROP_AUTO_EXPOSURE);
        names = append(names, PROP_EXPOSURE_MIN);
        names = append(names, PROP_EXPOSURE_MAX);
    }
    if (!element->gain_name.empty())
    {
        names = append(names, PROP_AUTO_GAIN);
        names = append(names, PROP_GAIN_MIN);
        names = append(names, PROP_GAIN_MAX);
    }
    if (!element->iris_name.empty())
    {
        names = append(names, PROP_AUTO_IRIS);
        names = append(names, PROP_IRIS_MIN);
        names = append(names, PROP_IRIS_MAX);
    }
    names = append(names, PROP_ROI_LEFT);
    names = append(names, PROP_ROI_WIDTH);
    names = append(names, PROP_ROI_TOP);
    names = append(names, PROP_ROI_HEIGHT);
    return names;
}


static gchar* gst_tcamautoexposure_get_property_type(TcamProp* self __attribute__((unused)),
                                                     const gchar* name)
{
    if (name == nullptr)
    {
        GST_ERROR("Name is empty");
        return nullptr;
    }

    if (!gst_tcamautoexposure_is_active_property(GST_TCAMAUTOEXPOSURE(self), name))
    {
        return nullptr;
    }

    const auto* entry = find_tcamautoexposure_property_entry(name);
    if (entry == nullptr)
    {
        return nullptr;
    }

    return strdup(to_string(entry->type));
}

static gboolean gst_tcamautoexposure_get_tcam_property(TcamProp* prop,
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

    if (!gst_tcamautoexposure_is_active_property(self, name))
    {
        return FALSE;
    }

    const auto* entry = find_tcamautoexposure_property_entry(name);
    if (entry == nullptr)
    {
        return FALSE;
    }

    fill_gvalue(type, entry->type);
    fill_int(flags, 0);
    fill_string(category, entry->category);
    fill_string(group, entry->group);

    if (entry->prop_id == PROP_AUTO_EXPOSURE)
    {
        fill_bool(value, self->auto_exposure);
        fill_bool(min, false);
        fill_bool(max, true);
        fill_bool(def, true);
        fill_int(step, 1);
        return TRUE;
    }
    else if (entry->prop_id == PROP_AUTO_GAIN)
    {
        fill_bool(value, self->auto_gain);
        fill_bool(min, false);
        fill_bool(max, true);
        fill_bool(def, true);
        fill_int(step, 1);
        return TRUE;
    }
    else if (entry->prop_id == PROP_AUTO_IRIS)
    {
        fill_bool(value, self->auto_iris);
        fill_bool(min, false);
        fill_bool(max, true);
        fill_bool(def, true);
        fill_int(step, 1);
        return TRUE;
    }
    else if (entry->prop_id == PROP_BRIGHTNESS_REFERENCE)
    {
        fill_int(value, self->brightness_reference);
        fill_int(min, 0);
        fill_int(max, 255);
        fill_int(def, 128);
        fill_int(step, 1);
        return TRUE;
    }
    else if (entry->prop_id == PROP_EXPOSURE_MIN)
    {
        fill_int(value, self->exposure_min);
        fill_int(min, self->exposure.min);
        fill_int(max, self->exposure.max);
        fill_int(def, self->exposure.min);
        fill_int(step, self->exposure.step);
        return TRUE;
    }
    else if (entry->prop_id == PROP_EXPOSURE_MAX)
    {
        fill_int(value, self->exposure_max);
        fill_int(min, self->exposure.min);
        fill_int(max, self->exposure.max);
        fill_int(def, self->exposure.max);
        fill_int(step, self->exposure.step);
        return TRUE;
    }
    else if (entry->prop_id == PROP_GAIN_MIN)
    {
        if (self->gain_is_double)
        {
            fill_double(value, self->gain_min / GAIN_FLOAT_MULTIPLIER);
            fill_double(min, self->gain.min / GAIN_FLOAT_MULTIPLIER);
            fill_double(max, self->gain.max / GAIN_FLOAT_MULTIPLIER);
            fill_double(def, self->gain.min / GAIN_FLOAT_MULTIPLIER);
            fill_double(step, 0.1);
        }
        else
        {
            fill_double(value, self->gain_min);
            fill_double(min, self->gain.min);
            fill_double(max, self->gain.max);
            fill_double(def, self->gain.min);
            fill_double(step, 1);
        }
        return TRUE;
    }
    else if (entry->prop_id == PROP_GAIN_MAX)
    {
        if (self->gain_is_double)
        {
            fill_double(value, self->gain_max / GAIN_FLOAT_MULTIPLIER);
            fill_double(min, self->gain.min / GAIN_FLOAT_MULTIPLIER);
            fill_double(max, self->gain.max / GAIN_FLOAT_MULTIPLIER);
            fill_double(def, self->gain.max / GAIN_FLOAT_MULTIPLIER);
            fill_double(step, 0.1);
        }
        else
        {
            fill_double(value, self->gain_max);
            fill_double(min, self->gain.min);
            fill_double(max, self->gain.max);
            fill_double(def, self->gain.max);
            fill_double(step, 1);
        }
        return TRUE;
    }
    else if (entry->prop_id == PROP_IRIS_MIN)
    {
        fill_int(value, self->iris_min);
        fill_int(min, self->iris.min);
        fill_int(max, self->iris.max);
        fill_int(def, self->iris.min);
        fill_int(step, 1);
        return TRUE;
    }
    else if (entry->prop_id == PROP_IRIS_MAX)
    {
        fill_int(value, self->iris_max);
        fill_int(min, self->iris.min);
        fill_int(max, self->iris.max);
        fill_int(def, self->iris.max);
        fill_int(step, 1);
        return TRUE;
    }
    else if (entry->prop_id == PROP_ROI_LEFT)
    {
        fill_int(value, roi_left(self->roi));
        fill_int(min, 0);
        fill_int(max, self->image_size.width - (SAMPLING_MIN_WIDTH + 1));
        fill_int(def, 0);
        fill_int(step, 1);
        return TRUE;
    }
    else if (entry->prop_id == PROP_ROI_WIDTH)
    {
        fill_int(value, roi_width(self->roi));
        fill_int(min, SAMPLING_MIN_WIDTH);
        fill_int(max, self->image_size.width);
        fill_int(def, self->image_size.width);
        fill_int(step, 1);
        return TRUE;
    }
    else if (entry->prop_id == PROP_ROI_TOP)
    {
        fill_int(value, roi_top(self->roi));
        fill_int(min, 0);
        fill_int(max, self->image_size.height - (SAMPLING_MIN_HEIGHT + 1));
        fill_int(def, 0);
        fill_int(step, 1);
        return TRUE;
    }
    else if (entry->prop_id == PROP_ROI_HEIGHT)
    {
        fill_int(value, roi_height(self->roi));
        fill_int(min, SAMPLING_MIN_HEIGHT);
        fill_int(max, self->image_size.height);
        fill_int(def, self->image_size.height);
        fill_int(step, 1);
        return TRUE;
    }

    return FALSE;
}


static gboolean gst_tcamautoexposure_set_tcam_property(TcamProp* self,
                                                       const gchar* name,
                                                       const GValue* value)
{
    auto entry = find_tcamautoexposure_property_entry(name);
    if (entry == nullptr)
    {
        return FALSE;
    }

    if (!gst_tcamautoexposure_is_active_property(GST_TCAMAUTOEXPOSURE(self), name))
    {
        return FALSE;
    }


    gst_tcamautoexposure_set_property(G_OBJECT(self), entry->prop_id, value, NULL);
    return TRUE;
}

static GSList* gst_tcamautoexposure_get_tcam_menu_entries(TcamProp* self __attribute__((unused)),
                                                          const gchar* name __attribute__((unused)))
{
    GSList* ret = nullptr;

    return ret;
}


static GSList* gst_tcamautoexposure_get_device_serials(TcamProp* self __attribute__((unused)))
{
    return nullptr;
}


static gboolean gst_tcamautoexposure_get_device_info(TcamProp* self __attribute__((unused)),
                                                     const char* serial __attribute__((unused)),
                                                     char** name __attribute__((unused)),
                                                     char** identifier __attribute__((unused)),
                                                     char** connection_type __attribute__((unused)))
{
    return FALSE;
}
/* pad templates */

#define VIDEO_CAPS                                                \
    GST_VIDEO_CAPS_MAKE("{GRAY8, GRAY16_LE}")                     \
    ";"                                                           \
    "video/x-bayer,format={rggb,bggr,gbrg,grbg},"                 \
    "width=" GST_VIDEO_SIZE_RANGE ",height=" GST_VIDEO_SIZE_RANGE \
    ",framerate=" GST_VIDEO_FPS_RANGE


/* class initialization */

static void gst_tcamautoexposure_class_init(GstTcamautoexposureClass* klass)
{
    GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
    GstBaseTransformClass* base_transform_class = GST_BASE_TRANSFORM_CLASS(klass);

    gst_element_class_add_pad_template(
        GST_ELEMENT_CLASS(klass),
        gst_pad_template_new("src", GST_PAD_SRC, GST_PAD_ALWAYS, gst_caps_from_string(VIDEO_CAPS)));
    gst_element_class_add_pad_template(
        GST_ELEMENT_CLASS(klass),
        gst_pad_template_new(
            "sink", GST_PAD_SINK, GST_PAD_ALWAYS, gst_caps_from_string(VIDEO_CAPS)));

    gst_element_class_set_details_simple(
        GST_ELEMENT_CLASS(klass),
        "The Imaging Source Auto Exposure Element",
        "Generic",
        "Adjusts the image brightness by setting camera properties.",
        "The Imaging Source Europe GmbH <support@theimagingsource.com>");

    GST_DEBUG_CATEGORY_INIT(
        gst_tcamautoexposure_debug_category, "tcamautoexposure", 0, "tcam autoexposure");

    GST_ELEMENT_CLASS(klass)->change_state = gst_tcamautoexposure_change_state;
    gobject_class->set_property = gst_tcamautoexposure_set_property;
    gobject_class->get_property = gst_tcamautoexposure_get_property;
    gobject_class->finalize = gst_tcamautoexposure_finalize;

    base_transform_class->transform_ip = gst_tcamautoexposure_transform_ip;
    base_transform_class->set_caps = GST_DEBUG_FUNCPTR(gst_tcamautoexposure_set_caps);

    g_object_class_install_property(
        gobject_class,
        PROP_AUTO_EXPOSURE,
        g_param_spec_boolean("auto-exposure",
                             "Auto Exposure",
                             "Automatically adjust exposure",
                             TRUE,
                             static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));
    g_object_class_install_property(
        gobject_class,
        PROP_AUTO_GAIN,
        g_param_spec_boolean("auto-gain",
                             "Auto Gain",
                             "Automatically adjust gain",
                             TRUE,
                             static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));
    g_object_class_install_property(
        gobject_class,
        PROP_AUTO_GAIN,
        g_param_spec_boolean("auto-iris",
                             "Auto Iris",
                             "Automatically adjust the iris, if camera allows adjustments",
                             TRUE,
                             static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));
    g_object_class_install_property(
        gobject_class,
        PROP_EXPOSURE_MAX,
        g_param_spec_int("exposure-max",
                         "Exposure Maximum",
                         "Maximum value exposure can take",
                         0,
                         G_MAXINT,
                         G_MAXINT,
                         static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));
    g_object_class_install_property(
        gobject_class,
        PROP_EXPOSURE_MIN,
        g_param_spec_int("exposure-min",
                         "Exposure Minimum",
                         "Minimum value exposure can take",
                         0,
                         G_MAXINT,
                         0,
                         static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));

    g_object_class_install_property(
        gobject_class,
        PROP_GAIN_MAX,
        g_param_spec_double("gain-max",
                            "Gain Maximum",
                            "Maximum value gain can take",
                            0.0,
                            G_MAXDOUBLE,
                            G_MAXDOUBLE,
                            static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));
    g_object_class_install_property(
        gobject_class,
        PROP_GAIN_MIN,
        g_param_spec_double("gain-min",
                            "Gain Minimum",
                            "Minimum value gain can take",
                            0.0,
                            G_MAXDOUBLE,
                            0.0,
                            static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));

    g_object_class_install_property(
        gobject_class,
        PROP_IRIS_MAX,
        g_param_spec_int("iris-max",
                         "Iris Maximum",
                         "Maximum value the iris can take",
                         0,
                         G_MAXINT,
                         G_MAXINT,
                         static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));
    g_object_class_install_property(
        gobject_class,
        PROP_IRIS_MIN,
        g_param_spec_int("iris-min",
                         "Iris Minimum",
                         "Minimum value the iris can take",
                         0,
                         G_MAXINT,
                         0,
                         static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));

    g_object_class_install_property(
        gobject_class,
        PROP_BRIGHTNESS_REFERENCE,
        g_param_spec_int("brightness-reference",
                         "Brightness Reference",
                         "Ideal average brightness of buffer",
                         0,
                         255,
                         128,
                         static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));
    g_object_class_install_property(
        gobject_class,
        PROP_ROI_LEFT,
        g_param_spec_int("left",
                         "Left boundary of ROI",
                         "Left boundary of the region of interest",
                         0,
                         G_MAXINT,
                         0,
                         static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));

    g_object_class_install_property(
        gobject_class,
        PROP_ROI_TOP,
        g_param_spec_int("top",
                         "Top boundary of ROI",
                         "Top boundary of the region of interest",
                         0,
                         G_MAXINT,
                         0,
                         static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));

    g_object_class_install_property(
        gobject_class,
        PROP_ROI_WIDTH,
        g_param_spec_int("width",
                         "Width of ROI starting at 'left'",
                         "Width of the region of interest",
                         0,
                         G_MAXINT,
                         0,
                         static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));

    g_object_class_install_property(
        gobject_class,
        PROP_ROI_HEIGHT,
        g_param_spec_int("height",
                         "Lower, right boundary starting at 'top'",
                         "Height of the region of interest",
                         0,
                         G_MAXINT,
                         0,
                         static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));

    g_object_class_install_property(
        gobject_class,
        PROP_CAMERA,
        g_param_spec_object("camera",
                            "camera gst element",
                            "Gstreamer element that shall be manipulated",
                            GST_TYPE_ELEMENT,
                            static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));
}

static void gst_tcamautoexposure_init(GstTcamautoexposure* self)
{
    self->auto_exposure = TRUE;
    self->auto_gain = TRUE;
    self->auto_iris = TRUE;
    self->gain_is_double = FALSE;

    self->exposure = {};
    self->exposure.min = 0;
    self->exposure.value = 0;
    self->exposure.step = 1;
    self->exposure.max = G_MAXINT;

    self->exposure_min = 0;
    self->exposure_max = G_MAXINT;

    self->gain.min = 0;
    self->gain.step = 0.1;
    self->gain.value = 0.0;
    self->gain.max = G_MAXDOUBLE;

    self->gain_min = 0.0;
    self->gain_max = G_MAXDOUBLE;

    self->frame_counter = 0;

    self->iris = {};
    self->iris_min = 0;
    self->iris_max = G_MAXINT;

    self->camera_src = NULL;
    self->camera_initialized = FALSE;

    self->module_is_disabled = FALSE;

    tcam_image_size s = { SAMPLING_MIN_WIDTH, SAMPLING_MIN_HEIGHT };

    self->roi = create_roi(&s, &s);

    // explicitly set properties
    roi_set_preset(self->roi, ROI_PRESET_FULL_SENSOR);
}

static void gst_tcamautoexposure_set_property(GObject* object,
                                              guint property_id,
                                              const GValue* value,
                                              GParamSpec* pspec)
{
    GstTcamautoexposure* self = GST_TCAMAUTOEXPOSURE(object);

    switch (property_id)
    {
        case PROP_AUTO_EXPOSURE:
        {
            self->auto_exposure = g_value_get_boolean(value);
            break;
        }
        case PROP_AUTO_GAIN:
        {
            self->auto_gain = g_value_get_boolean(value);
            break;
        }
        case PROP_AUTO_IRIS:
        {
            self->auto_iris = g_value_get_boolean(value);
            break;
        }
        case PROP_CAMERA:
        {
            if (self->camera_src)
            {
                gst_object_unref(self->camera_src);
                self->camera_src = nullptr;
            }
            self->camera_src = (GstElement*)g_value_dup_object(value);
            break;
        }
        case PROP_EXPOSURE_MIN:
        {
            if (g_value_get_int(value) > self->exposure_max)
            {
                GST_ERROR("New user value for exposure min is greater or equal to exposure max. "
                          "Ignoring request.");
                break;
            }

            if (g_value_get_int(value) % (int)self->exposure.step != 0)
            {
                GST_ERROR("Wrong step size. Please use a value that is divisible by %f",
                          self->exposure.step);
                break;
            }

            self->exposure_min = g_value_get_int(value);

            if (self->exposure.value < self->exposure_min)
            {
                self->exposure.value = self->exposure_min;
                set_exposure(self, self->exposure.value);
            }

            if (self->exposure_min == 0)
            {
                self->exposure_min = self->default_exposure_values.min;
            }
            break;
        }
        case PROP_EXPOSURE_MAX:
        {
            if (g_value_get_int(value) < self->exposure_min)
            {
                GST_ERROR("New user value for exposure max is smaller or equal to exposure min. "
                          "Ignoring request.");
                break;
            }

            if (self->exposure.value > self->exposure_max)
            {
                self->exposure.value = self->exposure_max;
                set_exposure(self, self->exposure.value);
            }

            if (g_value_get_int(value) % (int)self->exposure.step != 0)
            {
                GST_ERROR("Wrong step size. Please use a value that is divisible by %f",
                          self->exposure.step);
                break;
            }

            self->exposure_max = g_value_get_int(value);
            if (self->exposure_max == 0.0)
            {
                self->exposure_max = self->default_exposure_values.max;
            }
            break;
        }
        case PROP_GAIN_MIN:
        {
            GST_DEBUG("Setting gain min to : %f", g_value_get_double(value));
            if (g_value_get_double(value) > self->gain_max)
            {
                GST_WARNING("New user value for gain min is greater or equal to gain max. Ignoring "
                            "request.");
                break;
            }

            if (g_value_get_double(value) < self->gain.min)
            {
                GST_WARNING(
                    "New user value for gain min (%f) is greater than device gain min (%f).",
                    g_value_get_double(value),
                    self->gain.min);
                self->gain_min = self->gain.min;
                break;
            }

            self->gain_min = g_value_get_double(value);

            if (self->gain.value < self->gain_min)
            {
                self->gain.value = self->gain_min;
                set_gain(self, self->gain.value);
            }

            if (self->gain_min == 0.0)
            {
                self->gain_min = self->gain.min;
            }
            break;
        }
        case PROP_GAIN_MAX:
        {
            GST_DEBUG("Setting gain max to : %f", g_value_get_double(value));
            if (g_value_get_double(value) < self->gain_min)
            {
                GST_WARNING("New user value for gain max is smaller or equal to gain min. Ignoring "
                            "request.");
                break;
            }

            if ((g_value_get_double(value) > self->gain.max))
            {
                GST_WARNING("New user value for gain max is bigger that device gain max. Ignoring "
                            "request.");
                self->gain_max = self->gain.max;
                break;
            }

            self->gain_max = g_value_get_double(value);

            if (self->gain.value > self->gain_max)
            {
                self->gain.value = self->gain_max;
                set_gain(self, self->gain.value);
            }

            if (self->gain_max == G_MAXDOUBLE)
            {
                self->gain_max = self->gain.max;
            }
            break;
        }
        case PROP_IRIS_MIN:
        {
            if (g_value_get_int(value) > self->iris_max)
            {
                GST_ERROR("New user value for iris min is greater or equal to iris max. Ignoring "
                          "request.");
                break;
            }

            self->iris_min = g_value_get_int(value);

            if (self->iris.value < self->iris_min)
            {
                self->iris.value = self->iris_min;
                set_iris(self, self->iris.value);
            }

            if (self->iris_min == 0)
            {
                self->iris_min = self->iris.min;
            }
            break;
        }
        case PROP_IRIS_MAX:
        {
            if (g_value_get_int(value) < self->iris_min)
            {
                GST_ERROR("New user value for iris max is smaller or equal to iris min. Ignoring "
                          "request.");
                break;
            }

            self->iris_max = g_value_get_int(value);
            if (self->iris_max == 0.0 || self->iris_max == G_MAXINT)
            {
                self->iris_max = self->iris.max;
            }

            if (self->iris.value > self->iris_max)
            {
                self->iris.value = self->iris_max;
                set_iris(self, self->iris.value);
            }

            break;
        }
        case PROP_BRIGHTNESS_REFERENCE:
        {
            self->brightness_reference = g_value_get_int(value);
            break;
        }
        case PROP_ROI_LEFT:
        {
            roi_set_left(self->roi, g_value_get_int(value));
            break;
        }
        case PROP_ROI_TOP:
        {
            roi_set_top(self->roi, g_value_get_int(value));
            break;
        }
        case PROP_ROI_WIDTH:
        {
            roi_set_width(self->roi, g_value_get_int(value));
            break;
        }
        case PROP_ROI_HEIGHT:
        {
            roi_set_height(self->roi, g_value_get_int(value));
            break;
        }
        default:
        {
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
        }
    }
}

static void gst_tcamautoexposure_get_property(GObject* object,
                                              guint property_id,
                                              GValue* value,
                                              GParamSpec* pspec)
{
    GstTcamautoexposure* tcamautoexposure = GST_TCAMAUTOEXPOSURE(object);

    switch (property_id)
    {
        case PROP_AUTO_EXPOSURE:
            g_value_set_boolean(value, tcamautoexposure->auto_exposure);
            break;
        case PROP_AUTO_GAIN:
            g_value_set_boolean(value, tcamautoexposure->auto_gain);
            break;
        case PROP_AUTO_IRIS:
            g_value_set_boolean(value, tcamautoexposure->auto_iris);
            break;
        case PROP_CAMERA:
            g_value_set_object(value, tcamautoexposure->camera_src);
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
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static void gst_tcamautoexposure_finalize(GObject* object)
{
    auto self = GST_TCAMAUTOEXPOSURE(object);
    destroy_roi(self->roi);

    if (self->camera_src)
    {
        gst_object_unref(self->camera_src);
    }

    G_OBJECT_CLASS(gst_tcamautoexposure_parent_class)->finalize(object);
}


static GstStateChangeReturn gst_tcamautoexposure_change_state(GstElement* element,
                                                              GstStateChange trans)
{
    GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;

    GstTcamautoexposure* self = GST_TCAMAUTOEXPOSURE(element);

    switch (trans)
    {
        case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
        {

            if (self->camera_src == NULL)
            {
                self->camera_src = tcam_gst_find_camera_src(GST_ELEMENT(self));
                if (self->camera_src == nullptr)
                {
                    GST_ERROR("Could not find source element");
                    return GST_STATE_CHANGE_FAILURE;
                }
                else
                {
                    self->camera_initialized = FALSE;

                    // find_image_values(self);
                    // init_camera_resources(self);
                }
            }
            break;
        }
        default:
        {
            break;
        }
    }

    gst_element_set_locked_state(element, TRUE);
    ret = GST_ELEMENT_CLASS(gst_tcamautoexposure_parent_class)->change_state(element, trans);
    gst_element_set_locked_state(element, FALSE);

    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        return ret;
    }

    switch (trans)
    {
        case GST_STATE_CHANGE_READY_TO_NULL:
        {
            if (self->camera_src)
            {
                gst_object_unref(self->camera_src);
                self->camera_src = NULL;
                self->camera_initialized = FALSE;
            }

            break;
        }
        default:
        {
            break;
        }
    }
    return ret;
}

static void init_camera_resources(GstTcamautoexposure* self)
{
    bool has_auto_exposure = false;
    bool has_auto_gain = false;

    {
        GSList* names = tcam_prop_get_tcam_property_names(TCAM_PROP(self->camera_src));

        for (unsigned int i = 0; i < g_slist_length(names); ++i)
        {
            char* name = (char*)g_slist_nth(names, i)->data;

            if (g_strcmp0(name, "Exposure") == 0 || g_strcmp0(name, "Exposure Time (us)") == 0
                || g_strcmp0(name, "ExposureTime") == 0)
            {
                self->exposure_name = name;
            }
            else if (g_strcmp0(name, "Gain") == 0)
            {
                self->gain_name = name;
            }
            else if (g_strcmp0(name, "Iris") == 0)
            {
                self->iris_name = name;
            }
            else if (g_strcmp0(name, "Exposure Auto") == 0
                     || g_strcmp0(name, "ExposureTimeAuto") == 0)
            {
                has_auto_exposure = true;
            }
            else if (g_strcmp0(name, "Gain Auto") == 0 || g_strcmp0(name, "GainAuto") == 0)
            {
                has_auto_gain = true;
            }
        }

        g_slist_free_full(names, ::g_free);
    }


    if (has_auto_exposure || has_auto_gain)
    {
        GST_INFO("Device already has auto properties. Disabling module.");
        self->module_is_disabled = TRUE;
        return;
    }

    if (self->exposure_name.empty())
    {
        GST_ERROR("Exposure could not be found!");
        self->auto_exposure = FALSE;
    }
    else
    {
        GValue value = {};
        GValue min = {};
        GValue max = {};
        GValue step = {};
        GValue type = {};

        gboolean ret = tcam_prop_get_tcam_property(TCAM_PROP(self->camera_src),
                                                   self->exposure_name.c_str(),
                                                   &value,
                                                   &min,
                                                   &max,
                                                   nullptr,
                                                   &step,
                                                   &type,
                                                   nullptr,
                                                   nullptr,
                                                   nullptr);

        if (!ret)
        {
            printf("Could not query property '%s'\n", self->exposure_name.c_str());
            return;
        }

        const char* t = g_value_get_string(&type);
        if (strcmp(t, "integer") == 0)
        {
            self->exposure_is_double = FALSE;
            self->exposure.min = g_value_get_int(&min);
            // do not set exposure.max
            // we default to 0
            // if 0 -> use max setting according to framerate
            // if max is set by user we use that
            // self->exposure.max = g_value_get_int(&max);
            self->exposure.value = g_value_get_int(&value);

            self->exposure.step = g_value_get_int(&step);
        }
        else if (strcmp(t, "double") == 0)
        {
            self->exposure_is_double = TRUE;
            self->exposure.min = g_value_get_double(&min);
            // self->exposure.max = g_value_get_double(&max);
            self->exposure.value = g_value_get_double(&value);
            self->exposure.step = g_value_get_double(&step);
        }

        self->default_exposure_values.min = self->exposure.min;
        self->default_exposure_values.max =
            1000000 / double(self->framerate_numerator / self->framerate_denominator);

        // This is done the prevent error down the line.
        // By setting max to something framerate related it may not align with the
        // possible step size. This can lead to the algorithm trying to set
        // exposure to a value that is the calculated max but that cannot be set.
        // Thus realign with stepsize.
        auto modulo = (int)self->default_exposure_values.max % (int)self->exposure.step;
        self->default_exposure_values.max -= modulo;

        GST_INFO("Exposure boundaries are %f %d", self->exposure.min, self->exposure_max);
        GST_INFO(
            "Exposure boundaries are %f %f", self->exposure.min, self->default_exposure_values.max);

        g_value_unset(&value);
        g_value_unset(&min);
        g_value_unset(&max);
        g_value_unset(&type);
    }

    if (self->gain_name.empty())
    {
        GST_ERROR("Gain could not be found!");
        self->auto_gain = FALSE;
    }
    else
    {
        GValue value = {};
        GValue min = {};
        GValue max = {};
        GValue step_size = {};
        GValue type = {};

        gboolean ret = tcam_prop_get_tcam_property(TCAM_PROP(self->camera_src),
                                                   self->gain_name.c_str(),
                                                   &value,
                                                   &min,
                                                   &max,
                                                   nullptr,
                                                   &step_size,
                                                   &type,
                                                   nullptr,
                                                   nullptr,
                                                   nullptr);

        if (!ret)
        {
            GST_ERROR("Could not query property '%s'", self->gain_name.c_str());
            return;
        }


        if (strcmp(g_value_get_string(&type), "integer") == 0)
        {
            self->gain.min = g_value_get_int(&min);
            self->gain.max = g_value_get_int(&max);
            self->gain.value = g_value_get_int(&value);
            self->gain.step = g_value_get_int(&step_size);
        }
        else
        {
            self->gain_is_double = TRUE;
            if (g_value_get_double(&min) == 0.0)
            {
                self->gain.min = GAIN_FLOAT_MULTIPLIER;
            }
            else
            {
                self->gain.min = g_value_get_double(&min) * GAIN_FLOAT_MULTIPLIER;
            }
            // self->gain.min = p.value.d.min * GAIN_FLOAT_MULTIPLIER;
            self->gain.max = g_value_get_double(&max) * GAIN_FLOAT_MULTIPLIER;

            double gain_value = g_value_get_double(&value);

            if (gain_value != 0)
            {
                self->gain.value = gain_value * GAIN_FLOAT_MULTIPLIER;
            }
            else
            {
                self->gain.value = self->gain.min;
            }
            self->gain.step = g_value_get_double(&step_size) * GAIN_FLOAT_MULTIPLIER;
        }
        if (self->gain_max == 0 || self->gain_max == G_MAXDOUBLE || self->gain_max > self->gain.max)
        {
            self->gain_max = self->gain.max;
        }
        if (self->gain_min == 0 || self->gain_min < self->gain.min)
        {
            self->gain_min = self->gain.min;
        }
        GST_INFO("Gain boundaries are %f %f", self->gain.min, self->gain.max);

        g_value_unset(&value);
        g_value_unset(&min);
        g_value_unset(&max);
        g_value_unset(&step_size);
        g_value_unset(&type);
    }

    if (self->exposure_min == 0 || self->exposure_min < self->default_exposure_values.min)
    {
        self->exposure_min = self->default_exposure_values.min;
    }
    if (self->exposure_max == 0 || self->exposure_max == G_MAXINT
        || self->exposure_max > self->default_exposure_values.max)
    {
        self->exposure_max = self->default_exposure_values.max;
    }

    if (self->iris_name.empty())
    {
        GST_INFO("Iris could not be found");
        self->auto_iris = FALSE;
    }
    else
    {
        //self->iris.min = p.value.i.min;
        // most iris modules are the same
        // with a range from 0 - 1023
        // 100 is a good base number.
        // GigE cameras offer IrisAutoMin that suggests what this should be

        GValue value = {};
        GValue min = {}; // this is later ignored
        GValue max = {};
        gboolean ret = tcam_prop_get_tcam_property(TCAM_PROP(self->camera_src),
                                                   self->iris_name.c_str(),
                                                   &value,
                                                   &min,
                                                   &max,
                                                   nullptr,
                                                   nullptr,
                                                   nullptr,
                                                   nullptr,
                                                   nullptr,
                                                   nullptr);

        if (!ret)
        {
            GST_ERROR("Could not query property '%s'", self->iris_name.c_str());
            return;
        }

        self->iris.min = 100;
        self->iris.max = g_value_get_int(&max);
        self->iris.value = g_value_get_int(&value);

        if (self->iris_min == 0)
        {
            self->iris_min = self->iris.min;
        }
        if (self->iris_max == 0 || self->iris_max == G_MAXINT || self->iris_max > self->iris.max)
        {
            self->iris_max = self->iris.max;
        }

        g_value_unset(&value);
        g_value_unset(&min);
        g_value_unset(&max);

        GST_INFO("Iris boundaries are %d %d", self->iris.min, self->iris.max);
    }

    self->camera_initialized = TRUE;
}


static void set_exposure(GstTcamautoexposure* self, gdouble exposure)
{
    if (!G_IS_OBJECT(self->camera_src))
    {
        GST_WARNING("Have no camera source to set exposure.");
        return;
    }

    if (self->exposure_name.empty())
    {
        GST_WARNING("Attempting to set exposure while name is empty. Ignoring.");
        return;
    }

    GValue value = G_VALUE_INIT;

    if (self->exposure_is_double)
    {
        GST_TRACE("Setting exposure to %f", exposure);
        g_value_init(&value, G_TYPE_DOUBLE);
        g_value_set_double(&value, exposure);
    }
    else
    {
        GST_TRACE("Setting exposure to %f", exposure);
        g_value_init(&value, G_TYPE_INT);
        g_value_set_int(&value, exposure);
    }


    tcam_prop_set_tcam_property(TCAM_PROP(self->camera_src), self->exposure_name.c_str(), &value);

    g_value_unset(&value);
}


static void set_gain(GstTcamautoexposure* self, gdouble gain)
{
    if (!G_IS_OBJECT(self->camera_src))
    {
        GST_WARNING("Have no camera source to set gain.");
        return;
    }

    if (self->gain_name.empty())
    {
        GST_WARNING("Attempting to set exposure while name is empty. Ignoring.");
        return;
    }

    GValue value = G_VALUE_INIT;

    if (!self->gain_is_double)
    {
        GST_INFO("Setting gain to int %f", gain);
        g_value_init(&value, G_TYPE_INT);
        g_value_set_int(&value, gain);
    }
    else
    {
        g_value_init(&value, G_TYPE_DOUBLE);

        g_value_set_double(&value, (double)((double)gain / GAIN_FLOAT_MULTIPLIER));
        GST_INFO("Setting gain to float %f", (double)((double)gain / GAIN_FLOAT_MULTIPLIER));
    }
    tcam_prop_set_tcam_property(TCAM_PROP(self->camera_src), self->gain_name.c_str(), &value);

    g_value_unset(&value);
}


static void set_iris(GstTcamautoexposure* self, int iris)
{
    if (!G_IS_OBJECT(self->camera_src))
    {
        GST_WARNING("Have no camera source to set iris.");
        return;
    }

    if (self->iris_name.empty())
    {
        GST_WARNING("Attempting to set iris while name is empty. Ignoring.");
        return;
    }

    GST_DEBUG("Setting iris to %d", iris);
    GValue value = G_VALUE_INIT;

    g_value_init(&value, G_TYPE_INT);
    g_value_set_int(&value, iris);

    tcam_prop_set_tcam_property(TCAM_PROP(self->camera_src), self->iris_name.c_str(), &value);

    g_value_unset(&value);
}


static void retrieve_current_values(GstTcamautoexposure* self)
{
    if (!self->exposure_name.empty())
    {
        GValue value = {};
        gboolean ret = tcam_prop_get_tcam_property(TCAM_PROP(self->camera_src),
                                                   self->exposure_name.c_str(),
                                                   &value,
                                                   nullptr,
                                                   nullptr,
                                                   nullptr,
                                                   nullptr,
                                                   nullptr,
                                                   nullptr,
                                                   nullptr,
                                                   nullptr);

        if (!ret)
        {
            GST_ERROR("Could not query property '%s'\n", self->exposure_name.c_str());
            return;
        }

        //GST_DEBUG("Current exposure is %ld", p.value.i.value);
        if (self->exposure_is_double)
        {
            self->exposure.value = g_value_get_double(&value);
        }
        else
        {
            self->exposure.value = g_value_get_int(&value);
        }
        g_value_unset(&value);
    }

    if (!self->gain_name.empty())
    {
        GValue value = {};
        gboolean ret = tcam_prop_get_tcam_property(TCAM_PROP(self->camera_src),
                                                   self->gain_name.c_str(),
                                                   &value,
                                                   nullptr,
                                                   nullptr,
                                                   nullptr,
                                                   nullptr,
                                                   nullptr,
                                                   nullptr,
                                                   nullptr,
                                                   nullptr);

        if (!ret)
        {
            GST_ERROR("Could not query property '%s'\n", self->gain_name.c_str());
            return;
        }

        if (!self->gain_is_double)
        {
            self->gain.value = g_value_get_int(&value);
            if (self->gain.value < self->gain.min)
            {
                self->gain.value = self->gain.min;
            }
            GST_DEBUG("Current gain is %f", self->gain.value);
        }
        else
        {
            double val = g_value_get_double(&value);
            GST_DEBUG("Current gain is %f", val);
            if (val != 0)
                self->gain.value = val * GAIN_FLOAT_MULTIPLIER;
        }
        g_value_unset(&value);
    }

    if (self->iris_name.empty())
    {
        self->auto_iris = false;
    }
    else
    {

        GValue value = {};
        gboolean ret = tcam_prop_get_tcam_property(TCAM_PROP(self->camera_src),
                                                   self->iris_name.c_str(),
                                                   &value,
                                                   nullptr,
                                                   nullptr,
                                                   nullptr,
                                                   nullptr,
                                                   nullptr,
                                                   nullptr,
                                                   nullptr,
                                                   nullptr);

        if (!ret)
        {
            GST_ERROR("Could not query property '%s'\n", self->iris_name.c_str());
            return;
        }

        //GST_DEBUG("Current iris is %ld", p.value.i.value);
        self->iris.value = g_value_get_int(&value);
        g_value_unset(&value);
    }
}


static tBY8Pattern calculate_pattern_from_offset(GstTcamautoexposure* self)
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

static image_buffer retrieve_image_region(GstTcamautoexposure* self, GstBuffer* buf)
{
    GstMapInfo info;

    gst_buffer_map(buf, &info, GST_MAP_READ);

    tcam_image_buffer image = {};


    gst_buffer_to_tcam_image_buffer(buf, nullptr, &image);
    image.format = self->active_format;

    tcam_image_buffer roi_buffer = {};

    if (!roi_extract_view(self->roi, &image, &roi_buffer))
    {
        GST_ERROR("Unable to extract ROI");
        return {};
    }

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


static void new_exposure(GstTcamautoexposure* self, unsigned int brightness)
{
    auto_alg::property_cont_gain gain = {};
    gain.is_gain_db = false;
    gain.min = self->gain_min;
    gain.max = self->gain_max;
    gain.value = self->gain.value;
    gain.auto_enabled = self->auto_gain;

    auto_alg::property_cont_exposure exposure = {};
    exposure.min = self->exposure_min;
    exposure.max = self->exposure_max;
    exposure.val = self->exposure.value;
    exposure.auto_enabled = self->auto_exposure;
    exposure.granularity = self->exposure.step;

    auto_alg::property_cont_iris iris = {};
    if (!self->iris_name.empty())
    {
        iris.camera_fps = self->framerate_numerator / self->framerate_denominator;
        iris.auto_enabled = self->auto_iris;
        iris.min = self->iris_min;
        iris.max = self->iris_max;
        iris.val = self->iris.value;
        iris.is_pwm_iris = false;
    }
    else
    {
        iris.auto_enabled = false;
    }

    auto res = auto_alg::impl::calc_auto_gain_exposure_iris(
        brightness, self->brightness_reference, gain, exposure, iris);

    if (self->auto_exposure && self->exposure.value != res.exposure)
    {
        set_exposure(self, res.exposure);
    }

    if (self->auto_gain && self->gain.value != res.gain)
    {
        set_gain(self, res.gain);
    }

    if (self->auto_iris && self->iris.value != res.iris)
    {
        set_iris(self, res.iris);
    }
}


static void correct_brightness(GstTcamautoexposure* self, GstBuffer* buf)
{
    image_buffer buffer = retrieve_image_region(self, buf);
    guint brightness = 0;

    if (self->color_format == BAYER)
    {
        brightness = image_brightness_bayer(&buffer);
    }
    else
    {
        if (self->bit_depth == 8)
        {
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
    retrieve_current_values(self);

    new_exposure(self, brightness);
}


static gboolean find_image_values(GstTcamautoexposure* self)
{
    GstPad* pad = GST_BASE_TRANSFORM_SINK_PAD(self);
    GstCaps* caps = gst_pad_get_current_caps(pad);
    if (!gst_caps_to_tcam_video_format(caps, &self->active_format))
    {
        return FALSE;
    }

    GstStructure* structure = gst_caps_get_structure(caps, 0);

    gint tmp_w, tmp_h;
    g_return_val_if_fail(gst_structure_get_int(structure, "width", &tmp_w), FALSE);
    g_return_val_if_fail(gst_structure_get_int(structure, "height", &tmp_h), FALSE);
    self->image_size.width = tmp_w < 0 ? 0 : tmp_w;
    self->image_size.height = tmp_h < 0 ? 0 : tmp_h;

    if (self->image_region.x1 == 0)
    {
        self->image_region.x1 = self->image_size.width;
    }

    if (self->image_region.y1 == 0)
    {
        self->image_region.y1 = self->image_size.height;
    }

    gst_structure_get_fraction(
        structure, "framerate", &self->framerate_numerator, &self->framerate_denominator);

    if (strcmp(gst_structure_get_name(structure), "video/x-bayer") == 0)
    {
        self->color_format = BAYER;

        guint fourcc = 0;

        if (gst_structure_get_field_type(structure, "format") == G_TYPE_STRING)
        {
            const char* string;
            string = gst_structure_get_string(structure, "format");
            fourcc = GST_STR_FOURCC(string);
        }

        if (fourcc == MAKE_FOURCC('g', 'r', 'b', 'g'))
        {
            self->pattern = GR;
        }
        else if (fourcc == MAKE_FOURCC('r', 'g', 'g', 'b'))
        {
            self->pattern = RG;
        }
        else if (fourcc == MAKE_FOURCC('g', 'b', 'r', 'g'))
        {
            self->pattern = GB;
        }
        else if (fourcc == MAKE_FOURCC('b', 'g', 'g', 'r'))
        {
            self->pattern = BG;
        }
    }
    else
    {
        self->color_format = GRAY;
        // will not be used, but still explicitly define something
        self->pattern = BG;
    }

    gst_caps_unref(caps);

    return TRUE;
}


/*
  Entry point for actual transformation
*/
static GstFlowReturn gst_tcamautoexposure_transform_ip(GstBaseTransform* trans, GstBuffer* buf)
{
    GstTcamautoexposure* self = GST_TCAMAUTOEXPOSURE(trans);

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

    if (!self->camera_initialized)
    {
        init_camera_resources(self);
    }

    if (self->auto_exposure == FALSE && self->auto_gain == FALSE && self->auto_iris == FALSE)
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


static gboolean gst_tcamautoexposure_set_caps(GstBaseTransform* trans,
                                              GstCaps* incaps,
                                              GstCaps* outcaps)
{
    GstTcamautoexposure* self = GST_TCAMAUTOEXPOSURE(trans);
    GstStructure* structure = nullptr;

    GST_DEBUG(
        "in caps %" GST_PTR_FORMAT " out caps %" GST_PTR_FORMAT, (void*)incaps, (void*)outcaps);
    structure = gst_caps_get_structure(incaps, 0);

    self->bit_depth = 8;

    if (g_str_equal(gst_structure_get_name(structure), "video/x-bayer"))
    {
        const char* caps_format;
        caps_format = gst_structure_get_string(structure, "format");
        self->color_format = BAYER;
        if (g_str_equal(caps_format, "bggr"))
        {
            self->pattern = BG;
        }
        else if (g_str_equal(caps_format, "gbrg"))
        {
            self->pattern = GB;
        }
        else if (g_str_equal(caps_format, "grbg"))
        {
            self->pattern = GR;
        }
        else if (g_str_equal(caps_format, "rggb"))
        {
            self->pattern = RG;
        }
        else
        {
            g_critical("Format '%s' not handled by this element", caps_format);
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

    roi_set_image_size(self->roi, { width, height });

    return true;
}


static gboolean plugin_init(GstPlugin* plugin)
{
    return gst_element_register(
        plugin, "tcamautoexposure", GST_RANK_NONE, GST_TYPE_TCAMAUTOEXPOSURE);
}

#ifndef PACKAGE
#define PACKAGE "tcamautoexposure"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "tcamautoexposure"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "https://github.com/TheImagingSource/tiscamera"
#endif

GST_PLUGIN_DEFINE(GST_VERSION_MAJOR,
                  GST_VERSION_MINOR,
                  tcamautoexposure,
                  "The Imaging Source auto exposure plugin",
                  plugin_init,
                  get_version(),
                  "Proprietary",
                  PACKAGE_NAME,
                  GST_PACKAGE_ORIGIN)
