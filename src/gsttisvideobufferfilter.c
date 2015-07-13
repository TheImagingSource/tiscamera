/*
 * GStreamer
 * Copyright (C) 2006 Stefan Kost <ensonic@users.sf.net>
 * Copyright (C) 2015  <<user@hostname.org>>
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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:element-tisvideobufferfilter
 *
 * Filters incomplete video buffers
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! tisvideobufferfilter ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>

#include "gsttisvideobufferfilter.h"

#define VERSION "0.2"
#define PACKAGE "tisvideobufferfilter"

GST_DEBUG_CATEGORY_STATIC (gst_tisvideobufferfilter_debug);
#define GST_CAT_DEFAULT gst_tisvideobufferfilter_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_DROPCOUNT,
  PROP_FRAMECOUNT,
};

/* the capabilities of the inputs and outputs.
 */
static GstStaticPadTemplate sink_template =
GST_STATIC_PAD_TEMPLATE (
  "sink",
  GST_PAD_SINK,
  GST_PAD_ALWAYS,
  GST_STATIC_CAPS ("video/x-raw-gray,bpp=8,framerate=(fraction)[0/1,1000/1],width=[1,MAX],height=[1,MAX];"\
		   "video/x-raw-bayer,format=(string){bggr,grbg,gbrg,rggb},framerate=(fraction)[0/1,MAX],width=[1,MAX],height=[1,MAX]"));

static GstStaticPadTemplate src_template =
GST_STATIC_PAD_TEMPLATE (
  "src",
  GST_PAD_SRC,
  GST_PAD_ALWAYS,
  GST_STATIC_CAPS ("video/x-raw-gray,bpp=8,framerate=(fraction)[0/1,1000/1],width=[1,MAX],height=[1,MAX];"\
		   "video/x-raw-bayer,format=(string){bggr,grbg,gbrg,rggb},framerate=(fraction)[0/1,MAX],width=[1,MAX],height=[1,MAX]"));

/* debug category for fltering log messages
 *
 * FIXME:exchange the string 'Template tisvideobufferfilter' with your description
 */
#define DEBUG_INIT(bla) \
  GST_DEBUG_CATEGORY_INIT (gst_tisvideobufferfilter_debug, "tisvideobufferfilter", 0, "tisvideobufferfilter debug");

GST_BOILERPLATE_FULL (Gsttisvideobufferfilter, gst_tisvideobufferfilter, GstBaseTransform,
    GST_TYPE_BASE_TRANSFORM, DEBUG_INIT);

static void gst_tisvideobufferfilter_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_tisvideobufferfilter_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static GstFlowReturn gst_tisvideobufferfilter_transform_ip (GstBaseTransform * base,
    GstBuffer * outbuf);

/* GObject vmethod implementations */

static void
gst_tisvideobufferfilter_base_init (gpointer klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_set_details_simple (element_class,
    "tisvideobufferfilter",
    "Generic/Filter",
    "Video Buffer Filter",
    " <arne.caspari@theimagingsource.com>");

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&src_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&sink_template));
}

/* initialize the tisvideobufferfilter's class */
static void
gst_tisvideobufferfilter_class_init (GsttisvideobufferfilterClass * klass)
{
  GObjectClass *gobject_class;

  gobject_class = (GObjectClass *) klass;
  gobject_class->set_property = gst_tisvideobufferfilter_set_property;
  gobject_class->get_property = gst_tisvideobufferfilter_get_property;

  g_object_class_install_property (gobject_class, PROP_DROPCOUNT,
    g_param_spec_uint ("dropcount", "DropCount", "Number of frames dropped",
		       0, G_MAXUINT, 0, G_PARAM_READABLE));

  g_object_class_install_property (gobject_class, PROP_FRAMECOUNT,
    g_param_spec_uint ("framecount", "FrameCount", "Number of frames received (dropped + passed)",
		       0, G_MAXUINT, 0, G_PARAM_READABLE));

  GST_BASE_TRANSFORM_CLASS (klass)->transform_ip =
      GST_DEBUG_FUNCPTR (gst_tisvideobufferfilter_transform_ip);
}

/* initialize the new element
 * initialize instance structure
 */
static void
gst_tisvideobufferfilter_init (Gsttisvideobufferfilter *filter, GsttisvideobufferfilterClass * klass)
{
  filter->dropcount = 0;
  filter->framecount = 0;
}

static void
gst_tisvideobufferfilter_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
	//Gsttisvideobufferfilter *filter = GST_TISVIDEOBUFFERFILTER (object);

  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_tisvideobufferfilter_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  Gsttisvideobufferfilter *filter = GST_TISVIDEOBUFFERFILTER (object);

  switch (prop_id) {
    case PROP_DROPCOUNT:
      g_value_set_uint (value, filter->dropcount);
      break;
    case PROP_FRAMECOUNT:
      g_value_set_uint (value, filter->framecount);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* GstBaseTransform vmethod implementations */

/* this function does the actual processing
 */
static GstFlowReturn
gst_tisvideobufferfilter_transform_ip (GstBaseTransform * base, GstBuffer * outbuf)
{
  Gsttisvideobufferfilter *filter = GST_TISVIDEOBUFFERFILTER (base);
  gint width, height;
  GstCaps *caps;
  GstStructure *s;

  filter->framecount++;

  caps = GST_BUFFER_CAPS(outbuf);
  if (!caps){
	  filter->dropcount++;
	  return GST_BASE_TRANSFORM_FLOW_DROPPED;
  }
  
  s = gst_caps_get_structure(caps, 0);
  if (!s){
	  filter->dropcount++;
	  return GST_BASE_TRANSFORM_FLOW_DROPPED;
  }

  if (!gst_structure_get (s, "width", G_TYPE_INT, &width, "height", G_TYPE_INT, &height, NULL)){
	  filter->dropcount++;
	  return GST_BASE_TRANSFORM_FLOW_DROPPED;
  }

  if (GST_BUFFER_SIZE (outbuf) <
      (width * height)){
	  filter->dropcount++;
	  return GST_BASE_TRANSFORM_FLOW_DROPPED;
  }

  return GST_FLOW_OK;
}


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
tisvideobufferfilter_init (GstPlugin * tisvideobufferfilter)
{
  return gst_element_register (tisvideobufferfilter, "tisvideobufferfilter", GST_RANK_NONE,
      GST_TYPE_TISVIDEOBUFFERFILTER);
}

/* gstreamer looks for this structure to register tisvideobufferfilters
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "tisvideobufferfilter",
    "Filters incomplete video buffers",
    tisvideobufferfilter_init,
    VERSION,
    "LGPL",
    "tiscamera",
    "https://github.com/TheImagingSource/tiscamera"
)
