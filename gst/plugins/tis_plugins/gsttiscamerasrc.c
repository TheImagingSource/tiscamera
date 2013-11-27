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
 * SECTION:element-gsttiscamerasrc
 *
 * The tiscamerasrc element does FIXME stuff.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v fakesrc ! tiscamerasrc ! FIXME ! fakesink
 * ]|
 * FIXME Describe what the pipeline does.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
//#include <libudev.h>

#include "gsttiscamerasrc.h"

GST_DEBUG_CATEGORY_STATIC (gst_tiscamerasrc_debug_category);
#define GST_CAT_DEFAULT gst_tiscamerasrc_debug_category

/* prototypes */


static void gst_tiscamerasrc_set_property (GObject * object,
					   guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_tiscamerasrc_get_property (GObject * object,
					   guint property_id, GValue * value, GParamSpec * pspec);
static void gst_tiscamerasrc_dispose (GObject * object);
static void gst_tiscamerasrc_finalize (GObject * object);
static gboolean
gst_tiscamerasrc_setcaps_function (GstPad *pad, GstCaps *caps);

static gboolean
gst_tiscamerasrc_create_elements (GstTisCameraSrc *self);

static GstPadLinkReturn
gst_tiscamerasrc_pad_link_function (GstPad *pad, GstPad *peer);

enum
{
	PROP_0,
	PROP_DEVICE,
	PROP_EXPOSURE,
	PROP_GAIN,
};

/* pad templates */
static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
								   GST_PAD_SRC,
								   GST_PAD_ALWAYS,
								   GST_STATIC_CAPS ("video/x-raw-rgb,bpp=32,depth=32,width=[1,2147483647],height=[1,2147483647],framerate=[0/1,2147483647/1]; video/x-raw-bayer; video/x-raw-gray")
	);

/* class initialization */

G_DEFINE_TYPE_WITH_CODE (GstTisCameraSrc, gst_tiscamerasrc, GST_TYPE_BIN,
			 GST_DEBUG_CATEGORY_INIT (gst_tiscamerasrc_debug_category, "tiscamerasrc", 0,
						  "debug category for tiscamerasrc element"));

static void
gst_tiscamerasrc_class_init (GstTisCameraSrcClass * klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	/* Setting up pads and setting metadata should be moved to
	   base_class_init if you intend to subclass this class. */
	gst_element_class_set_details_simple (GST_ELEMENT_CLASS(klass),
					      "The Imaging Source Camera Source Element", "Generic", "Camera source element for The Imaging Source Cameras",
					      "Arne Caspari <arne.caspari@gmail.com>");

	gst_element_class_add_pad_template (GST_ELEMENT_CLASS (klass),
					    gst_static_pad_template_get (&src_factory));

	gobject_class->set_property = gst_tiscamerasrc_set_property;
	gobject_class->get_property = gst_tiscamerasrc_get_property;
	gobject_class->dispose = gst_tiscamerasrc_dispose;
	gobject_class->finalize = gst_tiscamerasrc_finalize;
	gobject_class->set_property = gst_tiscamerasrc_set_property;
	gobject_class->get_property = gst_tiscamerasrc_get_property;
	gobject_class->dispose = gst_tiscamerasrc_dispose;
	gobject_class->finalize = gst_tiscamerasrc_finalize;

	g_object_class_install_property (gobject_class,
					 PROP_DEVICE,
					 g_param_spec_string ("device",
							      "device",
							      "Device location",
							      "/dev/video0", 
							      G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	g_object_class_install_property (gobject_class, 
					 PROP_EXPOSURE, 
					 g_param_spec_double ("exposure",
							      "exposure",
							      "Exposure",
							      0.0, 30.0, 0.03,
							      G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

	g_object_class_install_property (gobject_class, 
					 PROP_GAIN, 
					 g_param_spec_double ("gain",
							      "gain",
							      "Gain",
							      0, 255, 34,
							      G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
							      

}

static void
gst_tiscamerasrc_init (GstTisCameraSrc *tiscamerasrc)
{
	//GstElementClass *element_class = GST_ELEMENT_CLASS (tiscamerasrc);
	GstPadTemplate *tmpl;

	tmpl = gst_static_pad_template_get (&src_factory);
	tiscamerasrc->srcpad = 
		gst_ghost_pad_new_no_target_from_template ("src", tmpl);
	gst_object_unref (tmpl);
	gst_element_add_pad (GST_ELEMENT (tiscamerasrc), tiscamerasrc->srcpad); 
	gst_pad_set_setcaps_function (tiscamerasrc->srcpad, gst_tiscamerasrc_setcaps_function);
	
	gst_pad_set_link_function (tiscamerasrc->srcpad, gst_tiscamerasrc_pad_link_function);


	gst_tiscamerasrc_create_elements (tiscamerasrc);
}

static gboolean
gst_tiscamerasrc_setcaps_function (GstPad *pad, GstCaps *caps)
{
	/* GstStructure *s; */
	/* s = gst_caps_get_structure (caps, 0); */
	
	return TRUE;
}

static GstPadLinkReturn
gst_tiscamerasrc_pad_link_function (GstPad *pad, GstPad *peer)
{
	GstTisCameraSrc *self = GST_TISCAMERASRC (gst_pad_get_parent (pad));
	gboolean ret = GST_PAD_LINK_REFUSED;
	GstCaps *caps;
	GstStructure *s;
	caps = gst_pad_get_caps (peer);
	s = gst_caps_get_structure (caps, 0);
	GST_DEBUG_OBJECT (self, "Link Caps: %s", gst_caps_to_string (caps));
	GST_DEBUG_OBJECT (self, "Name: %s", gst_structure_get_name (s));


	gst_element_unlink_many (self->src, self->flt, self->debayer, self->identity, NULL);
	if (gst_structure_has_name (s, "video/x-raw-gray")){
		if (gst_element_link_many (self->src, self->flt, self->identity, NULL))
			ret = GST_PAD_LINK_OK;
	} else if (gst_structure_has_name (s, "video/x-raw-rgb")){
		if (gst_element_link_many (self->src, self->flt, self->wb, self->debayer, self->identity, NULL))
			ret = GST_PAD_LINK_OK;
	}
	
	return ret;
}



/* char **_get_tis_devices (const char* devfile) */
/* { */
/* 	struct udev *udev; */
/* 	struct udev_enumerate *enumerate; */
/* 	struct udev_list_entry *devices, *dev_list_entry; */
/* 	struct udev_device *dev; */
	
/* 	udev = udev_new (); */
/* 	if (!udev) */
/* 		return NULL; */

/* 	char **ret = g_new0 (char*, 17); */
/* 	gint i; */
	
	
/* 	enumerate = udev_enumerate_new (udev); */
/* 	udev_enumerate_add_match_subsystem (enumerate, "video4linux"); */
/* 	udev_enumerate_scan_devices (enumerate); */
/* 	devices = udev_enumerate_get_list_entry (enumerate); */
/* 	udev_list_entry_foreach (dev_list_entry, devices){ */
/* 		const char *path; */
		
/* 		path = udev_list_entry_get_name (dev_list_entry); */
/* 		dev = udev_device_new_from_syspath (udev, path); */

/* 		if (!strcmp (udev_device_get_devnode (dev), devfile)){ */
/* 			dev = udev_device_get_parent_with_subsystem_devtype (dev, "usb", "usb_device"); */
/* 			if (dev){ */
/* 				const char *serial; */
				
/* 				serial = udev_device_get_sysattr_value(dev, "serial"); */
/* 				if (serial){ */
/* 					ret = malloc (strlen (serial)+1); */
/* 					strcpy (ret, serial); */
/* 				} */
/* 				udev_device_unref (dev); */
/* 			} */
/* 		} */
/* 	} */
/* 	/\* Free the enumerator object *\/ */
/* 	udev_enumerate_unref(enumerate); */

/* 	udev_unref(udev); */

/* 	return ret; */
/* } */

static gboolean
gst_tiscamerasrc_create_elements (GstTisCameraSrc *self)
{
	GstPad *pad;

	self->src = gst_element_factory_make ("v4l2src", NULL);
	self->capsfilter = gst_element_factory_make ("capsfilter", NULL);
	self->flt = gst_element_factory_make ("tisvideobufferfilter", NULL);	
	self->wb = gst_element_factory_make ("tiswhitebalance", NULL);
	self->debayer = gst_element_factory_make ("bayer2rgb", NULL);
	self->identity = gst_element_factory_make ("identity", NULL);

	if (!self->src || !self->flt || !self->wb || !self->debayer || !self->identity){
		GST_ERROR_OBJECT (self, "Missing elements");
		goto error_out;
	}
	
	gst_bin_add_many (GST_BIN (self), self->src, self->flt, self->wb, self->debayer, self->identity, NULL);
	/* gst_element_link_many (self->src, self->flt, self->identity, NULL); */

	pad = gst_element_get_static_pad (self->identity, "src");
	if (!gst_ghost_pad_set_target (GST_GHOST_PAD (self->srcpad), pad)){
		GST_ERROR_OBJECT (self, "Failed to set ghost pad!");
		gst_element_unlink_many (self->src, self->flt, NULL);
		goto error_out;
	}
	gst_object_unref (pad);

	return TRUE;
	
  error_out:
	if (self->src)
		gst_object_unref (self->src);
	if (self->flt)
		gst_object_unref (self->flt);
	if (self->debayer)
		gst_object_unref (self->debayer);
	if (self->identity)
		gst_object_unref (self->identity);

	return FALSE;
}



void
gst_tiscamerasrc_set_property (GObject * object, guint property_id,
			       const GValue * value, GParamSpec * pspec)
{
	GstTisCameraSrc *self = GST_TISCAMERASRC (object);

	switch (property_id) {

	case PROP_DEVICE:
		if (self->device)
			g_free (self->device);
		self->device = g_strdup (g_value_get_string (value));
		g_object_set (self->src, "device", self->device, NULL);
		break;

	case PROP_EXPOSURE:
	{
		gint fd;
		struct v4l2_control ctrl;

		g_object_get (self->src, "device-fd", &fd, NULL);
		ctrl.id = V4L2_CID_EXPOSURE_ABSOLUTE;
		ctrl.value = g_value_get_double (value) * 10000;
		ioctl( fd, VIDIOC_S_CTRL, &ctrl );
	}
	break;

	case PROP_GAIN:
	{
		gint fd;
		struct v4l2_control ctrl;

		g_object_get (self->src, "device-fd", &fd, NULL);
		ctrl.id = V4L2_CID_GAIN;
		ctrl.value = g_value_get_double (value);
		ioctl( fd, VIDIOC_S_CTRL, &ctrl );
	}
	break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}

void
gst_tiscamerasrc_get_property (GObject * object, guint property_id,
			       GValue * value, GParamSpec * pspec)
{
	GstTisCameraSrc *self = GST_TISCAMERASRC (object);

	switch (property_id) {

	case PROP_DEVICE:
		g_value_set_string (value, self->device);
		break;
	case PROP_EXPOSURE:
	{
		gint fd;
		struct v4l2_control ctrl;
		g_object_get (self->src, "device-fd", &fd, NULL);
		ctrl.id = V4L2_CID_EXPOSURE_ABSOLUTE;
		ioctl( fd, VIDIOC_G_CTRL, &ctrl );
		g_value_set_double (value, ctrl.value / 10000.0);
	}
	break;

	case PROP_GAIN:
	{
		gint fd;
		struct v4l2_control ctrl;
		g_object_get (self->src, "device-fd", &fd, NULL);
		ctrl.id = V4L2_CID_GAIN;
		ioctl( fd, VIDIOC_G_CTRL, &ctrl );
		g_value_set_double (value, ctrl.value);
	}
	break;
		
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}

void
gst_tiscamerasrc_dispose (GObject * object)
{
	/* GstTisCameraSrc *tiscamerasrc = GST_TISCAMERASRC (object); */

	/* clean up as possible.  may be called multiple times */

	G_OBJECT_CLASS (gst_tiscamerasrc_parent_class)->dispose (object);
}

void
gst_tiscamerasrc_finalize (GObject * object)
{
	/* GstTisCameraSrc *tiscamerasrc = GST_TISCAMERASRC (object); */

	/* clean up object here */

	G_OBJECT_CLASS (gst_tiscamerasrc_parent_class)->finalize (object);
}


static gboolean
plugin_init (GstPlugin * plugin)
{

	return gst_element_register (plugin, "tiscamerasrc", GST_RANK_NONE,
				     GST_TYPE_TISCAMERASRC);
}

#ifndef VERSION
#define VERSION "0.0.FIXME"
#endif
#ifndef PACKAGE
#define PACKAGE "FIXME_package"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "FIXME_package_name"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "http://FIXME.org/"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
		   GST_VERSION_MINOR,
		   "tiscamerasrc",
		   "The Imaging Source camera source plugin",
		   plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)

