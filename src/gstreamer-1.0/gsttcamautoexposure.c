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

#include "tcam_c.h"

#include <math.h>
#include <stdlib.h>
#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include "gsttcamautoexposure.h"
#include <linux/videodev2.h>
#include <sys/ioctl.h>

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
    PROP_EXPOSURE_MAX,
    PROP_GAIN_MAX,
};

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

G_DEFINE_TYPE_WITH_CODE (GstTcamautoexposure,
                         gst_tcamautoexposure,
                         GST_TYPE_BASE_TRANSFORM,
                         GST_DEBUG_CATEGORY_INIT (gst_tcamautoexposure_debug_category,
                                                  "tcamautoexposure",
                                                  0,
                                                  "debug category for tcamautoexposure element"));

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

    gobject_class->set_property = gst_tcamautoexposure_set_property;
    gobject_class->get_property = gst_tcamautoexposure_get_property;
    gobject_class->finalize = gst_tcamautoexposure_finalize;
    base_transform_class->transform_ip = gst_tcamautoexposure_transform_ip;
//    base_transform_class->transform_caps = gst_tcamautoexposure_transform_caps;
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
            tcamautoexposure->camera_src = g_value_get_object(value);
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
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

void gst_tcamautoexposure_get_property (GObject* object,
                                        guint    property_id,
                                        GValue*  value,
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
    else if (g_strcmp0(element_name, CAMERASRC_TCAM) == 0)
    {
        self->source_type = TCAM;

        tcam_capture_device* dev = NULL;

        g_object_get(G_OBJECT(self->camera_src), "camera", &dev, NULL);

        struct tcam_device_property p = {};

        bool ret = tcam_capture_device_find_property(dev, TCAM_PROPERTY_EXPOSURE, &p);

        if (!ret)
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
            self->exposure.min   = p.value.i.min;
            self->exposure.max   = p.value.i.max;
            self->exposure.value = p.value.i.value;

            self->default_exposure_values.max = 10000 / (self->framerate_numerator / self->framerate_denominator);

        }


        ret = tcam_capture_device_find_property(dev, TCAM_PROPERTY_GAIN, &p);

        if (!ret)
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
            self->gain.min   = p.value.i.min;
            self->gain.max   = p.value.i.max;
            self->gain.value = p.value.i.value;
        }
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
            gst_debug_log (gst_tcamautoexposure_debug_category,
                           GST_LEVEL_ERROR,
                           "tcamautoexposure",
                           "set_exposure",
                           __LINE__,
                           NULL,
                           "Unable to write exposure for USB device");
        }
    }
    else if (self->source_type == TCAM)
    {
        struct tcam_capture_device* dev;
        g_object_get(G_OBJECT(self->camera_src), "camera", &dev, NULL);

        struct tcam_device_property exp = {};

        exp.id = TCAM_PROPERTY_EXPOSURE;
        exp.value.i.value = exposure;

        tcam_capture_device_set_property(dev, &exp);
    }
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
        {
            gst_debug_log (gst_tcamautoexposure_debug_category,
                           GST_LEVEL_ERROR,
                           "tcamautoexposure",
                           "set_gain",
                           __LINE__,
                           NULL,
                           "Unable to write gain for USB device");
        }
    }
    else if (self->source_type == TCAM)
    {
        tcam_capture_device* dev;
        g_object_get(G_OBJECT(self->camera_src), "camera", &dev, NULL);

        struct tcam_device_property exp = {};

        exp.id = TCAM_PROPERTY_GAIN;
        exp.value.i.value = gain;

        tcam_capture_device_set_property(dev, &exp);
    }
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
            gst_debug_log (gst_tcamautoexposure_debug_category,
                           GST_LEVEL_ERROR,
                           "",
                           "",
                           __LINE__,
                           NULL,
                           "Unable to read gain from USB device.");
        self->gain.value = ctrl.value;

        ctrl.id = V4L2_CID_EXPOSURE_ABSOLUTE;

        if (ioctl(fd, VIDIOC_G_CTRL, &ctrl) == -1)
            gst_debug_log (gst_tcamautoexposure_debug_category,
                           GST_LEVEL_ERROR,
                           "",
                           "",
                           __LINE__,
                           NULL,
                           "Unable to read exposure from USB device.");

        self->exposure.value = ctrl.value;
    }
    else if (self->source_type == TCAM)
    {
        tcam_capture_device* dev = NULL;
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

        bool ret = tcam_capture_device_find_property(dev, TCAM_PROPERTY_EXPOSURE, &p);

        if (ret == false)
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
            gst_debug_log (gst_tcamautoexposure_debug_category,
                           GST_LEVEL_DEBUG,
                           "",
                           "",
                           __LINE__,
                           NULL,
                           "Current exposure is %d", p.value.i.value);
            self->exposure.value = p.value.i.value;
        }

        ret = tcam_capture_device_find_property(dev, TCAM_PROPERTY_GAIN, &p);

        if (!ret)
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
            gst_debug_log (gst_tcamautoexposure_debug_category,
                           GST_LEVEL_DEBUG,
                           "",
                           "",
                           __LINE__,
                           NULL,
                           "Current gain is %d", p.value.i.value);
            self->gain.value = p.value.i.value;
        }
    }
    else
    {
        gst_debug_log (gst_tcamautoexposure_debug_category,
                       GST_LEVEL_ERROR,
                       "",
                       "",
                       __LINE__,
                       NULL,
                       "Unable to determine source type");

    }
}


static void correct_brightness (GstTcamautoexposure* self, GstBuffer* buf)
{
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
        brightness = image_brightness_bayer(buf, self->pattern, self->image_size);
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
        brightness = buffer_brightness_gray(buf, self->image_size);
    }
    /* assure we have the current values */
    retrieve_current_values (self);

    /* get distance from optimum */
    guint dist = calc_dist(ref_val, brightness);

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
                           "Comparing gain: %f < %f", new_gain, self->gain.value);
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

    GList* l =  GST_BIN(e)->children;

    while (1==1)
    {
        const char* name = g_type_name(gst_element_factory_get_element_type (gst_element_get_factory(l->data)));

        if (g_strcmp0(name, "GstV4l2Src") == 0)
        {
            GST_TCAMAUTOEXPOSURE(trans)->camera_src = l->data;
            break;
        }
        if (g_strcmp0(name, "GstAravis") == 0)
        {
            GST_TCAMAUTOEXPOSURE(trans)->camera_src = l->data;
            break;
        }
        if (g_strcmp0(name, "GstTcam") == 0)
        {
            GST_TCAMAUTOEXPOSURE(trans)->source_type = TCAM;
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
    GstPad* pad  = GST_BASE_TRANSFORM_SINK_PAD(self);
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
                   plugin_init, VERSION,
                   "Proprietary",
                   PACKAGE_NAME, GST_PACKAGE_ORIGIN)
