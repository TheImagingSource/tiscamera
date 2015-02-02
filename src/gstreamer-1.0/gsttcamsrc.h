

#ifndef TCAM_GSTTCAMSRC_H
#define TCAM_GSTTCAMSRC_H

#include <gst/gst.h>
#include <gst/base/gstpushsrc.h>

#include <tcam_c.h>

G_BEGIN_DECLS


#define GST_TYPE_TCAM           (gst_tcam_get_type())
#define GST_TCAM(obj)           (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_TCAM, GstTcam))
#define GST_TCAM_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_TCAM, GstTcam))
#define GST_IS_TCAM(obj)        (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_TCAM))
#define GST_IS_TCAM_CLASS(obj)  (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_TCAM))

typedef struct _GstTcam GstTcam;
typedef struct _GstTcamClass GstTcamClass;

struct _GstTcam
{
    GstPushSrc element;

    char* device_serial;
    tcam_capture_device* device;


    int n_buffers;
    const struct tcam_image_buffer* ptr;
    gboolean new_buffer;

    int payload;

    int buffer_timeout_us;

    int run;

    GstCaps *all_caps;
    GstCaps *fixed_caps;

    guint64 timestamp_offset;
    guint64 last_timestamp;
};


struct _GstTcamClass
{
    GstPushSrcClass parent_class;
};

GType gst_tcam_get_type (void);

G_END_DECLS

#endif /* TCAM_GSTTCAMSRC_H */
