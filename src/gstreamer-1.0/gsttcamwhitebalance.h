

#ifndef __GST_TCAMWHITEBALANCE_H__
#define __GST_TCAMWHITEBALANCE_H__

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include <gst/gstbuffer.h>
#include <gst/video/video.h>

#include "image_sampling.h"

#define GST_TYPE_TCAMWHITEBALANCE            (gst_tcamwhitebalance_get_type())
#define GST_TCAMWHITEBALANCE(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_TCAMWHITEBALANCE,GstTcamWhitebalance))
#define GST_TCAMWHITEBALANCE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_TCAMWHITEBALANCE,GstTcamWhitebalanceClass))
#define GST_IS_TCAMWHITEBALANCE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_TCAMWHITEBALANCE))
#define GST_IS_TCAMWHITEBALANCE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_TCAMWHITEBALANCE))

typedef struct _GstTcamWhitebalance GstTcamWhitebalance;
typedef struct _GstTcamWhitebalanceClass GstTcamWhitebalanceClass;


static const guint MAX_STEPS = 20;
static const guint WB_IDENTITY = 64;
static const guint WB_MAX = 255;
static const guint BREAK_DIFF = 2;

const guint NEARGRAY_MIN_BRIGHTNESS      = 10;
const guint NEARGRAY_MAX_BRIGHTNESS      = 253;
const float NEARGRAY_MAX_COLOR_DEVIATION = 0.25f;
const float NEARGRAY_REQUIRED_AMOUNT     = 0.08f;

/* rgb values have to be evaluated differently. These are the according factors */
static const guint r_factor = (guint32)((1 << 8) * 0.299f);
static const guint g_factor = (guint32)((1 << 8) * 0.587f);
static const guint b_factor = (guint32)((1 << 8) * 0.114f);

typedef unsigned char byte;


struct _GstTcamWhitebalance {
    GstBaseTransform base_object;

    GstPad        *srcpad;
    GstPad        *sinkpad1;

    gst_tcam_image_size image_size;
    gdouble        framerate;
    tBY8Pattern    pattern;

    /* user defined values */
    gint red;
    gint green;
    gint blue;

    /* persistent values */
    rgb_tripel rgb;
    gboolean auto_wb;
    gboolean auto_enabled;

};

struct _GstTcamWhitebalanceClass {
    GstBaseTransformClass gstbasetransform_class;
};

GType gst_tcamwhitebalance_get_type (void);


#endif /* __GST_TCAMWHITEBALANCE_H__ */
