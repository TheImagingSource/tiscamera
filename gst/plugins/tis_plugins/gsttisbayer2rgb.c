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
 * SECTION:element-gsttisbayer2rgb
 *
 * The tisbayer2rgb element does FIXME stuff.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v fakesrc ! tisbayer2rgb ! FIXME ! fakesink
 * ]|
 * FIXME Describe what the pipeline does.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include "gsttisbayer2rgb.h"

GST_DEBUG_CATEGORY_STATIC (gst_tisbayer2rgb_debug_category);
#define GST_CAT_DEFAULT gst_tisbayer2rgb_debug_category

/* prototypes */


static void gst_tisbayer2rgb_set_property (GObject * object,
					   guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_tisbayer2rgb_get_property (GObject * object,
					   guint property_id, GValue * value, GParamSpec * pspec);
static void gst_tisbayer2rgb_dispose (GObject * object);
static void gst_tisbayer2rgb_finalize (GObject * object);
static gboolean
gst_tisbayer2rgb_transform_size (GstBaseTransform *trans,
				 GstPadDirection direction,
				 GstCaps *caps, gsize size,
				 GstCaps *othercaps, gsize *othersize);
static GstCaps*
gst_tisbayer2rgb_transform_caps (GstBaseTransform *trans,
				 GstPadDirection direction, GstCaps *caps);
static gboolean gst_tisbayer2rgb_accept_caps (GstBaseTransform *trans,
					      GstPadDirection direction, GstCaps *caps);
static gboolean gst_tisbayer2rgb_set_caps (GstBaseTransform *trans, GstCaps *incaps,
					   GstCaps *outcaps);
static GstFlowReturn
gst_tisbayer2rgb_transform (GstBaseTransform * trans, GstBuffer * inbuf,
			    GstBuffer * outbuf);



enum
{
	PROP_0, 
	PROP_RGAIN, 
	PROP_BGAIN, 
	PROP_GGAIN,
	PROP_AUTO_GAIN,
};

/* pad templates */
static GstStaticPadTemplate gst_tisbayer2rgb_sink_template =
	GST_STATIC_PAD_TEMPLATE ("sink",
				 GST_PAD_SINK,
				 GST_PAD_ALWAYS,
				 GST_STATIC_CAPS ("video/x-raw-gray,bpp=8,framerate=(fraction)[0/1,1000/1],width=[1,MAX],height=[1,MAX]")
		);

static GstStaticPadTemplate gst_tisbayer2rgb_src_template =
	GST_STATIC_PAD_TEMPLATE ("src",
				 GST_PAD_SRC,
				 GST_PAD_ALWAYS,
				 GST_STATIC_CAPS ("video/x-raw-rgb,depth=24,bpp=24,endianess=4321,red-mask=0xff00000,green-mask=0xff0000,blue-mask=0xff00,framerate=(fraction)[0/1,1000/1],width=[1,MAX],height=[1,MAX]")
		);

/* class initialization */

G_DEFINE_TYPE_WITH_CODE (GstTisBayer2RGB, gst_tisbayer2rgb, GST_TYPE_BASE_TRANSFORM,
			 GST_DEBUG_CATEGORY_INIT (gst_tisbayer2rgb_debug_category, "tisbayer2rgb", 0,
						  "debug category for tisbayer2rgb element"));

static void
gst_tisbayer2rgb_class_init (GstTisBayer2RGBClass * klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	GstBaseTransformClass *base_transform_class = GST_BASE_TRANSFORM_CLASS (klass);

	/* Setting up pads and setting metadata should be moved to
	   base_class_init if you intend to subclass this class. */
	gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
					    gst_static_pad_template_get (&gst_tisbayer2rgb_sink_template));
	gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
					    gst_static_pad_template_get (&gst_tisbayer2rgb_src_template));

	gst_element_class_set_details_simple (GST_ELEMENT_CLASS(klass),
					      "The Imaging Source Bayer Interpolation Filter", 
					      "Generic", "Applies bayer pattern color interpolation to monochrome video images",
					      "Arne Caspari <arne.caspari@gmail.com>");

	gobject_class->set_property = gst_tisbayer2rgb_set_property;
	gobject_class->get_property = gst_tisbayer2rgb_get_property;
	gobject_class->dispose = gst_tisbayer2rgb_dispose;
	gobject_class->finalize = gst_tisbayer2rgb_finalize;
	base_transform_class->transform_size = GST_DEBUG_FUNCPTR (gst_tisbayer2rgb_transform_size);
	base_transform_class->accept_caps = GST_DEBUG_FUNCPTR (gst_tisbayer2rgb_accept_caps);
	base_transform_class->set_caps = GST_DEBUG_FUNCPTR (gst_tisbayer2rgb_set_caps);
	base_transform_class->transform_caps = GST_DEBUG_FUNCPTR (gst_tisbayer2rgb_transform_caps);
	base_transform_class->transform = GST_DEBUG_FUNCPTR (gst_tisbayer2rgb_transform);

	g_object_class_install_property (gobject_class,
					 PROP_RGAIN,
					 g_param_spec_double ("red gain", "Red Gain",
							      "Value for red gain", 0, 4.0,
							      1.0,
							      G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	g_object_class_install_property (gobject_class,
					 PROP_GGAIN,
					 g_param_spec_double ("green gain", "Green Gain",
							      "Value for red gain", 0, 4.0,
							      1.0,
							      G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	g_object_class_install_property (gobject_class,
					 PROP_BGAIN,
					 g_param_spec_double ("blue gain", "Blue Gain",
							      "Value for blue gain", 0, 4.0,
							      1.0,
							      G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	g_object_class_install_property (gobject_class,
					 PROP_AUTO_GAIN,
					 g_param_spec_boolean ("auto gain", "Auto White Balance",
							       "Automatically adjust white balance",
							       FALSE,
							       G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}

static void
gst_tisbayer2rgb_init (GstTisBayer2RGB *tisbayer2rgb)
{

	tisbayer2rgb->sinkpad = gst_pad_new_from_static_template (&gst_tisbayer2rgb_sink_template,     
								  "sink");

	tisbayer2rgb->srcpad = gst_pad_new_from_static_template (&gst_tisbayer2rgb_src_template,     
								 "src");
}

void
gst_tisbayer2rgb_set_property (GObject * object, guint property_id,
			       const GValue * value, GParamSpec * pspec)
{
	GstTisBayer2RGB *self = GST_TISBAYER2RGB (object);

	switch (property_id) {
	case PROP_RGAIN:
		self->rgain = g_value_get_double (value) * 4096;
		break;
	case PROP_GGAIN:
		self->ggain = g_value_get_double (value) * 4096;
		break;
	case PROP_BGAIN:
		self->bgain = g_value_get_double (value) * 4096;
		break;
	case PROP_AUTO_GAIN:
		self->auto_gain = g_value_get_boolean (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}

void
gst_tisbayer2rgb_get_property (GObject * object, guint property_id,
			       GValue * value, GParamSpec * pspec)
{
	GstTisBayer2RGB *self = GST_TISBAYER2RGB (object);

	switch (property_id) {
	case PROP_RGAIN:
		g_value_set_double (value, self->rgain / 4096);
		break;
	case PROP_GGAIN:
		g_value_set_double (value, self->ggain / 4096);
		break;
	case PROP_BGAIN:
		g_value_set_double (value, self->bgain / 4096);
		break;
	case PROP_AUTO_GAIN:
		g_value_set_boolean (value, self->auto_gain);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}

void
gst_tisbayer2rgb_dispose (GObject * object)
{
	/* GstTisBayer2RGB *tisbayer2rgb = GST_TISBAYER2RGB (object); */

	/* clean up as possible.  may be called multiple times */

	G_OBJECT_CLASS (gst_tisbayer2rgb_parent_class)->dispose (object);
}

void
gst_tisbayer2rgb_finalize (GObject * object)
{
	/* GstTisBayer2RGB *tisbayer2rgb = GST_TISBAYER2RGB (object); */

	/* clean up object here */

	G_OBJECT_CLASS (gst_tisbayer2rgb_parent_class)->finalize (object);
}


static gboolean
gst_tisbayer2rgb_accept_caps (GstBaseTransform *trans,
			      GstPadDirection direction, GstCaps *caps)
{
	return TRUE;
}

static gboolean
gst_tisbayer2rgb_set_caps (GstBaseTransform * trans, GstCaps * incaps,
			   GstCaps * outcaps)
{
	GstStructure *is = gst_caps_get_structure (incaps, 0);
	GstStructure *os = gst_caps_get_structure (outcaps, 0);
	GstCaps *sinkcaps;
	gint width, height;
	const GValue *framerate = NULL;
	
	gst_structure_get (is, "width", G_TYPE_INT, &width, "height", G_TYPE_INT, &height, NULL);
	framerate = gst_structure_get_value (is, "framerate");
	gst_structure_set (os, "width", G_TYPE_INT, width, "height", G_TYPE_INT, height, NULL);
	gst_structure_set_value (os, "framerate", framerate);

	return TRUE;
}

static gboolean
gst_tisbayer2rgb_transform_size (GstBaseTransform *trans,
				 GstPadDirection direction,
				 GstCaps *caps, gsize size,
				 GstCaps *othercaps, gsize *othersize)
{
	GstStructure *s;
	s = gst_caps_get_structure (othercaps, 0);
	gint width, height, bpp;
	
	gst_structure_get (s,
			   "width", G_TYPE_INT, &width,
			   "height", G_TYPE_INT, &height,
			   "bpp", G_TYPE_INT, &bpp, 
			   NULL);
	*othersize = width * height * bpp / 8;
	
	return TRUE;
	
}

static GstCaps *
gst_rgb2bayer_transform_caps (GstBaseTransform * trans,
    GstPadDirection direction, GstCaps * caps, GstCaps * filter)
{
  GstStructure *structure;
  GstStructure *new_structure;
  GstCaps *newcaps;
  const GValue *value;

  GST_DEBUG_OBJECT (trans, "transforming caps (from) %" GST_PTR_FORMAT, caps);

  structure = gst_caps_get_structure (caps, 0);

  if (direction == GST_PAD_SRC) {
    newcaps = gst_caps_new_empty_simple ("video/x-raw");
  } else {
    newcaps = gst_caps_new_empty_simple ("video/x-bayer");
  }
  new_structure = gst_caps_get_structure (newcaps, 0);

  value = gst_structure_get_value (structure, "width");
  gst_structure_set_value (new_structure, "width", value);

  value = gst_structure_get_value (structure, "height");
  gst_structure_set_value (new_structure, "height", value);

  value = gst_structure_get_value (structure, "framerate");
  gst_structure_set_value (new_structure, "framerate", value);

  GST_DEBUG_OBJECT (trans, "transforming caps (into) %" GST_PTR_FORMAT,
      newcaps);

  if (filter) {
    GstCaps *tmpcaps = newcaps;
    newcaps = gst_caps_intersect (newcaps, filter);
    gst_caps_unref (tmpcaps);
  }

  return newcaps;
}

static GstCaps *
gst_rgb2bayer_transform_caps (GstBaseTransform * trans,
    GstPadDirection direction, GstCaps * caps, GstCaps * filter)
{
	GstStructure *s;
	GstStructure *ns;
	GstCaps *outcaps;
	const GValue *value;
	
	s = gst_caps_get_structrure (caps, 0);

	if (direction == GST_PAD_SRC) {
		outcaps = gst_caps_new_empty_simple ("video/x-raw-rgb");
	} else {
		outcaps = gst_caps_new_empty_simple ("video/x-raw-gray");
	}
	

return outcaps;
}
	


union _rgb24pixel
{
	struct
	{
		unsigned char r;
		unsigned char g;
		unsigned char b;
	}c;
      
	unsigned int combined:24;
}__attribute__((packed));

typedef union _rgb24pixel rgb24pixel_t;
#define CLIP(x) (x>255?255:x)

#define R_RG(ptr,off,x,w) ptr[off+x]
#define G_RG(ptr,off,x,w) ((int)ptr[off + x + w ] + (int)ptr[off + x + 1]) / 2
#define B_RG(ptr,off,x,w) ptr[off + x + w + 1]
#define R_GR(ptr,off,x,w) ptr[off + x + 1 ]
#define G_GR(ptr,off,x,w) ((int)ptr[off + x + w + 1 ] + (int)ptr[off + x ]) / 2

void debayer_rgb24_nn( guchar *_destbuf, guchar *source, gint width, gint height, gint rgain, gint ggain, gint bgain )
{
   int x, y;
   unsigned char *dest = _destbuf;

   for( y = 1; y < height-1; y+=2 )
   {
      int lineoffset = y*width;

      //RGRGR
      //GBGBG
      //RGRGR

      for( x = 0; x < width -1; x+=2 )
      {
	      guint8 b1 = source[ lineoffset + x ];
	      guint8 b2 = (b1 + source[ lineoffset + x + 1])/2;
	      guint8 g2 = source[ lineoffset + x + 1];
	      guint8 g1 = ( (int)source[ lineoffset + x + width ] + (int)g2) / 2;
	      guint8 r = source[ lineoffset + x + width + 1];

	      *dest++ = CLIP ((r * rgain) / 4096);
	      *dest++ = CLIP ((g1 * ggain) / 4096);
	      *dest++ = CLIP ((b1 * bgain) / 4096);

	      *dest++ = CLIP ((r * rgain) / 4096);
	      *dest++ = CLIP ((g2 * ggain) / 4096);
	      *dest++ = CLIP ((b2 * bgain) / 4096);
      }

      lineoffset = (y+1)*width;

      //GBGBG
      //RGRGR
      //GBGBG

      for( x = 0; x < width -1; x+=2 )
      {
	      guint8 r2 = source[ lineoffset + x + 1];
	      guint8 r1 = ((gint)r2 + (gint)source[ lineoffset + x-1 ])/2;
	      guint8 g1 = source[ lineoffset + x ];
	      guint8 g2 = ((gint)g1 + (gint)source[ lineoffset + x + 2])/2;
	      guint8 b = source[ lineoffset + x + width];
	 
	      *dest++ = CLIP ((r1 * rgain) / 4096);
	      *dest++ = CLIP ((g1 * ggain) / 4096);
	      *dest++ = CLIP ((b * bgain) / 4096);

	      *dest++ = CLIP ((r2 * rgain) / 4096);
	      *dest++ = CLIP ((g2 * ggain) / 4096);
	      *dest++ = CLIP ((b * bgain) / 4096);
	 
      }
   }
}

static void
auto_gbrg (GstBuffer *buf, gint *gain_r, gint *gain_g, gint *gain_b)
{
	GstCaps *caps = GST_BUFFER_CAPS (buf);
	GstStructure *structure = gst_caps_get_structure (caps, 0);

	guint32 sum_r = 0;
	guint32 sum_g = 0;
	guint32 sum_b = 0;
	guint32 samples_r = 1;
	guint32 samples_g = 1;
	guint32 samples_b = 1;
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

	gdouble fr = 1.0;
	gdouble fg = 1.0;
	gdouble fb = 1.0;

	if (sum_r > sum_g){
		if (sum_r > sum_b){
			// max r
			fb = (gdouble)sum_r / (gdouble)sum_b;
			fg = (gdouble)sum_r / (gdouble)sum_g;
		} else {
			// max b
			fr = (gdouble)sum_b / (gdouble)sum_r;
			fg = (gdouble)sum_b / (gdouble)sum_g;
		}
	} else {
		if (sum_b > sum_g){
			// max b
			fr = (gdouble)sum_b / (gdouble)sum_r;
			fg = (gdouble)sum_b / (gdouble)sum_g;
		} else {
			// max g
			fb = (gdouble)sum_g / (gdouble)sum_b;
			fr = (gdouble)sum_g / (gdouble)sum_r;
		}
	}

	if (fr > 4.0) fr = 4.0;
	if (fg > 4.0) fg = 4.0;
	if (fb > 4.0) fb = 4.0;

	*gain_r = fr * 4096;
	*gain_g = fg * 4096;
	*gain_b = fb * 4096;
}

static GstFlowReturn
gst_tisbayer2rgb_transform (GstBaseTransform * trans, GstBuffer * inbuf,
			    GstBuffer * outbuf)
{
	GstTisBayer2RGB *self = GST_TISBAYER2RGB (trans);
	GstCaps *caps;
	GstStructure *s;
	gint width, height;

	caps = GST_BUFFER_CAPS (inbuf);
	s = gst_caps_get_structure (caps, 0);
	
	if (!gst_structure_get_int (s, "width", &width) || 
	    !gst_structure_get_int (s, "height", &height)){
		GST_DEBUG_OBJECT (trans, "Could not determine buffer width or height");
		return GST_FLOW_ERROR;
	}

	if (self->auto_gain)
		auto_gbrg (inbuf, &self->rgain, &self->ggain, &self->bgain);

	debayer_rgb24_nn (GST_BUFFER_DATA (outbuf), GST_BUFFER_DATA (inbuf), width, height, self->rgain, self->ggain, self->bgain);

	return GST_FLOW_OK;
}

static gboolean
plugin_init (GstPlugin * plugin)
{

	return gst_element_register (plugin, "tisbayer2rgb", GST_RANK_NONE,
				     GST_TYPE_TISBAYER2RGB);
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
		   "tisbayer2rgb",
		   "FIXME plugin description",
		   plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)

