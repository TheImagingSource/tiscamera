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
 *  processing is done in a simple manner:
 *
 *  process only a few frames to give the camera time to adjust
 *
 *  on selected frame determine brightness of the whole image
 *
 *  The configuration from exposure and gain. Works under the following assumptions:
 *  Gain should always be as low as possible.
 *  Exposure can only be set to a certain value due to framerate limitations.
 *
 *  This leads to following workflow:
 *
 *  Determine image brightness by taking sample pixel and analyzing them.
 *  If the brightness is not our prefered area, we have to adjust.
 *  Calculate a new gain value. Since we prefer it small we only set it if it is smaller to the current one.
 *  Next we calculate exposure
 *  If exposure can not be moved, we increase gain to the already calculated value.
 *
 *  In every case we check if exposure and gain can be switched,
 *  meaning we try to reduce gain by increasing exposure.
 *
 *
 *  Currently v4l2 and aravis compatible cameras are supported.
 *
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef ENABLE_ARAVIS
#include <arv.h>
#endif

#include <math.h>
#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include "gsttis_auto_exposure.h"
#include <linux/videodev2.h>
#include <sys/ioctl.h>

#include "bayer.h"
#include "image_sampling.h"

GST_DEBUG_CATEGORY_STATIC (gst_tis_auto_exposure_debug_category);
#define GST_CAT_DEFAULT gst_tis_auto_exposure_debug_category

#define CLIP(val,l,h) ( (val) < (l) ? (l) : (val) > (h) ? (h) : (val) )


/* prototypes */

static void gst_tis_auto_exposure_set_property (GObject* object,
                                                guint property_id,
                                                const GValue* value,
                                                GParamSpec* pspec);
static void gst_tis_auto_exposure_get_property (GObject* object,
                                                guint property_id,
                                                GValue* value,
                                                GParamSpec* pspec);
static void gst_tis_auto_exposure_finalize (GObject* object);

static GstFlowReturn gst_tis_auto_exposure_transform_ip (GstBaseTransform* trans,
                                                         GstBuffer* buf);
static GstCaps* gst_tis_auto_exposure_transform_caps (GstBaseTransform* trans,
                                                      GstPadDirection direction,
                                                      GstCaps* caps);

static void gst_tis_auto_exposure_fixate_caps (GstBaseTransform* base,
                                               GstPadDirection direction,
                                               GstCaps* caps,
                                               GstCaps* othercaps);
static void init_camera_resources (GstTis_Auto_Exposure* self);


enum
{
    PROP_0,
    PROP_AUTO_EXPOSURE,
    PROP_AUTO_GAIN,
    PROP_CAMERA,
    PROP_EXPOSURE_MAX,
    PROP_GAIN_MAX,
};

/* pad templates */

static GstStaticPadTemplate gst_tis_auto_exposure_sink_template =
    GST_STATIC_PAD_TEMPLATE ("sink",
                             GST_PAD_SINK,
                             GST_PAD_ALWAYS,
                             GST_STATIC_CAPS ("ANY"));

static GstStaticPadTemplate gst_tis_auto_exposure_src_template =
    GST_STATIC_PAD_TEMPLATE ("src",
                             GST_PAD_SRC,
                             GST_PAD_ALWAYS,
                             GST_STATIC_CAPS ("ANY"));


/* class initialization */

G_DEFINE_TYPE_WITH_CODE (GstTis_Auto_Exposure,
                         gst_tis_auto_exposure,
                         GST_TYPE_BASE_TRANSFORM,
                         GST_DEBUG_CATEGORY_INIT (gst_tis_auto_exposure_debug_category,
                                                  "tis_auto_exposure",
                                                  0,
                                                  "debug category for tis_auto_exposure element"));

static void gst_tis_auto_exposure_class_init (GstTis_Auto_ExposureClass* klass)
{
    GObjectClass* gobject_class = G_OBJECT_CLASS (klass);
    GstBaseTransformClass* base_transform_class = GST_BASE_TRANSFORM_CLASS (klass);

    gst_element_class_add_static_pad_template (GST_ELEMENT_CLASS(klass),
                                               &gst_tis_auto_exposure_src_template);
    gst_element_class_add_static_pad_template (GST_ELEMENT_CLASS(klass),
                                               &gst_tis_auto_exposure_sink_template);

    gst_element_class_set_details_simple (GST_ELEMENT_CLASS(klass),
                                          "The Imaging Source Brightness Balance Element",
                                          "Generic",
                                          "Adjusts the image brightness by setting camera properties.",
                                          "Edgar Thier <edgarthier@gmail.com>");

    gobject_class->set_property = gst_tis_auto_exposure_set_property;
    gobject_class->get_property = gst_tis_auto_exposure_get_property;
    gobject_class->finalize = gst_tis_auto_exposure_finalize;
    base_transform_class->transform_ip = GST_DEBUG_FUNCPTR (gst_tis_auto_exposure_transform_ip);
    base_transform_class->transform_caps = GST_DEBUG_FUNCPTR (gst_tis_auto_exposure_transform_caps);
    base_transform_class->fixate_caps = GST_DEBUG_FUNCPTR (gst_tis_auto_exposure_fixate_caps);

    g_object_class_install_property (gobject_class,
                                     PROP_AUTO_EXPOSURE,
                                     g_param_spec_boolean ("auto-exposure",
                                                           "Auto Exposure Balance",
                                                           "Automatically adjust exposure balance",
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
                                     PROP_AUTO_GAIN,
                                     g_param_spec_boolean ("auto-gain",
                                                           "Auto Gain",
                                                           "Automatically adjust gain",
                                                           TRUE,
                                                           G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    g_object_class_install_property (gobject_class,
                                     PROP_GAIN_MAX,
                                     g_param_spec_double ("gain-max",
                                                          "Gain Maximum",
                                                          "Maximum value gain can take",
                                                          0.0, G_MAXDOUBLE, 0.0,
                                                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    g_object_class_install_property (gobject_class,
                                     PROP_CAMERA,
                                     g_param_spec_object ("camera",
                                                          "camera gst element",
                                                          "Gstreamer element that shall be manipulated",
                                                          GST_TYPE_ELEMENT,
                                                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}

static void gst_tis_auto_exposure_init (GstTis_Auto_Exposure *self)
{
    self->auto_exposure = TRUE;
    self->auto_gain = TRUE;

    self->frame_counter = 0;
    self->camera_src = NULL;
}

void gst_tis_auto_exposure_set_property (GObject* object,
                                         guint property_id,
                                         const GValue* value,
                                         GParamSpec* pspec)
{
    GstTis_Auto_Exposure* tis_auto_exposure = GST_TIS_AUTO_EXPOSURE (object);

    switch (property_id)
    {
        case PROP_AUTO_EXPOSURE:
            tis_auto_exposure->auto_exposure = g_value_get_boolean (value);
            break;
        case PROP_AUTO_GAIN:
            tis_auto_exposure->auto_gain = g_value_get_boolean(value);
            break;
        case PROP_CAMERA:
            tis_auto_exposure->camera_src = g_value_get_object (value);
            break;
        case PROP_EXPOSURE_MAX:
            tis_auto_exposure->exposure.max = g_value_get_double (value);
            if (tis_auto_exposure->exposure.max == 0.0)
                tis_auto_exposure->exposure = tis_auto_exposure->default_exposure_values;
            break;
        case PROP_GAIN_MAX:
            tis_auto_exposure->gain.max = g_value_get_double (value);
            if (tis_auto_exposure->gain.max == 0.0)
                tis_auto_exposure->gain = tis_auto_exposure->default_gain_values;
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

void gst_tis_auto_exposure_get_property (GObject* object,
                                         guint property_id,
                                         GValue* value,
                                         GParamSpec* pspec)
{
    GstTis_Auto_Exposure* tis_auto_exposure = GST_TIS_AUTO_EXPOSURE (object);

    switch (property_id)
    {
        case PROP_AUTO_EXPOSURE:
            g_value_set_boolean (value, tis_auto_exposure->auto_exposure);
            break;
        case PROP_AUTO_GAIN:
            g_value_set_boolean (value, tis_auto_exposure->auto_gain);
            break;
        case PROP_CAMERA:
            g_value_set_object (value, tis_auto_exposure->camera_src);
            break;
        case PROP_EXPOSURE_MAX:
            g_value_set_double(value, tis_auto_exposure->exposure.max);
            break;
        case PROP_GAIN_MAX:
            g_value_set_double(value, tis_auto_exposure->gain.max);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

void gst_tis_auto_exposure_finalize (GObject* object)
{
    G_OBJECT_CLASS (gst_tis_auto_exposure_parent_class)->finalize (object);
}

static GstCaps* gst_tis_auto_exposure_transform_caps (GstBaseTransform* trans,
                                                      GstPadDirection direction,
                                                      GstCaps* caps)
{
    GstCaps *outcaps = gst_caps_copy (caps);

    if (GST_TIS_AUTO_EXPOSURE(trans)->camera_src != NULL)
        return outcaps;

    /* if camera_src is not set we assume that the first default camera src found shall be used */

    GstElement* e = GST_ELEMENT( gst_object_get_parent(GST_OBJECT(trans)));

    GList* l =  GST_BIN(e)->children;

    while (1==1)
    {

        const char* name = g_type_name(gst_element_factory_get_element_type (gst_element_get_factory(l->data)));

        if (g_strcmp0(name, "GstV4l2Src") == 0)
        {
            GST_TIS_AUTO_EXPOSURE(trans)->camera_src = l->data;
            break;
        }
        if (g_strcmp0(name, "GstAravis") == 0)
        {
            GST_TIS_AUTO_EXPOSURE(trans)->camera_src = l->data;
            break;
        }

        if (g_list_next(l) == NULL)
            break;

        l = g_list_next(l);
    }

    return outcaps;
}

static void gst_tis_auto_exposure_fixate_caps (GstBaseTransform* base,
                                               GstPadDirection direction,
                                               GstCaps* incoming,
                                               GstCaps* outgoing)
{
    GstTis_Auto_Exposure* self = GST_TIS_AUTO_EXPOSURE (base);

    GstStructure* ins;
    GstStructure* outs;
    gint width, height;
    g_return_if_fail (gst_caps_is_fixed (incoming));

    GST_DEBUG_OBJECT (base, "trying to fixate outgoing %" GST_PTR_FORMAT
                      " based on caps %" GST_PTR_FORMAT, outgoing, incoming);

    ins = gst_caps_get_structure (incoming, 0);
    outs = gst_caps_get_structure (outgoing, 0);

    gst_structure_get_fraction(ins, "framerate", &self->framerate_numerator, &self->framerate_denominator);

    if (gst_structure_get_int (ins, "width", &width))
    {
        if (gst_structure_has_field (outs, "width"))
        {
            gst_structure_fixate_field_nearest_int (outs, "width", width);
        }
    }

    if (gst_structure_get_int (ins, "height", &height))
    {
        if (gst_structure_has_field (outs, "height"))
        {
            gst_structure_fixate_field_nearest_int (outs, "width", height);
        }
    }

    const char* p = gst_structure_get_name (ins);
    guint fourcc;
    if (g_strcmp0(p, "video/x-raw-bayer") == 0)
    {
        self->color_format = BAYER;

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

    }
    else if (g_strcmp0(p, "video/x-raw-gray") == 0 )
    {
        self->color_format = GRAY;
    }
    else if (g_strcmp0(p, "video/x-raw-rgb") == 0 )
    {
        self->color_format = RGB;
    }
    else
    {
        self->color_format = UNDEFINED_FORMAT;
    }

    gst_debug_log (gst_tis_auto_exposure_debug_category,
                   GST_LEVEL_WARNING,
                   "tis_auto_exposure",
                   "gst_tis_auto_exposure_fixate_caps",
                   336,
                   NULL,
                   "Structure name %s\n", gst_structure_get_name (ins));

    init_camera_resources(self);
}

static void init_camera_resources (GstTis_Auto_Exposure* self)
{
    g_return_if_fail (self->camera_src != NULL);

    /* retrieve the element name e.g. GstAravis or GstV4l2Src*/
    const char* element_name = g_type_name(gst_element_factory_get_element_type (gst_element_get_factory(self->camera_src)));

    if (g_strcmp0 (element_name, CAMERASRC_NETWORK) == 0)
    {
        self->source_type = NETWORK;
        self->exposure.value = 0.0;
        self->gain.value = 0.0;

#ifdef ENABLE_ARAVIS

        ArvCamera* cam;
        g_object_get (G_OBJECT (self->camera_src), "camera", &cam, NULL);

        arv_camera_get_exposure_time_bounds(cam,
                                            &self->default_exposure_values.min,
                                            &self->default_exposure_values.max);
        arv_camera_get_gain_bounds(cam,
                                   &self->default_gain_values.min,
                                   &self->default_gain_values.max);
        /* do not free camera; it is just a pointer to the internally used object */

        /* all gige cameras use the same time unit, thus can be handled identically */
        /* 1 000 000 = 1s */
        self->default_exposure_values.max = 1000000 / (self->framerate_numerator / self->framerate_denominator);

#endif

    }
    else if (g_strcmp0(element_name, CAMERASRC_USB) == 0)
    {
        self->source_type = USB;
        self->exposure.value = 0.0;
        self->gain.value = 0.0;

        gint fd;
        g_object_get(G_OBJECT(self->camera_src), "device-fd", &fd, NULL);

        struct v4l2_queryctrl qctrl = { V4L2_CTRL_FLAG_NEXT_CTRL };
        struct v4l2_control ctrl = { 0 };

        while (ioctl(fd, VIDIOC_QUERYCTRL, &qctrl) == 0)
        {
            if (qctrl.id == V4L2_CID_GAIN)
            {

                ctrl.id = qctrl.id;
                if (ioctl(fd, VIDIOC_G_CTRL, &ctrl))
                {
                    continue;
                }
                self->gain.value= ctrl.value;
                self->default_gain_values.min = qctrl.minimum;
                self->default_gain_values.max = qctrl.maximum;

            }
            else if (qctrl.id == V4L2_CID_EXPOSURE_ABSOLUTE)
            {
                ctrl.id = qctrl.id;
                if (ioctl(fd, VIDIOC_G_CTRL, &ctrl))
                {
                    continue;
                }
                self->exposure.value = ctrl.value;
                self->default_exposure_values.min = qctrl.minimum;
                self->default_exposure_values.max = qctrl.maximum;
            }
            qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
        }
        /* we know the framerate and adjust the maximum for exposure */
        /* in order to always have exposure settings that are within range */

        /* exposure_absolute is treated as follows: */
        /* Determines the exposure time of the camera sensor.
           The exposure time is limited by the frame interval.
           Drivers should interpret the values as 100 µs units,
           where the value 1 stands for 1/10000th of a second,
           10000 for 1 second and 100000 for 10 seconds. */

        self->default_exposure_values.max = 10000 / (self->framerate_numerator / self->framerate_denominator);
    }

    if (self->gain.max == 0.0)
    {
        self->gain = self->default_gain_values;
    }
    else
    {
        self->gain.min = self->default_gain_values.min;
    }

    /* simply use the retrieved default values */
    if (self->exposure.max == 0.0)
    {
        self->exposure = self->default_exposure_values;
    }
    else
    {
        self->exposure.min = self->default_exposure_values.min;
    }

    gst_debug_log (gst_tis_auto_exposure_debug_category,
                   GST_LEVEL_INFO,
                   "tis_auto_exposure",
                   "init_camera_resources",
                   450,
                   NULL,
                   "Exposure boundaries are %f %f", self->exposure.min, self->exposure.max);

    gst_debug_log (gst_tis_auto_exposure_debug_category,
                   GST_LEVEL_INFO,
                   "tis_auto_exposure",
                   "init_camera_resources",
                   450,
                   NULL,
                   "Gain boundaries are %f %f", self->gain.min, self->gain.max);
}


static unsigned int calc_dist (unsigned int reference_val, unsigned int brightness_val)
{
    if ( brightness_val == 0 )
        brightness_val = 1;
    return (reference_val * dist_mid) / brightness_val;
}

static void set_exposure (GstTis_Auto_Exposure* self, gdouble exposure)
{
    gst_debug_log (gst_tis_auto_exposure_debug_category,
                   GST_LEVEL_INFO,
                   "tis_auto_exposure",
                   "set_exposure",
                   420,
                   NULL,
                   "Setting exposure to %f", exposure);

    if (self->source_type == NETWORK)
        g_object_set(G_OBJECT(self->camera_src), "exposure", exposure, NULL);
    else if (self->source_type == USB)
    {
        gint fd;
        g_object_get(G_OBJECT(self->camera_src), "device-fd", &fd, NULL);

        struct v4l2_control ctrl;

        ctrl.id = V4L2_CID_EXPOSURE_ABSOLUTE;
        ctrl.value = exposure;

        if(ioctl(fd, VIDIOC_S_CTRL, &ctrl) == -1)
        {
            gst_debug_log (gst_tis_auto_exposure_debug_category,
                           GST_LEVEL_ERROR,
                           "tis_auto_exposure",
                           "set_exposure",
                           442,
                           NULL,
                           "Unable to write exposure for USB device");
        }
    }
}

static gdouble calc_exposure (GstTis_Auto_Exposure* self, guint dist, gdouble exposure)
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

static void set_gain (GstTis_Auto_Exposure* self, gdouble gain)
{
    gst_debug_log (gst_tis_auto_exposure_debug_category,
                   GST_LEVEL_INFO,
                   "",
                   "",
                   666,
                   NULL,
                   "Setting gain to %f", gain);

    if (self->source_type == NETWORK)
    {
        g_object_set(G_OBJECT(self->camera_src), "gain", gain, NULL);
    }
    else if (self->source_type == USB)
    {
        gint fd;
        g_object_get(G_OBJECT(self->camera_src), "device-fd", &fd, NULL);

        struct v4l2_control ctrl;

        ctrl.id = V4L2_CID_GAIN;
        ctrl.value = gain;

        if (ioctl(fd, VIDIOC_S_CTRL, &ctrl) == -1)
            gst_debug_log (gst_tis_auto_exposure_debug_category,
                           GST_LEVEL_ERROR,
                           "tis_auto_exposure",
                           "set_gain",
                           493,
                           NULL,
                           "Unable to write gain for USB device");
    }
}

static gdouble calc_gain (GstTis_Auto_Exposure* self, guint dist)
{
    gdouble gain = self->gain.value;

    /* when we have to reduce, we reduce it faster */
    if ( dist >= dist_mid )
    {
        /* this dampens the change in dist factor */
        dist = ( dist + (dist_mid * 2) ) / 3;
    }
    double val = log( dist / (double)dist_mid ) / log( 2.0f );

    gain += (val * steps_to_double_brightness);

    return CLIP( gain, self->gain.min, self->gain.max );
}


void retrieve_current_values (GstTis_Auto_Exposure* self)
{
    if (self->source_type == NETWORK)
    {
        g_object_get(G_OBJECT(self->camera_src), "gain", &self->gain.value, NULL);
        g_object_get(G_OBJECT(self->camera_src), "exposure", &self->exposure.value, NULL);

    }
    else if (self->source_type == USB)
    {
        gint fd;
        g_object_get(G_OBJECT(self->camera_src), "device-fd", &fd, NULL);

        struct v4l2_control ctrl;

        ctrl.id = V4L2_CID_GAIN;

        if(ioctl(fd, VIDIOC_G_CTRL, &ctrl) == -1)
            gst_debug_log (gst_tis_auto_exposure_debug_category,
                           GST_LEVEL_ERROR,
                           "",
                           "",
                           666,
                           NULL,
                           "Unable to read gain from USB device.");
        self->gain.value = ctrl.value;

        ctrl.id = V4L2_CID_EXPOSURE_ABSOLUTE;

        if (ioctl(fd, VIDIOC_G_CTRL, &ctrl) == -1)
            gst_debug_log (gst_tis_auto_exposure_debug_category,
                           GST_LEVEL_ERROR,
                           "",
                           "",
                           666,
                           NULL,
                           "Unable to read exposure fromUSB device.");

        self->exposure.value = ctrl.value;
    }
}


static void correct_brightness (GstTis_Auto_Exposure* self, GstBuffer* buf)
{
    guint brightness = 0;
    if (self->color_format == BAYER)
    {
        brightness = image_brightness_bayer(buf, self->pattern);
    }
    else
    {
        brightness = buffer_brightness_gray(buf);
    }
    /* assure we have the current values */
    retrieve_current_values (self);

    /* get distance from optimum */
    guint dist = calc_dist(ref_val, brightness);

    if (dist < 98 || dist > 102)
    {
        /* set_gain */
        gdouble new_gain = 0.0;

        if (self->auto_gain == TRUE)
        {

            new_gain = calc_gain(self, dist);
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

/*
  Entry point for actual transformation
 */
static GstFlowReturn gst_tis_auto_exposure_transform_ip (GstBaseTransform* trans, GstBuffer* buf)
{
    GstTis_Auto_Exposure* self = GST_TIS_AUTO_EXPOSURE (trans);

    if (!self->auto_exposure && !self->auto_gain)
    {
        return GST_FLOW_OK;
    }

    if (self->frame_counter > 3)
    {
        correct_brightness(self, buf);
        self->frame_counter = 0;
    }
    self->frame_counter++;

    return GST_FLOW_OK;
}

static gboolean plugin_init (GstPlugin * plugin)
{
    return gst_element_register (plugin,
                                 "tis_auto_exposure",
                                 GST_RANK_NONE,
                                 GST_TYPE_TIS_AUTO_EXPOSURE);
}

#ifndef VERSION
#define VERSION "0.0.1"
#endif
#ifndef PACKAGE
#define PACKAGE "tis_auto_exposure_package"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "tis_auto_exposure_package_name"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "https://github.com/TheImagingSource/tiscamera"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
                   GST_VERSION_MINOR,
                   "tis_auto_exposure",
                   "The Imaging Source white balance plugin",
                   plugin_init, VERSION,
                   "LGPL",
                   PACKAGE_NAME, GST_PACKAGE_ORIGIN)

