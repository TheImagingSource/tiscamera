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
 * SECTION:element-gsttiswhitebalance
 *
 * The tiswhitebalance element does FIXME stuff.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v fakesrc ! tiswhitebalance ! FIXME ! fakesink
 * ]|
 * FIXME Describe what the pipeline does.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include "gsttiswhitebalance.h"

GST_DEBUG_CATEGORY_STATIC (gst_tiswhitebalance_debug_category);
#define GST_CAT_DEFAULT gst_tiswhitebalance_debug_category

/* prototypes */


static void gst_tiswhitebalance_set_property (GObject * object,
					      guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_tiswhitebalance_get_property (GObject * object,
					      guint property_id, GValue * value, GParamSpec * pspec);
static void gst_tiswhitebalance_finalize (GObject * object);

static GstFlowReturn
gst_tiswhitebalance_transform_ip (GstBaseTransform * trans, GstBuffer * buf);
static GstCaps *
gst_tiswhitebalance_transform_caps (GstBaseTransform * trans,
				    GstPadDirection direction, GstCaps * caps);

static void gst_tiswhitebalance_fixate_caps (GstBaseTransform * base,
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

static GstStaticPadTemplate gst_tiswhitebalance_sink_template =
	GST_STATIC_PAD_TEMPLATE ("sink",
				 GST_PAD_SINK,
				 GST_PAD_ALWAYS,
				 GST_STATIC_CAPS ("video/x-raw-gray,bpp=8,framerate=(fraction)[0/1,1000/1],width=[1,MAX],height=[1,MAX]")
		);

static GstStaticPadTemplate gst_tiswhitebalance_src_template =
	GST_STATIC_PAD_TEMPLATE ("src",
				 GST_PAD_SRC,
				 GST_PAD_ALWAYS,
				 GST_STATIC_CAPS ("video/x-raw-bayer,format=grbg,framerate=(fraction)[0/1,1000/1],width=[1,MAX],height=[1,MAX],format=(string)grbg")
		);


/* class initialization */

G_DEFINE_TYPE_WITH_CODE (GstTisWhiteBalance, gst_tiswhitebalance, GST_TYPE_BASE_TRANSFORM,
			 GST_DEBUG_CATEGORY_INIT (gst_tiswhitebalance_debug_category, "tiswhitebalance", 0,
						  "debug category for tiswhitebalance element"));

static void
gst_tiswhitebalance_class_init (GstTisWhiteBalanceClass * klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	GstBaseTransformClass *base_transform_class = GST_BASE_TRANSFORM_CLASS (klass);

	/* Setting up pads and setting metadata should be moved to
	   base_class_init if you intend to subclass this class. */

	gst_element_class_add_static_pad_template (GST_ELEMENT_CLASS(klass),
						   &gst_tiswhitebalance_src_template);
	gst_element_class_add_static_pad_template (GST_ELEMENT_CLASS(klass),
						   &gst_tiswhitebalance_sink_template);

	gst_element_class_set_details_simple (GST_ELEMENT_CLASS(klass),
					      "The Imaging Source White Balance Element", "Generic", "Adjusts white balancing of RAW video data buffers",
					      "Arne Caspari <arne.caspari@gmail.com>");

	gobject_class->set_property = gst_tiswhitebalance_set_property;
	gobject_class->get_property = gst_tiswhitebalance_get_property;
	gobject_class->finalize = gst_tiswhitebalance_finalize;
	base_transform_class->transform_ip = GST_DEBUG_FUNCPTR (gst_tiswhitebalance_transform_ip);
	//base_transform_class->transform = GST_DEBUG_FUNCPTR (gst_tiswhitebalance_transform);
	base_transform_class->transform_caps = GST_DEBUG_FUNCPTR (gst_tiswhitebalance_transform_caps);
	base_transform_class->fixate_caps = GST_DEBUG_FUNCPTR (gst_tiswhitebalance_fixate_caps);

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
gst_tiswhitebalance_init (GstTisWhiteBalance *self)
{
	/* gst_pad_set_getcaps_function (self->sinkpad, */
	/* 			      GST_DEBUG_FUNCPTR(gst_tiswhitebalance_sink_getcaps)); */
}

void
gst_tiswhitebalance_set_property (GObject * object, guint property_id,
				  const GValue * value, GParamSpec * pspec)
{
	GstTisWhiteBalance *tiswhitebalance = GST_TISWHITEBALANCE (object);

	switch (property_id) {
	case PROP_GAIN_RED:
		tiswhitebalance->gain_red = g_value_get_int (value);
		break;
	case PROP_GAIN_GREEN:
		tiswhitebalance->gain_green = g_value_get_int (value);
		break;
	case PROP_GAIN_BLUE:
		tiswhitebalance->gain_blue = g_value_get_int (value);
		break;
	case PROP_AUTO:
		tiswhitebalance->auto_wb = g_value_get_boolean (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}

void
gst_tiswhitebalance_get_property (GObject * object, guint property_id,
				  GValue * value, GParamSpec * pspec)
{
	GstTisWhiteBalance *tiswhitebalance = GST_TISWHITEBALANCE (object);

	switch (property_id) {
	case PROP_GAIN_RED:
		g_value_set_int (value, tiswhitebalance->gain_red);
		break;
	case PROP_GAIN_GREEN:
		g_value_set_int (value, tiswhitebalance->gain_green);
		break;
	case PROP_GAIN_BLUE:
		g_value_set_int (value, tiswhitebalance->gain_blue);
		break;
	case PROP_AUTO:
		g_value_set_boolean (value, tiswhitebalance->auto_wb);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}


void
gst_tiswhitebalance_finalize (GObject * object)
{
	/* GstTisWhiteBalance *tiswhitebalance = GST_TISWHITEBALANCE (object); */

	/* clean up object here */

	G_OBJECT_CLASS (gst_tiswhitebalance_parent_class)->finalize (object);
}

static void
auto_grbg (GstTisWhiteBalance *self, GstBuffer *buf)
{
	GstCaps *caps = GST_BUFFER_CAPS (buf);
	GstStructure *structure = gst_caps_get_structure (caps, 0);

	guint32 sum_r = 0;
	guint32 sum_g = 0;
	guint32 sum_b = 0;
	guint32 samples_r = 0;
	guint32 samples_g = 0;
	guint32 samples_b = 0;
	guint8 *data = (guint8*)GST_BUFFER_DATA (buf);
	gint width, height;
	
	g_return_if_fail (gst_structure_get_int (structure, "width", &width));
	g_return_if_fail (gst_structure_get_int (structure, "height", &height));

	guint x,y;

	for (y = 64; y < height-63; y+= 64){
		for (x = 0; x < width; x += 64){
			guint8 value = data[y*width+x];
			if ( value < 250 ){
				samples_g++;
				sum_g += value;
			}
			value = data[y*width+x+1];
			if ( value < 250 ){
				samples_r++;
				sum_r += value;
			}
			value = data[(y+1)*width+x];
			if ( value < 250 ){
				samples_b++;
				sum_b += value;
			}
		}
	}

	sum_r /= samples_r;
	sum_g /= samples_g;
	sum_b /= samples_b;


	if ((sum_r > sum_g) && (sum_r > sum_b)){
		// R max
		self->gain_red = 0;
		self->gain_green = (int)(((float)sum_r/(float)sum_g)*1024)-1024;
		self->gain_blue = (int)(((float)sum_r/(float)sum_b)*1024)-1024;
	} else if ((sum_g > sum_r) && (sum_g > sum_b)){
		// G max
		self->gain_red = (int)(((float)sum_g/(float)sum_r)*1024)-1024;
		self->gain_green = 0;
		self->gain_blue = (int)(((float)sum_g/(float)sum_b)*1024)-1024;
	} else {
		// B max
		self->gain_red = (int)(((float)sum_g/(float)sum_r)*1024)-1024;
		self->gain_green = (int)(((float)sum_r/(float)sum_g)*1024)-1024;
		self->gain_blue = 0;
	}

	GST_LOG_OBJECT (self, "R: %03d[%04d] G: %03d[%04d], B: %03d[%04d]", sum_r, self->gain_red, sum_g, self->gain_green, sum_b, self->gain_blue);
}

static void
wb_grbg (GstBuffer *buf, gint gain_r, gint gain_g, gint gain_b)
{
	guint8 *data = (guint8*)GST_BUFFER_DATA (buf);
	GstCaps *caps = GST_BUFFER_CAPS (buf);
	GstStructure *structure = gst_caps_get_structure (caps, 0);
	gint width, height;
	g_return_if_fail (gst_structure_get_int (structure, "width", &width));
	g_return_if_fail (gst_structure_get_int (structure, "height", &height));

	guint gr = gain_r + 1024;
	guint gg = gain_g + 1024;
	guint gb = gain_b + 1024;

	guint x,y;
	for ( y = 0; y < height; y += 2){
		for (x = 0; x < width; x += 2){
			guint32 val;
			val = ( (guint32)*data * gg ) / 1024;
			if ((val) > 255)
				val = 255;
			*data = val & 0xff;
			data++;

			val = ( (guint32)*data * gr ) / 1024;
			if ((val) > 255)
				val = 255;
			*data = val & 0xff;
			data++;
		}
		for (x = 0; x < width; x += 2){
			guint32 val;
			val = ( (guint32)*data * gb ) / 1024;
			if ((val) > 255)
				val = 255;
			*data = val & 0xff;
			data++;

			val = ((guint32)*data * gg ) / 1024;
			if ((val) > 255)
				val = 255;
			*data = val & 0xff;
			data++;
		}
	}
}

static GstCaps *
gst_tiswhitebalance_transform_caps (GstBaseTransform * trans,
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

static void gst_tiswhitebalance_fixate_caps (GstBaseTransform * base,
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
gst_tiswhitebalance_transform_ip (GstBaseTransform * trans, GstBuffer * buf)
{
	GstTisWhiteBalance *self = GST_TISWHITEBALANCE (trans);

	if (self->auto_wb){
		auto_grbg (self, buf);
	}

	wb_grbg (buf,
		 self->gain_red, 
		 self->gain_green, 
		 self->gain_blue);

	return GST_FLOW_OK;
}

static gboolean
plugin_init (GstPlugin * plugin)
{

	return gst_element_register (plugin, "tiswhitebalance", GST_RANK_NONE,
				     GST_TYPE_TISWHITEBALANCE);
}

#ifndef VERSION
#define VERSION "0.0.1"
#endif
#ifndef PACKAGE
#define PACKAGE "tiswhitebalance_package"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "tiswhitebalance_package_name"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "http://code.google.com/p/tiscamera"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
		   GST_VERSION_MINOR,
		   "tiswhitebalance",
		   "The Imaging Source white balance plugin",
		   plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)
