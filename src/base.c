/*
 * Copyright 2015 The Imaging Source Europe GmbH
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

#include "base.h"

#include <linux/videodev2.h>
#include <sys/ioctl.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#ifdef ENABLE_ARAVIS
#include <arv.h>
#endif

/* names of gstreamer elements used for camera interaction */
static const char* CAMERASRC_NETWORK = "GstAravis";
static const char* CAMERASRC_USB = "GstV4l2Src";


struct device_resources find_source (GstElement* self)
{

    struct device_resources res = {};
    res.color.max = 255;

    /* if camera_src is not set we assume that the first default camera src found shall be used */

    GstElement* e = GST_ELEMENT( gst_object_get_parent(GST_OBJECT(self)));

    GList* l =  GST_BIN(e)->children;

    while (1==1)
    {

        const char* name = g_type_name(gst_element_factory_get_element_type (gst_element_get_factory(l->data)));

        if (g_strcmp0(name, CAMERASRC_USB) == 0)
        {
            GST_LOG_OBJECT(self, "Found v4l2 device");
            res.source_type = V4L2;
            res.source_element = l->data;
            break;
        }
        if (g_strcmp0(name, CAMERASRC_NETWORK) == 0)
        {
            GST_LOG_OBJECT(self, "Found aravis device");
            res.source_type = ARAVIS;
            res.source_element = l->data;
            break;
        }

        if (g_list_next(l) == NULL)
            break;

        l = g_list_next(l);
    }

    if (res.source_type == UNKNOWN)
    {
        GST_LOG_OBJECT(self, "Unknown source type");
    }

    update_device_resources (&res);

    return res;
}


static void update_v4l2_device (struct device_resources* res)
{
    gint fd;
    g_object_get(G_OBJECT(res->source_element), "device-fd", &fd, NULL);

    struct v4l2_queryctrl qctrl = { V4L2_CTRL_FLAG_NEXT_CTRL };
    struct v4l2_control ctrl = { 0 };

    while (ioctl(fd, VIDIOC_QUERYCTRL, &qctrl) == 0)
    {
        if (qctrl.id == V4L2_CID_GAIN)
        {
            ctrl.id = qctrl.id;
            if (ioctl(fd, VIDIOC_G_CTRL, &ctrl))
            {
                qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
                continue;
            }
            res->gain.value= ctrl.value;
            res->gain.min = qctrl.minimum;
            res->gain.max = qctrl.maximum;
        }
        else if (qctrl.id == V4L2_CID_EXPOSURE_ABSOLUTE)
        {
            ctrl.id = qctrl.id;
            if (ioctl(fd, VIDIOC_G_CTRL, &ctrl))
            {
                qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
                continue;
            }
            res->exposure.value = ctrl.value;
            res->exposure.min = qctrl.minimum;
            res->exposure.max = qctrl.maximum;
        }
        else if (qctrl.id == V4L2_CID_EUVC_GAIN_R || qctrl.id == V4L2_CID_EUVC_GAIN_G_OLD)
        {
            ctrl.id = qctrl.id;
            if (ioctl(fd, VIDIOC_G_CTRL, &ctrl) <= 0)
            {
                qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
                continue;
            }
            res->color.rgb.R = ctrl.value;
            res->color.max = qctrl.maximum;
            res->color.default_value = 36; // 36 for DFK 72
        }
        else if (qctrl.id == V4L2_CID_EUVC_GAIN_G || qctrl.id == V4L2_CID_EUVC_GAIN_G_OLD)
        {
            ctrl.id = qctrl.id;
            if (ioctl(fd, VIDIOC_G_CTRL, &ctrl))
            {
                qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
                continue;
            }
            res->color.rgb.G = ctrl.value;
            res->color.max = qctrl.maximum;
            res->color.default_value = 36; // 36 for DFK 72

        }
        else if (qctrl.id == V4L2_CID_EUVC_GAIN_B || qctrl.id == V4L2_CID_EUVC_GAIN_B_OLD)
        {
            ctrl.id = qctrl.id;
            if (ioctl(fd, VIDIOC_G_CTRL, &ctrl))
            {
                qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;

                continue;
            }
            res->color.rgb.B = ctrl.value;
            res->color.max = qctrl.maximum;
            res->color.default_value = 36; // 36 for DFK 72

        }


        // must come last
        qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
    }
}


void update_device_resources (struct device_resources* res)
{

    switch (res->source_type)
    {
        case V4L2:
        {
            update_v4l2_device(res);
            break;
        }
        case ARAVIS:
        {
            break;
        }
        case UNKNOWN:
        default:
        {
            break;
        }

    }
}


gboolean v4l2_set_rgb (struct device_resources* res)
{

    gint fd;
    g_object_get(G_OBJECT(res->source_element), "device-fd", &fd, NULL);

    struct v4l2_control ctrl = { 0 };
    ctrl.id = V4L2_CID_EUVC_GAIN_R;
    ctrl.value = res->color.rgb.R;

    if (ioctl(fd, VIDIOC_S_CTRL, &ctrl))
    {
        return FALSE;
    }

    ctrl.id = V4L2_CID_EUVC_GAIN_G;
    ctrl.value = res->color.rgb.G;

    if (ioctl(fd, VIDIOC_S_CTRL, &ctrl))
    {
        return FALSE;
    }
    ctrl.id = V4L2_CID_EUVC_GAIN_B;
    ctrl.value = res->color.rgb.B;

    if (ioctl(fd, VIDIOC_S_CTRL, &ctrl))
    {
        return FALSE;
    }


    return TRUE;
}


gboolean device_set_rgb (struct device_resources* res)
{

    return v4l2_set_rgb(res);
}
