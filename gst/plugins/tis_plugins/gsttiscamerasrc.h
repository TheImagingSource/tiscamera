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
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _GST_TISCAMERASRC_H_
#define _GST_TISCAMERASRC_H_

#include <gst/gstbin.h>

G_BEGIN_DECLS

#define GST_TYPE_TISCAMERASRC   (gst_tiscamerasrc_get_type())
#define GST_TISCAMERASRC(obj)   (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_TISCAMERASRC,GstTisCameraSrc))
#define GST_TISCAMERASRC_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_TISCAMERASRC,GstTisCameraSrcClass))
#define GST_IS_TISCAMERASRC(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_TISCAMERASRC))
#define GST_IS_TISCAMERASRC_CLASS(obj)   (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_TISCAMERASRC))

typedef struct _GstTisCameraSrc GstTisCameraSrc;
typedef struct _GstTisCameraSrcClass GstTisCameraSrcClass;

struct _GstTisCameraSrc
{
	GstBin base_tiscamerasrc;

	GstPad *srcpad;

	GstElement *src;
	GstElement *flt;
	GstElement *wb;
	GstElement *capsfilter;
	GstElement *capssetter;
	GstElement *debayer;
	GstElement *identity;

	gchar *device;

};

struct _GstTisCameraSrcClass
{
	GstBinClass base_tiscamerasrc_class;
};

GType gst_tiscamerasrc_get_type (void);

G_END_DECLS

#endif
