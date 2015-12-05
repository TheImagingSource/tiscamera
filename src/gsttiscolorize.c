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

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include "gsttiscolorize.h"

#include <string.h>
#include "bayer.h"
#include <libudev.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/videodev2.h>


GST_DEBUG_CATEGORY_STATIC (gst_tiscolorize_debug_category);
#define GST_CAT_DEFAULT gst_tiscolorize_debug_category

/* prototypes */


static void gst_tiscolorize_set_property (GObject* object,
                                          guint property_id,
                                          const GValue* value,
                                          GParamSpec* pspec);

static void gst_tiscolorize_get_property (GObject* object,
                                          guint property_id,
                                          GValue* value,
                                          GParamSpec* pspec);

static void gst_tiscolorize_finalize (GObject* object);

static GstFlowReturn gst_tiscolorize_transform_ip (GstBaseTransform* trans, GstBuffer* buf);
static GstCaps* gst_tiscolorize_transform_caps (GstBaseTransform* trans,
                                                GstPadDirection direction,
                                                GstCaps* caps);

static void gst_tiscolorize_fixate_caps (GstBaseTransform* base,
                                         GstPadDirection direction,
                                         GstCaps* caps,
                                         GstCaps* othercaps);

enum
{
    PROP_0,
    PROP_PATTERN,
};

/* pad templates */

static GstStaticPadTemplate gst_tiscolorize_sink_template =
    GST_STATIC_PAD_TEMPLATE ("sink",
                             GST_PAD_SINK,
                             GST_PAD_ALWAYS,
                             GST_STATIC_CAPS ("video/x-raw-gray,bpp=8,framerate=(fraction)[0/1,1000/1],width=[1,MAX],height=[1,MAX]")
        );

static GstStaticPadTemplate gst_tiscolorize_src_template =
    GST_STATIC_PAD_TEMPLATE ("src",
                             GST_PAD_SRC,
                             GST_PAD_ALWAYS,
                             GST_STATIC_CAPS ("video/x-raw-bayer,format=(string){bggr,grbg,gbrg,rggb},framerate=(fraction)[0/1,MAX],width=[1,MAX],height=[1,MAX]")
        );


/* class initialization */

G_DEFINE_TYPE_WITH_CODE (GstTisColorize, gst_tiscolorize, GST_TYPE_BASE_TRANSFORM,
                         GST_DEBUG_CATEGORY_INIT (gst_tiscolorize_debug_category, "tiscolorize", 0,
                                                  "debug category for tiscolorize element"));


static void gst_tiscolorize_class_init (GstTisColorizeClass* klass)
{
    GObjectClass* gobject_class = G_OBJECT_CLASS (klass);
    GstBaseTransformClass* base_transform_class = GST_BASE_TRANSFORM_CLASS (klass);

    /* Setting up pads and setting metadata should be moved to
       base_class_init if you intend to subclass this class. */

    gst_element_class_add_static_pad_template (GST_ELEMENT_CLASS(klass),
                                               &gst_tiscolorize_src_template);
    gst_element_class_add_static_pad_template (GST_ELEMENT_CLASS(klass),
                                               &gst_tiscolorize_sink_template);


    gobject_class->set_property = gst_tiscolorize_set_property;
	gobject_class->get_property = gst_tiscolorize_get_property;
    gobject_class->finalize = gst_tiscolorize_finalize;
    base_transform_class->transform_ip = GST_DEBUG_FUNCPTR (gst_tiscolorize_transform_ip);
    base_transform_class->transform_caps = GST_DEBUG_FUNCPTR (gst_tiscolorize_transform_caps);
    base_transform_class->fixate_caps = GST_DEBUG_FUNCPTR (gst_tiscolorize_fixate_caps);

    gst_element_class_set_details_simple (GST_ELEMENT_CLASS(klass),
                                          "The Imaging Source White Balance Element",
                                          "Generic",
                                          "Adjusts white balancing of RAW video data buffers",
                                          "Arne Caspari <arne.caspari@gmail.com>");

	g_object_class_install_property (gobject_class,
                                     PROP_PATTERN,
                                     g_param_spec_string ("bayer-pattern",
                                                          "Bayer Pattern",
                                                          "Bayer Pattern to use:",
                                                          NULL,
                                                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

}



void gst_tiscolorize_set_property (GObject* object,
                                   guint property_id,
                                   const GValue* value,
                                   GParamSpec* pspec)
{
    GstTisColorize* self = GST_TISCOLORIZE (object);

    switch (property_id)
    {
        case PROP_PATTERN:
            g_free(self->pattern);
            self->pattern = g_strdup(g_value_get_string( value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}


void gst_tiscolorize_get_property (GObject* object,
                                     guint property_id,
                                     GValue* value,
                                     GParamSpec* pspec)
{
    GstTisColorize* self = GST_TISCOLORIZE (object);

    switch (property_id)
    {
        case PROP_PATTERN:
            g_value_set_string (value, self->pattern);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}



static void gst_tiscolorize_init (GstTisColorize* self)
{
    self->i = 0;
    self->pattern = NULL;
}


void gst_tiscolorize_finalize (GObject* object)
{
    GstTisColorize* self = (GstTisColorize*) object;
    g_free(self->pattern);

    G_OBJECT_CLASS (gst_tiscolorize_parent_class)->finalize (object);
}


typedef struct
{
    const char* device_name;
    unsigned int device_id;
    tBY8Pattern bayer_pattern;
} dev_info;


typedef struct
{
    char devnode [100];
    unsigned int productid;
    unsigned int serial;
} usb_cam;


dev_info device_info [] =
{
    {
        "DFx 72BUC02",
        8307,
        GR
    },
    {
        "DFx 72BUC02",
        8207,
        GR
    },
    {
        "DFx 22BUC02",
        8302,
        GB
    },
    {
        "DFx 22BUC02",
        8202,
        GB
    },
    {
        "DFx 42UC03",
        8308,
        GR
    },
    {
        "DFx 42UC03",
        8208,
        GR
    }
};


#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))

void get_usb_cameras (usb_cam (*cameras)[], int* camera_count)
{

    struct udev* udev;
    struct udev_enumerate* enumerate;
    struct udev_list_entry* devices;
    struct udev_list_entry* dev_list_entry;
    struct udev_device* dev;

    int camera_cnt = 0;
    (*camera_count) = 0;

    udev = udev_new();
    if (!udev)
    {
        gst_debug_log (gst_tiscolorize_debug_category,
                       GST_LEVEL_ERROR,
                       "tiscolorize",
                       "get_usbs_cameras",
                       184,
                       NULL,
                       "Unable to create udev object");

        return;
    }

    /* Create a list of the devices in the 'video4linux' subsystem. */
    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "video4linux");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(dev_list_entry, devices)
    {
        const char* path;
        char needed_path[100];

        /* Get the filename of the /sys entry for the device
           and create a udev_device object (dev) representing it */
        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);

        /* The device pointed to by dev contains information about
           the hidraw device. In order to get information about the
           USB device, get the parent device with the
           subsystem/devtype pair of "usb"/"usb_device". This will
           be several levels up the tree, but the function will find
           it.*/

        /* we need to copy the devnode (/dev/videoX) before the path
           is changed to the path of the usb device behind it (/sys/class/....) */
        strcpy(needed_path, udev_device_get_devnode(dev));

        dev = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");

        if (!dev)
        {
            gst_debug_log (gst_tiscolorize_debug_category,
                           GST_LEVEL_ERROR,
                           "tiscolorize",
                           "get_usbs_cameras",
                           184,
                           NULL,
                           "Unable to retrieve usb device for %s", needed_path);
            return;
        }

        /* From here, we can call get_sysattr_value() for each file
           in the device's /sys entry. The strings passed into these
           functions (idProduct, idVendor, serial, etc.) correspond
           directly to the files in the directory which represents
           the USB device. Note that USB strings are Unicode, UCS2
           encoded, but the strings returned from
           udev_device_get_sysattr_value() are UTF-8 encoded. */

        if (strcmp(udev_device_get_sysattr_value(dev, "idVendor"), "199e") == 0)
        {

            strcpy((*cameras)[camera_cnt].devnode, needed_path);
            (*cameras)[camera_cnt].productid = atoi(udev_device_get_sysattr_value(dev, "idProduct"));
            (*cameras)[camera_cnt].serial = atoi(udev_device_get_sysattr_value(dev, "serial"));

            gst_debug_log (gst_tiscolorize_debug_category,
                           GST_LEVEL_INFO,
                           "tiscolorize",
                           "get_usbs_cameras",
                           250,
                           NULL,
                           "Found v4l2 device %s", needed_path);

            camera_cnt++;
        }
        udev_device_unref(dev);
    }

    (*camera_count) = camera_cnt;
    /* Free the enumerator object */
    udev_enumerate_unref(enumerate);

    udev_unref(udev);

}


int get_product_id (char* devnode)
{
    usb_cam cameras [100];
    int camera_cnt = 0;
    get_usb_cameras(&cameras, &camera_cnt);
    int i;
    for (i = 0; i < camera_cnt; ++i)
    {
        if (strcmp(devnode, cameras[i].devnode) == 0)
        {
            gst_debug_log (gst_tiscolorize_debug_category,
                           GST_LEVEL_INFO,
                           "tiscolorize",
                           "get_product_id",
                           280,
                           NULL,
                           "Found product id %d for device %s", cameras[i].productid, cameras[i].devnode);

            return cameras[i].productid;
        }
    }
    return 0;
}


char* device_bayer_pattern (int id)
{
    unsigned int i;
    for (i = 0; i < ARRAYSIZE(device_info); i++)
    {
        if (device_info[i].device_id == id)
        {
            return bayer_to_string(device_info[i].bayer_pattern);
        }
    }

    return NULL;
}


int check_bayer_pattern (char* desc)
{
    if (strcmp(desc, "47425247-0000-0010-8000-00aa003") == 0 || strcmp(desc, "GRBG Bayer (GRBG)") == 0)
    {
        return GR;
    }
    else if (strcmp(desc, "42474752-0000-0010-8000-00aa003") == 0 || strcmp(desc, "RGGB Bayer (RGGB)") == 0)
    {
        return RG;
    }
    else if (strcmp(desc, "47524247-0000-0010-8000-00aa003") == 0 || strcmp(desc, "GBRG Bayer (GBRG)") == 0)
    {
        return GB;
    }
    else if (strcmp(desc, "31384142-0000-0010-8000-00aa003") == 0 || strcmp(desc, "BGGR Bayer (BGGR)") == 0 || strcmp(desc, "BGGR Bayer (BA81)") == 0)
    {
        return BG;
    }
    else if (strcmp(desc, "36314142-0000-0010-8000-00aa003") == 0 || strcmp(desc, "GRBG Bayer (GRBG)") == 0) 
    {
        return GR;
    }
    else
    {
        return -1;
    }
}


static GstCaps* gst_tiscolorize_transform_caps (GstBaseTransform* trans,
                                                GstPadDirection direction,
                                                GstCaps* caps)
{
    GstStructure* s;
    GstCaps *outcaps;

    outcaps = gst_caps_copy (caps);
    s = gst_caps_get_structure (outcaps, 0);

    GstTisColorize* self = (GstTisColorize*) trans;

    if (direction == GST_PAD_SINK)
    {

        if (self->pattern != NULL)
        {

            gst_debug_log (gst_tiscolorize_debug_category,
                           GST_LEVEL_INFO,
                           "tiscolorize",
                           "gst_tiscolorize_transform_caps",
                           __LINE__,
                           NULL,
                           "Setting format to \"%s\". As defined by user input.", self->pattern);
            gst_structure_set_name (s, "video/x-raw-bayer");
            gst_structure_set (s, "format", G_TYPE_STRING, self->pattern, NULL);

            return outcaps;
        }

        /* Here we get the parent element of the gstcolorize element (i.e. the pipeline),
           iterate all its children and search for the v4l2src element.

           When we have the gstelement we take the device description (/dev/videoX)
           and use it to iterate all udev v4l devices to find the according usb device
           and retrieve the idProduct as a unique identifier. */

        char* dev;

        GstElement* e = GST_ELEMENT( gst_object_get_parent(GST_OBJECT(trans)));

        GList* l = GST_BIN(e)->children;

        while (1==1)
        {
            const char* name = g_type_name(gst_element_factory_get_element_type (gst_element_get_factory(l->data)));

            if (g_strcmp0(name, "GstV4l2Src") == 0)
            {
                g_object_get(G_OBJECT(l->data), "device", &dev, NULL);
            }

            if (g_list_next(l) == NULL)
            {
                break;
            }

            l = g_list_next(l);
        }
        gst_structure_set_name (s, "video/x-raw-bayer");

        /* Here we simply fake the whole thing.
           On the first run we are unable to determine the correct pattern
           because GstV4l2src has not yet initialized the device and we therefor cannot
           ask the device anything (including its id).
           We pass one random pattern to allow a correct pipeline build up.
           On the second round the device is correctly initialized. Now we ask its id and
           use it to look up the correct pattern. */

        GstTisColorize *tiscolorize = GST_TISCOLORIZE (trans);
        if (tiscolorize->i == 0)
        {

            tiscolorize->i++;

            gst_debug_log (gst_tiscolorize_debug_category,
                           GST_LEVEL_INFO,
                           "tiscolorize",
                           "gst_tiscolorize_transform_caps",
                           365,
                           NULL,
                           "Setting format to grbg while waiting for device init.");

            gst_structure_set (s, "format", G_TYPE_STRING, "grbg", NULL);

        }
        else
        {
            /* Check the format descriptions v4l2 offers.
               Iterate them and look for one matching a bayer pattern description.
               If none are present fall back to the udev variant where the bayer pattern
               is found through the productid */

            struct v4l2_fmtdesc fmtdesc;
            fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

            int dev_fd = open(dev, O_RDONLY);

            if (dev_fd == -1)
            {

            }

            char* bayer_pattern = NULL;

            for (fmtdesc.index = 0; ! ioctl (dev_fd, VIDIOC_ENUM_FMT, &fmtdesc); fmtdesc.index ++)
            {
                int pattern = check_bayer_pattern((char*)fmtdesc.description);

                gst_debug_log (gst_tiscolorize_debug_category,
                               GST_LEVEL_DEBUG,
                               "tiscolorize",
                               "gst_tiscolorize_transform_caps",
                               __LINE__,
                               NULL,
                               "Checked format: %s ", (char*)fmtdesc.description);

                if (pattern != -1)
                {
                    bayer_pattern = bayer_to_string(pattern);
                }
            }

            close(dev_fd);

            if (bayer_pattern == NULL)
            {
                int id = get_product_id(dev);
                bayer_pattern = device_bayer_pattern(id);
            }

            if (bayer_pattern == NULL)
            {
                gst_debug_log (gst_tiscolorize_debug_category,
                               GST_LEVEL_ERROR,
                               "tiscolorize",
                               "gst_tiscolorize_transform_caps",
                               447,
                               NULL,
                               "Unable to determine bayer pattern");
            }

            gst_structure_set (s, "format", G_TYPE_STRING, bayer_pattern, NULL);

            gst_debug_log (gst_tiscolorize_debug_category,
                           GST_LEVEL_INFO,
                           "tiscolorize",
                           "gst_tiscolorize_transform_caps",
                           380,
                           NULL,
                           "Setting real format for device %s to %s", dev, bayer_pattern);

        }
        gst_structure_remove_fields (s, "bpp", "depth", NULL);
    }
    else
    {
        gst_structure_set_name (s, "video/x-raw-gray");
        gst_structure_set (s, "bpp", G_TYPE_INT, 8, NULL);
        gst_structure_remove_field (s, "format");
    }

    GST_LOG_OBJECT (trans, "Transform caps\n\nin:%"GST_PTR_FORMAT"\nout:%"GST_PTR_FORMAT, caps, outcaps);

    return outcaps;
}


static void gst_tiscolorize_fixate_caps (GstBaseTransform* base,
                                         GstPadDirection direction,
                                         GstCaps* caps,
                                         GstCaps* othercaps)
{
    GstStructure* ins;
    GstStructure* outs;
    gint width;
    gint height;
    g_return_if_fail (gst_caps_is_fixed (caps));

    GST_DEBUG_OBJECT (base, "trying to fixate othercaps %" GST_PTR_FORMAT
                      " based on caps %" GST_PTR_FORMAT, othercaps, caps);

    ins = gst_caps_get_structure (caps, 0);
    outs = gst_caps_get_structure (othercaps, 0);

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

}


static GstFlowReturn gst_tiscolorize_transform_ip (GstBaseTransform* trans, GstBuffer* buf)
{
    /* We change the caps automatically in the background. */
    /* Here we simply say everything is OK and go on. */

    return GST_FLOW_OK;
}


static gboolean plugin_init (GstPlugin* plugin)
{
    return gst_element_register (plugin, "tiscolorize", GST_RANK_NONE, GST_TYPE_TISCOLORIZE);
}

#ifndef VERSION
#define VERSION "0.0.1"
#endif
#ifndef PACKAGE
#define PACKAGE "tiscolorize_package"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "tiscolorize_package_name"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "https://github.com/TheImagingSource/tiscamera"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
                   GST_VERSION_MINOR,
                   "tiscolorize",
                   "The Imaging Source white balance plugin",
                   plugin_init,
                   VERSION,
                   "LGPL",
                   PACKAGE_NAME,
                   GST_PACKAGE_ORIGIN)
