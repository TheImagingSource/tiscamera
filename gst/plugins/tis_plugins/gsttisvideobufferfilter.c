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
 * SECTION:element-gsttisvideobufferfilter
 *
 * The tisvideobufferfilter element does FIXME stuff.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v fakesrc ! tisvideobufferfilter ! FIXME ! fakesink
 * ]|
 * FIXME Describe what the pipeline does.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/gstelement.h>
#include "gsttisvideobufferfilter.h"

GST_DEBUG_CATEGORY_STATIC (gst_tisvideobufferfilter_debug_category);
#define GST_CAT_DEFAULT gst_tisvideobufferfilter_debug_category

/* prototypes */


static void gst_tisvideobufferfilter_set_property (GObject * object,
						   guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_tisvideobufferfilter_get_property (GObject * object,
						   guint property_id, GValue * value, GParamSpec * pspec);
static void gst_tisvideobufferfilter_finalize (GObject * object);
static GstFlowReturn
gst_tisvideobufferfilter_sink_chain (GstPad *pad, GstBuffer *buffer);
static GstCaps *
gst_tisvideobufferfilter_sink_getcaps (GstPad *pad);

enum
{
	PROP_0
};

/* pad templates */
static GstStaticPadTemplate gst_tisvideobufferfilter_sink_template =
	GST_STATIC_PAD_TEMPLATE ("sink",
				 GST_PAD_SINK,
				 GST_PAD_ALWAYS,
				 GST_STATIC_CAPS ("video/x-raw-gray,bpp=8,framerate=(fraction)[0/1,1000/1]")
		);

static GstStaticPadTemplate gst_tisvideobufferfilter_src_template =
	GST_STATIC_PAD_TEMPLATE ("src",
				 GST_PAD_SRC,
				 GST_PAD_ALWAYS,
				 GST_STATIC_CAPS ("video/x-raw-gray,bpp=8,framerate=(fraction)[0/1,1000/1];video/x-raw-bayer,format=(fourcc)grbg,width=[1,MAX],height=[1,MAX],framerate=(fraction)[0/1,1000/1]")
		);


/* class initialization */

G_DEFINE_TYPE_WITH_CODE (GstTisVideoBufferFilter, gst_tisvideobufferfilter, GST_TYPE_ELEMENT,
			 GST_DEBUG_CATEGORY_INIT (gst_tisvideobufferfilter_debug_category, "tisvideobufferfilter", 0,
						  "debug category for tisvideobufferfilter element"));

static void
gst_tisvideobufferfilter_class_init (GstTisVideoBufferFilterClass * klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	/* Setting up pads and setting metadata should be moved to
	   base_class_init if you intend to subclass this class. */
	gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
					    gst_static_pad_template_get (&gst_tisvideobufferfilter_sink_template));
	gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
					    gst_static_pad_template_get (&gst_tisvideobufferfilter_src_template));

	gst_element_class_set_details_simple (GST_ELEMENT_CLASS(klass),
					      "The Imaging Source Video Buffer Filter", "Generic", "Filters(drops) corrupt raw video buffers from stream",
					      "Arne Caspari <arne.caspari@gmail.com>");
	/* gst_element_class_set_static_metadata (GST_ELEMENT_CLASS(klass), */
	/* 				       "Tis Video Buffer Filter", "Generic", "Filters(drops) corrupt raw video buffers from stream", */
	/* 				       "Arne Caspari <arne.caspari@gmail.com>"); */

	gobject_class->set_property = gst_tisvideobufferfilter_set_property;
	gobject_class->get_property = gst_tisvideobufferfilter_get_property;
	gobject_class->finalize = gst_tisvideobufferfilter_finalize;

}

static void
gst_tisvideobufferfilter_init (GstTisVideoBufferFilter *self)
{

	self->sinkpad = gst_pad_new_from_static_template (&gst_tisvideobufferfilter_sink_template,     
							  "sink");

	self->srcpad = gst_pad_new_from_static_template (&gst_tisvideobufferfilter_src_template,     
							 "src");

	gst_element_add_pad (GST_ELEMENT (self), 
			     self->sinkpad);
	gst_element_add_pad (GST_ELEMENT (self), 
			     self->srcpad);

	gst_pad_set_chain_function (self->sinkpad,
				    GST_DEBUG_FUNCPTR(gst_tisvideobufferfilter_sink_chain));

	gst_pad_set_getcaps_function (self->sinkpad,
				      GST_DEBUG_FUNCPTR(gst_tisvideobufferfilter_sink_getcaps));

				      
}

void
gst_tisvideobufferfilter_set_property (GObject * object, guint property_id,
				       const GValue * value, GParamSpec * pspec)
{
	/* GstTisVideoBufferFilter *tisvideobufferfilter = GST_TISVIDEOBUFFERFILTER (object); */

	switch (property_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}

void
gst_tisvideobufferfilter_get_property (GObject * object, guint property_id,
				       GValue * value, GParamSpec * pspec)
{
	/* GstTisVideoBufferFilter *tisvideobufferfilter = GST_TISVIDEOBUFFERFILTER (object); */

	switch (property_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}

void
gst_tisvideobufferfilter_finalize (GObject * object)
{
	/* GstTisVideoBufferFilter *tisvideobufferfilter = GST_TISVIDEOBUFFERFILTER (object); */

	/* clean up object here */

	G_OBJECT_CLASS (gst_tisvideobufferfilter_parent_class)->finalize (object);
}


static GstCaps *
gst_tisvideobufferfilter_sink_getcaps (GstPad *pad)
{
	GstTisVideoBufferFilter *self = GST_TISVIDEOBUFFERFILTER (gst_pad_get_parent (pad));
	GstPad *peerpad;
	
	peerpad = gst_pad_get_peer (self->srcpad);
	if (peerpad){
		GstCaps *caps;
		GstStructure *s;
		caps = gst_pad_get_caps (peerpad);
		s = gst_caps_get_structure (caps, 0);
		if (gst_structure_has_name (s, "video/x-raw-bayer")){
			caps = gst_caps_make_writable (caps);
			gst_structure_set_name (s, "video/x-raw-gray");
		}

		GST_DEBUG_OBJECT (self, "flt Caps: %s", gst_caps_to_string (caps));
		
		return caps;
	}

	GST_DEBUG_OBJECT (self, "%s", gst_caps_to_string (gst_pad_get_caps (self->srcpad)));
		
	return gst_pad_get_caps (self->srcpad);
}


static GstFlowReturn
gst_tisvideobufferfilter_sink_chain (GstPad *pad, GstBuffer *buffer)
{
	GstTisVideoBufferFilter *filter;
	GstCaps *caps = GST_BUFFER_CAPS (buffer);
	GstStructure *s = gst_caps_get_structure (caps, 0);
	gint width, height;

	if (!gst_structure_get_int (s, "width", &width) || 
	    !gst_structure_get_int (s, "height", &height)){
		return GST_FLOW_ERROR;
	}

	filter = GST_TISVIDEOBUFFERFILTER (gst_pad_get_parent (pad));
	GST_DEBUG_OBJECT(filter, "chain");
	
	if (GST_BUFFER_SIZE (buffer) >=
	    (width * height)){
		// if buffer contains enough data for video format, push it to the src pad
		gst_pad_push (filter->srcpad, buffer);

	}
	gst_object_unref (filter);
	return GST_FLOW_OK;
}

static gboolean
plugin_init (GstPlugin * plugin)
{

	return gst_element_register (plugin, "tisvideobufferfilter", GST_RANK_NONE,
				     GST_TYPE_TISVIDEOBUFFERFILTER);
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
		   "tisvideobufferfilter",
		   "FIXME plugin description",
		   plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)

