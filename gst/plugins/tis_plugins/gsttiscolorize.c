/* GStreamer
 * Copyright (C) 2013 FIXME <fixme@example.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Suite 500,
 * Boston, MA 02110-1335, USA.
 */
/**
 * SECTION:element-gsttiscolorize
 *
 * The tiscolorize element does FIXME stuff.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v fakesrc ! tiscolorize ! FIXME ! fakesink
 * ]|
 * FIXME Describe what the pipeline does.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include "gsttiscolorize.h"

GST_DEBUG_CATEGORY_STATIC (gst_tiscolorize_debug_category);
#define GST_CAT_DEFAULT gst_tiscolorize_debug_category

/* prototypes */


static void gst_tiscolorize_set_property (GObject * object,
					      guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_tiscolorize_get_property (GObject * object,
					      guint property_id, GValue * value, GParamSpec * pspec);
static void gst_tiscolorize_finalize (GObject * object);

static GstFlowReturn
gst_tiscolorize_transform_ip (GstBaseTransform * trans, GstBuffer * buf);
static GstCaps *
gst_tiscolorize_transform_caps (GstBaseTransform * trans,
				    GstPadDirection direction, GstCaps * caps);

static void gst_tiscolorize_fixate_caps (GstBaseTransform * base,
					     GstPadDirection direction, GstCaps * caps, GstCaps * othercaps);



enum
{
	PROP_0,
	PROP_GAIN_RED,
	PROP_GAIN_GREEN,
	PROP_GAIN_BLUE,
	PROP_AUTO,
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

static void
gst_tiscolorize_class_init (GstTisColorizeClass * klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	GstBaseTransformClass *base_transform_class = GST_BASE_TRANSFORM_CLASS (klass);

	/* Setting up pads and setting metadata should be moved to
	   base_class_init if you intend to subclass this class. */

	gst_element_class_add_static_pad_template (GST_ELEMENT_CLASS(klass),
						   &gst_tiscolorize_src_template);
	gst_element_class_add_static_pad_template (GST_ELEMENT_CLASS(klass),
						   &gst_tiscolorize_sink_template);

	gst_element_class_set_details_simple (GST_ELEMENT_CLASS(klass),
					      "The Imaging Source White Balance Element", "Generic", "Adjusts white balancing of RAW video data buffers",
					      "Arne Caspari <arne.caspari@gmail.com>");

	gobject_class->set_property = gst_tiscolorize_set_property;
	gobject_class->get_property = gst_tiscolorize_get_property;
	gobject_class->finalize = gst_tiscolorize_finalize;
	base_transform_class->transform_ip = GST_DEBUG_FUNCPTR (gst_tiscolorize_transform_ip);
	//base_transform_class->transform = GST_DEBUG_FUNCPTR (gst_tiscolorize_transform);
	base_transform_class->transform_caps = GST_DEBUG_FUNCPTR (gst_tiscolorize_transform_caps);
	base_transform_class->fixate_caps = GST_DEBUG_FUNCPTR (gst_tiscolorize_fixate_caps);

	g_object_class_install_property (gobject_class,
					 PROP_GAIN_RED,
					 g_param_spec_int ("red gain", "Red Gain",
							   "Value for red gain", 0, 2048,
							   0,
							   G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	g_object_class_install_property (gobject_class,
					 PROP_GAIN_GREEN,
					 g_param_spec_int ("green gain", "Green Gain",
							   "Value for red gain", 0, 2048,
							   0,
							   G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	g_object_class_install_property (gobject_class,
					 PROP_GAIN_BLUE,
					 g_param_spec_int ("blue gain", "Blue Gain",
							   "Value for blue gain", 0, 2048,
							   0,
							   G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	g_object_class_install_property (gobject_class,
					 PROP_AUTO,
					 g_param_spec_boolean ("auto", "Auto White Balance",
							       "Automatically adjust white balance",
							       FALSE,
							       G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}

static void
gst_tiscolorize_init (GstTisColorize *self)
{
	/* gst_pad_set_getcaps_function (self->sinkpad, */
	/* 			      GST_DEBUG_FUNCPTR(gst_tiscolorize_sink_getcaps)); */
}

void
gst_tiscolorize_set_property (GObject * object, guint property_id,
				  const GValue * value, GParamSpec * pspec)
{
	GstTisColorize *tiscolorize = GST_TISCOLORIZE (object);

	switch (property_id) {
	case PROP_GAIN_RED:
		tiscolorize->gain_red = g_value_get_int (value);
		break;
	case PROP_GAIN_GREEN:
		tiscolorize->gain_green = g_value_get_int (value);
		break;
	case PROP_GAIN_BLUE:
		tiscolorize->gain_blue = g_value_get_int (value);
		break;
	case PROP_AUTO:
		tiscolorize->auto_wb = g_value_get_boolean (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}

void
gst_tiscolorize_get_property (GObject * object, guint property_id,
				  GValue * value, GParamSpec * pspec)
{
	GstTisColorize *tiscolorize = GST_TISCOLORIZE (object);

	switch (property_id) {
	case PROP_GAIN_RED:
		g_value_set_int (value, tiscolorize->gain_red);
		break;
	case PROP_GAIN_GREEN:
		g_value_set_int (value, tiscolorize->gain_green);
		break;
	case PROP_GAIN_BLUE:
		g_value_set_int (value, tiscolorize->gain_blue);
		break;
	case PROP_AUTO:
		g_value_set_boolean (value, tiscolorize->auto_wb);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}


void
gst_tiscolorize_finalize (GObject * object)
{
	/* GstTiscolorize *tiscolorize = GST_TISCOLORIZE (object); */

	/* clean up object here */

	G_OBJECT_CLASS (gst_tiscolorize_parent_class)->finalize (object);
}


static GstCaps *
gst_tiscolorize_transform_caps (GstBaseTransform * trans,
    GstPadDirection direction, GstCaps * caps)
{
	GstStructure *s;
	/* GstStructure *ns; */
	GstCaps *outcaps;
	/* const GValue *value; */

	
	outcaps = gst_caps_copy (caps);
	
	s = gst_caps_get_structure (outcaps, 0);


	if (direction == GST_PAD_SINK) {
		gst_structure_set_name (s, "video/x-raw-bayer");
		gst_structure_set (s, "format", G_TYPE_STRING, "grbg", NULL);
		gst_structure_remove_fields (s, "bpp", "depth", NULL);
	} else {
		gst_structure_set_name (s, "video/x-raw-gray");
		gst_structure_set (s, "bpp", G_TYPE_INT, 8, NULL);
		gst_structure_remove_field (s, "format");
	}

	GST_LOG_OBJECT (trans, "Transform caps\n\nin:%"GST_PTR_FORMAT"\nout:%"GST_PTR_FORMAT, caps, outcaps);
	
	return outcaps;
}

static void gst_tiscolorize_fixate_caps (GstBaseTransform * base,
					     GstPadDirection direction, GstCaps * caps, GstCaps * othercaps)
{
	GstStructure *ins, *outs;
	gint width, height;
	g_return_if_fail (gst_caps_is_fixed (caps));
	
	GST_DEBUG_OBJECT (base, "trying to fixate othercaps %" GST_PTR_FORMAT
			  " based on caps %" GST_PTR_FORMAT, othercaps, caps);

	ins = gst_caps_get_structure (caps, 0);
	outs = gst_caps_get_structure (othercaps, 0);

	if (gst_structure_get_int (ins, "width", &width)) {
		if (gst_structure_has_field (outs, "width")) {
			gst_structure_fixate_field_nearest_int (outs, "width", width);
		}
	}
	
	if (gst_structure_get_int (ins, "height", &height)) {
		if (gst_structure_has_field (outs, "height")) {
			gst_structure_fixate_field_nearest_int (outs, "width", height);
		}
	}
	
}

static GstFlowReturn
gst_tiscolorize_transform_ip (GstBaseTransform * trans, GstBuffer * buf)
{
    /* We change the caps automatically in the background. */
    /* Here we simply say everything is OK and go on. */

	return GST_FLOW_OK;
}

static gboolean
plugin_init (GstPlugin * plugin)
{

	return gst_element_register (plugin, "tiscolorize", GST_RANK_NONE,
				     GST_TYPE_TISCOLORIZE);
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
#define GST_PACKAGE_ORIGIN "http://code.google.com/p/tiscamera"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
		   GST_VERSION_MINOR,
		   "tiscolorize",
		   "The Imaging Source white balance plugin",
		   plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)
