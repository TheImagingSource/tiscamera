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

#ifdef ENABLE_ARAVIS
#include <arv.h>
#endif

#include <math.h>
#include <stdbool.h>
#include <ctype.h>
#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include "gsttis_autofocus.h"

GST_DEBUG_CATEGORY_STATIC (gst_tis_autofocus_debug_category);
#define GST_CAT_DEFAULT gst_tis_autofocus_debug_category


/* prototypes */

static void gst_tis_autofocus_set_property (GObject* object,
                                            guint property_id,
                                            const GValue* value,
                                            GParamSpec* pspec);
static void gst_tis_autofocus_get_property (GObject* object,
                                                guint property_id,
                                                GValue* value,
                                                GParamSpec* pspec);
static void gst_tis_autofocus_finalize (GObject* object);

static GstFlowReturn gst_tis_autofocus_transform_ip (GstBaseTransform* trans,
                                                         GstBuffer* buf);
static GstCaps* gst_tis_autofocus_transform_caps (GstBaseTransform* trans,
                                                      GstPadDirection direction,
                                                      GstCaps* caps);

static void gst_tis_autofocus_fixate_caps (GstBaseTransform* base,
                                               GstPadDirection direction,
                                               GstCaps* caps,
                                               GstCaps* othercaps);

enum
{
    PROP_0,
    PROP_AUTO,
    PROP_X,
    PROP_Y,
    PROP_SIZE,
};


static GstStaticPadTemplate gst_tis_autofocus_sink_template =
    GST_STATIC_PAD_TEMPLATE ("sink",
                             GST_PAD_SINK,
                             GST_PAD_ALWAYS,
                             GST_STATIC_CAPS ("ANY"));

static GstStaticPadTemplate gst_tis_autofocus_src_template =
    GST_STATIC_PAD_TEMPLATE ("src",
                             GST_PAD_SRC,
                             GST_PAD_ALWAYS,
                             GST_STATIC_CAPS ("ANY"));

G_DEFINE_TYPE_WITH_CODE (GstTis_AutoFocus,
                         gst_tis_autofocus,
                         GST_TYPE_BASE_TRANSFORM,
                         GST_DEBUG_CATEGORY_INIT (gst_tis_autofocus_debug_category,
                                                  "tis_autofocus",
                                                  0,
                                                  "debug category for tis_autofocus element"));

GstElement* get_camera_src (GstElement* object)
{
    GstTis_AutoFocus* self = GST_TIS_AUTOFOCUS(object);
   
    GstElement* e = GST_ELEMENT( gst_object_get_parent(GST_OBJECT(object)));

    GList* l =  GST_BIN(e)->children;

    while (1==1)
    {
        const char* name = g_type_name(gst_element_factory_get_element_type (gst_element_get_factory(l->data)));

        if (g_strcmp0(name, "GstAravis") == 0)
        {
            self->camera_src = l->data;
            break;
        }

        if (g_list_next(l) == NULL)
            break;

        l = g_list_next(l);
    }


    return self->camera_src;
}


void focus_run (GstTis_AutoFocus* self)
{
    if (self->camera_src == NULL)
        get_camera_src (GST_ELEMENT(self));
    
    ArvCamera* camera;
    g_object_get (G_OBJECT (self->camera_src), "camera", &camera, NULL);
    
    if (self->camera_src == NULL)
    {
        gst_debug_log (gst_tis_autofocus_debug_category,
                       GST_LEVEL_ERROR,
                       "tis_autofocus",
                       "run",
                       336,
                       NULL,
                       "SOURCE EMPTY! Aborting. ");
    }
    if (camera == NULL)
    {
        gst_debug_log (gst_tis_autofocus_debug_category,
                       GST_LEVEL_ERROR,
                       "tis_autofocus",
                       "run",
                       336,
                       NULL,
                       "Unable to retrieve camera! Aborting. ");
    }
    
    ArvDevice* device = arv_camera_get_device(camera);

    if (device == NULL)
    {
        gst_debug_log (gst_tis_autofocus_debug_category,
                       GST_LEVEL_ERROR,
                       "tis_autofocus",
                       "run",
                       336,
                       NULL,
                       "Unable to retrieve device! Aborting. ");
    }
    
    int current_focus = arv_device_get_integer_feature_value(device, "Focus");
    int focus_auto_min =  arv_device_get_integer_feature_value(device, "FocusAutoMin"); 
    int focus_auto_step_divisor = arv_device_get_integer_feature_value(device, "FocusAutoStepDivisor");
    
    gint64 min;
    gint64 max;
    arv_device_get_integer_feature_bounds(device, "Focus", &min, &max);

    if (max < focus_auto_min)
    {
        gst_debug_log (gst_tis_autofocus_debug_category,
                       GST_LEVEL_ERROR,
                       "tis_autofocus",
                       "run",
                       336,
                       NULL,
                       "Illogical values: Focus %d Min %d Max %d Divisor %d \nAborting run.",
                       current_focus,
                       focus_auto_min,
                       max,
                       focus_auto_step_divisor);
        
    }
    
    /* values from windows driver */
    /* min = 0; */
    /* max = 1000; */
    /* roi = {0, 0, 0, 0} */
    /* speed = 500 */
    /* auto_step_divisor = 4 */
    /* suggest_sweep = false */

    RECT r = {0, 0, 0, 0};

    /* user defined rectangle */
    if (self->x != 0 || self->y != 0)
    {
        r.left = (self->x - self->size < 0) ? 0 : self->x - self->size;
        r.right =(self->x - self->size > self->width) ? self->width : self->x - self->size;
        r.top = (self->y - self->size < 0) ? 0 : self->y - self->size;
        r.bottom = (self->y - self->size > self->height) ? self->height : self->y - self->size;
    }
    
    if (current_focus < focus_auto_min)
    {
        arv_device_set_integer_feature_value(device, "Focus", focus_auto_min);
        current_focus = focus_auto_min;
    }
    self->cur_focus = current_focus;

    gst_debug_log (gst_tis_autofocus_debug_category,
                   GST_LEVEL_ERROR,
                   "tis_autofocus",
                   "run",
                   336,
                   NULL,
                   "Callig autofocus_run with: Focus %d Min %d Max %d Divisor %d ",
                   current_focus,
                   focus_auto_min,
                   max,
                   focus_auto_step_divisor);
    
    autofocus_run(self->focus,
                  current_focus,
                  focus_auto_min,
                  max,
                  r,
                  500,
                  focus_auto_step_divisor,
                  false);
}


static void gst_tis_autofocus_class_init (GstTis_AutoFocusClass* klass)
{
    GObjectClass* gobject_class = G_OBJECT_CLASS (klass);
    GstBaseTransformClass* base_transform_class = GST_BASE_TRANSFORM_CLASS (klass);

    gst_element_class_add_static_pad_template (GST_ELEMENT_CLASS(klass),
                                               &gst_tis_autofocus_src_template);
    gst_element_class_add_static_pad_template (GST_ELEMENT_CLASS(klass),
                                               &gst_tis_autofocus_sink_template);

    gst_element_class_set_details_simple (GST_ELEMENT_CLASS(klass),
                                          "The Imaging Source auto focus Element",
                                          "Generic",
                                          "Adjusts the image focus by setting camera properties.",
                                          "Edgar Thier <edgar.thier@theimagingsource.com>");

    gobject_class->set_property = gst_tis_autofocus_set_property;
    gobject_class->get_property = gst_tis_autofocus_get_property;
    gobject_class->finalize = gst_tis_autofocus_finalize;
    base_transform_class->transform_ip = GST_DEBUG_FUNCPTR (gst_tis_autofocus_transform_ip);
    base_transform_class->transform_caps = GST_DEBUG_FUNCPTR (gst_tis_autofocus_transform_caps);
    base_transform_class->fixate_caps = GST_DEBUG_FUNCPTR (gst_tis_autofocus_fixate_caps);

    g_object_class_install_property (gobject_class,
                                     PROP_AUTO,
                                     g_param_spec_boolean ("auto",
                                                           "Activate auto focus run",
                                                           "Automatically adjust exposure balance",
                                                           FALSE,
                                                           G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    g_object_class_install_property (gobject_class,
                                     PROP_X,
                                     g_param_spec_int ("x",
                                                       "X Coordinate",
                                                       "Coordinate for focus region.",
                                                       0, G_MAXINT, 0,
                                                       G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    g_object_class_install_property (gobject_class,
                                     PROP_Y,
                                     g_param_spec_int ("y",
                                                       "Y Coordinate",
                                                       "Coordinate for focus region.",
                                                       0, G_MAXINT, 0,
                                                       G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    g_object_class_install_property (gobject_class,
                                     PROP_SIZE,
                                     g_param_spec_int ("size",
                                                       "Size of focus region",
                                                       "Radius of the focus region around the given coordinates.",
                                                       0, G_MAXINT, 50,
                                                       G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

}


static void gst_tis_autofocus_init (GstTis_AutoFocus *self)
{
    self->focus = autofocus_create();
    self->cur_focus = 0;
    self->x = 0;
    self->y = 0;
    self->size = 50;
    self->camera_src = NULL;
}


void gst_tis_autofocus_set_property (GObject* object,
                                     guint property_id,
                                     const GValue* value,
                                     GParamSpec* pspec)
{
    GstTis_AutoFocus* self = GST_TIS_AUTOFOCUS (object);

    switch (property_id)
    {
        case PROP_AUTO:
            self->focus_active = g_value_get_boolean (value);
            if (self->focus_active == TRUE)
                focus_run(self);
            break;
        case PROP_X:
            self->x = g_value_get_int (value);
            break;
        case PROP_Y:
            self->y = g_value_get_int (value);
            break;
        case PROP_SIZE:
            self->size = g_value_get_int (value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}


void gst_tis_autofocus_get_property (GObject* object,
                                     guint property_id,
                                     GValue* value,
                                     GParamSpec* pspec)
{
    GstTis_AutoFocus* self = GST_TIS_AUTOFOCUS (object);

    switch (property_id)
    {
        case PROP_AUTO:
            g_value_set_boolean (value, self->focus_active);
            break;
        case PROP_X:
            g_value_set_int (value, self->x);
            break;
        case PROP_Y:
            g_value_set_int (value, self->y);
            break;
        case PROP_SIZE:
            g_value_set_int (value, self->size);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}


void gst_tis_autofocus_finalize (GObject* object)
{
    GstTis_AutoFocus* self = GST_TIS_AUTOFOCUS (object);
    
    autofocus_destroy(self->focus);
    G_OBJECT_CLASS (gst_tis_autofocus_parent_class)->finalize (object);
}


static GstCaps* gst_tis_autofocus_transform_caps (GstBaseTransform* trans,
                                                  GstPadDirection direction,
                                                  GstCaps* caps)
{
    GstTis_AutoFocus* self = GST_TIS_AUTOFOCUS(trans);
    GstCaps *outcaps = gst_caps_copy (caps);

    if (self->camera_src != NULL)
        return outcaps;

    /* if camera_src is not set we assume that the first default camera src found shall be used */

    get_camera_src(GST_ELEMENT(trans));

    return outcaps;
}


static void gst_tis_autofocus_fixate_caps (GstBaseTransform* base,
                                           GstPadDirection direction,
                                           GstCaps* incoming,
                                           GstCaps* outgoing)
{
    GstTis_AutoFocus* self = GST_TIS_AUTOFOCUS (base);

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
        self->width = width;
    }

    if (gst_structure_get_int (ins, "height", &height))
    {
        if (gst_structure_has_field (outs, "height"))
        {
            gst_structure_fixate_field_nearest_int (outs, "width", height);
        }
        self->height = height;
    }

    if (gst_structure_get_field_type (ins, "format") == G_TYPE_STRING)
    {
		const char *string;

        string = gst_structure_get_string (ins, "format");
		fourcc = GST_STR_FOURCC (string);
	}
    else if (gst_structure_get_field_type (ins, "format") == GST_TYPE_FOURCC)
    {
		gst_structure_get_fourcc (ins, "format", &fourcc);
	}
    else
		fourcc = 0;

    if (fourcc == 0)
    {
        gst_debug_log (gst_tis_autofocus_debug_category,
                       GST_LEVEL_ERROR,
                       "tis_autofocus",
                       "",
                       555,
                       NULL,
                       "Unable to determine fourcc. Using Y800");
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

    
    gst_debug_log (gst_tis_autofocus_debug_category,
                   GST_LEVEL_ERROR,
                   "tis_auto_exposure",
                   "gst_tis_autofocus_fixate_caps",
                   336,
                   NULL,
                   "Fixated caps %d %d %d %d %c%c%c%c",
                   FOURCC_GRBG8,
                   FOURCC_GBRG8,
                   FOURCC_BGGR8,
                   FOURCC_RGGB8,
                   GST_FOURCC_ARGS(fourcc));
        
    /* gst_debug_log (gst_tis_autofocus_debug_category, */
    /*                GST_LEVEL_ERROR, */
    /*                "tis_auto_exposure", */
    /*                "gst_tis_autofocus_fixate_caps", */
    /*                336, */
    /*                NULL, */
    /*                "Fixated caps %d %d %d %d %d %d", */
    /*                FOURCC_GRBG, */
    /*                FOURCC_GRBG8, */
    /*                FOURCC_GBRG8, */
    /*                FOURCC_BGGR8, */
    /*                FOURCC_RGGB8, */
    /*                fourcc); */
    
}


static int clip(int min, int value, int max)
{
    if (min > value)
        return min;
    if (max < value)
        return max;
    return value;
}


static GstFlowReturn gst_tis_autofocus_transform_ip (GstBaseTransform* trans, GstBuffer* buf)
{
    GstTis_AutoFocus* self = GST_TIS_AUTOFOCUS (trans);

    if (autofocus_is_running(self->focus))
    {
        if (self->camera_src)
            get_camera_src(GST_ELEMENT(self));
        
        ArvCamera* camera;

        g_object_get (G_OBJECT (self->camera_src), "camera", &camera, NULL);

        ArvDevice* device = arv_camera_get_device(camera);

        int current_focus = arv_device_get_integer_feature_value(device, "Focus");
        int focus_auto_min =  arv_device_get_integer_feature_value(device, "FocusAutoMin"); 

        gint64 min;
        gint64 max;
        arv_device_get_integer_feature_bounds(device, "Focus", &min, &max);

        gst_debug_log (gst_tis_autofocus_debug_category,
                       GST_LEVEL_ERROR,
                       "tis_autofocus",
                       "fixate_caps",
                       336,
                       NULL,
                       "cur focus %d ", current_focus);

        /* assure we use the current focus value */
        autofocus_update_focus(self->focus, clip(focus_auto_min, current_focus, max));
        
        img_descriptor img =
            {
                GST_BUFFER_DATA(buf),
                GST_BUFFER_SIZE(buf),
                FOURCC_GRBG,    /* TODO: DYNAMICALLY FIND FORMAT */
                self->width,
                self->height,
                self->width
            };

        int new_focus_value;
        POINT p = {0, 0};

        bool ret = autofocus_analyze_frame(self->focus,
                                           img,
                                           p,
                                           1,
                                           &new_focus_value);

        if (ret)
        {
            gst_debug_log (gst_tis_autofocus_debug_category,
                           GST_LEVEL_ERROR,
                           "tis_autofocus",
                           "fixate_caps",
                           336,
                           NULL,
                           "Setting focus %d", new_focus_value);
            arv_device_set_integer_feature_value(device, "Focus", new_focus_value);
            self->cur_focus = new_focus_value;
        }
        
        return GST_FLOW_OK;
    }

    autofocus_end(self->focus);
    self->focus_active = FALSE;
    return GST_FLOW_OK;
}


static gboolean plugin_init (GstPlugin * plugin)
{
    return gst_element_register (plugin,
                                 "tis_autofocus",
                                 GST_RANK_NONE,
                                 GST_TYPE_TIS_AUTOFOCUS);
}


#ifndef VERSION
#define VERSION "0.0.1"
#endif
#ifndef PACKAGE
#define PACKAGE "tis_autofocus_package"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "tis_autofocus_package_name"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "https://github.com/TheImagingSource/tiscamera"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
                   GST_VERSION_MINOR,
                   "tis_autofocus",
                   "The Imaging Source auto exposure plugin",
                   plugin_init,
                   VERSION,
                   "LGPL",
                   PACKAGE_NAME, GST_PACKAGE_ORIGIN)

