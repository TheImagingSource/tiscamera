

#include "gsttcamsrc.h"
#include "gsttcambase.h"

#include "tcam_c.h"

#include <unistd.h>


#define GST_TCAM_DEFAULT_N_BUFFERS		10

GST_DEBUG_CATEGORY_STATIC (tcam_debug);
#define GST_CAT_DEFAULT tcam_debug


enum
{
    PROP_0,
    PROP_SERIAL,
    PROP_DEVICE,
    PROP_NUM_BUFFERS,
};

G_DEFINE_TYPE (GstTcam, gst_tcam, GST_TYPE_PUSH_SRC);


static GstStaticPadTemplate tcam_src_template = GST_STATIC_PAD_TEMPLATE ("src",
                                                                         GST_PAD_SRC,
                                                                         GST_PAD_ALWAYS,
                                                                         GST_STATIC_CAPS ("ANY"));


static GstCaps* gst_tcam_get_all_camera_caps (GstTcam* self)
{
    GST_ERROR("ALL_CAPS");

    GstCaps* caps;
    gint64* pixel_formats;
    double min_frame_rate, max_frame_rate;
    int min_height, min_width;
    int max_height, max_width;
    int n_pixel_formats;

    g_return_val_if_fail (GST_IS_TCAM (self), NULL);

    if (self->device == NULL)
        return NULL;

    GST_LOG_OBJECT (self, "Get all camera caps");

    int format_count = tcam_capture_device_get_image_format_descriptions_count(self->device);
    struct tcam_video_format_description format [format_count];

    n_pixel_formats = tcam_capture_device_get_image_format_descriptions (self->device,
                                                                         format,
                                                                         format_count);

    GST_WARNING("Found %i pixel formats", n_pixel_formats);

    caps = gst_caps_new_empty ();
    unsigned int i;
    for (i = 0; i < n_pixel_formats; i++)
    {
        const char* caps_string = tcam_fourcc_to_gst_1_0_caps_string(format[i].fourcc);

        min_width  = format[i].min_size.width;
        min_height = format[i].min_size.height;

        max_width  = format[i].max_size.width;
        max_height = format[i].max_size.height;

        if (caps_string != NULL)
        {
            GstStructure* structure;

            structure = gst_structure_from_string (caps_string, NULL);
            gst_structure_set (structure,
                               "width", GST_TYPE_INT_RANGE, min_width, max_width,
                               "height", GST_TYPE_INT_RANGE, min_height, max_height,
                               "framerate", GST_TYPE_FRACTION_RANGE,
                               1, 1,
                               100, 1,
                               NULL);
            gst_caps_append_structure (caps, structure);
        }
    }

    GST_ERROR(gst_caps_to_string(caps));

    return caps;
}


static GstCaps* gst_tcam_get_caps (GstBaseSrc* src,
                                   GstCaps* filter)
{
    GstTcam* self = GST_TCAM(src);
    GstCaps* caps;

    if (self->all_caps != NULL)
        caps = gst_caps_copy (self->all_caps);
    else
        caps = gst_caps_new_any ();

    GST_LOG_OBJECT (self, "Available caps = %" GST_PTR_FORMAT, caps);

    return caps;
}


static void gst_tcam_callback (const struct tcam_image_buffer* buffer,
                               void* data)
{
    GstTcam* self = GST_TCAM(data);

    self->ptr = buffer;
    self->new_buffer = TRUE;
}


static gboolean gst_tcam_set_caps (GstBaseSrc* src,
                                   GstCaps* caps)
{

    GstTcam* self = GST_TCAM(src);

    GstStructure *structure;

    int height = 0;
    int width = 0;
    const GValue* frame_rate;
    const char* caps_string;
    const char* format_string;

    GST_LOG_OBJECT (self, "Requested caps = %" GST_PTR_FORMAT, caps);

    tcam_capture_device_stop_stream(self->device);

    structure = gst_caps_get_structure (caps, 0);

    gst_structure_get_int (structure, "width", &width);
    gst_structure_get_int (structure, "height", &height);
    frame_rate = gst_structure_get_value (structure, "framerate");
    format_string = gst_structure_get_string (structure, "format");

    uint32_t fourcc = tcam_fourcc_from_gst_1_0_caps_string(gst_structure_get_name (structure), format_string);

    struct tcam_video_format format = {};

    format.fourcc = fourcc;
    format.width = width;
    format.height = height;
    // TODO
    format.framerate = 10.0;

    bool ret = tcam_capture_device_set_image_format(self->device, &format);

    if (!ret)
    {
        GST_ERROR("Unable to set format in device");

        return FALSE;
    }

    if (frame_rate != NULL) {
        double dbl_frame_rate;

        dbl_frame_rate = (double) gst_value_get_fraction_numerator (frame_rate) /
            (double) gst_value_get_fraction_denominator (frame_rate);

        GST_DEBUG_OBJECT (self, "Frame rate = %g Hz", dbl_frame_rate);
    }

    if (self->fixed_caps != NULL)
        gst_caps_unref (self->fixed_caps);

    caps_string = tcam_fourcc_to_gst_1_0_caps_string(fourcc);
    if (caps_string != NULL) {
        GstStructure *structure;
        GstCaps *caps;

        caps = gst_caps_new_empty ();
        structure = gst_structure_from_string (caps_string, NULL);
        gst_structure_set (structure,
                           "width", G_TYPE_INT, width,
                           "height", G_TYPE_INT, height,
                           NULL);

        if (frame_rate != NULL)
            gst_structure_set_value (structure, "framerate", frame_rate);

        gst_caps_append_structure (caps, structure);

        self->fixed_caps = caps;
    } else
        self->fixed_caps = NULL;

    GST_LOG_OBJECT (self, "Start acquisition");

    self->timestamp_offset = 0;
    self->last_timestamp = 0;

    tcam_capture_device_start_stream(self->device, gst_tcam_callback, self);

    GST_ERROR("successfully set caps");

    return TRUE;
}


void gst_tcam_init_camera (GstTcam* self)
{
    GST_DEBUG_OBJECT (self, "Initializing device.");

    if (self->device != NULL)
        tcam_destroy_capture_device(self->device);

    tcam_device_index* index = tcam_create_device_index();

    /* wait for network cameras to be indexed */
    /* ? does this have to be incrreased ? */
    sleep(2);

    int dev_count = tcam_device_index_get_device_count(index);

    struct tcam_device_info infos[dev_count];

    int devs = tcam_device_index_get_device_infos (index,
                                            infos,
                                            dev_count);

    struct tcam_device_info info = {};

    GST_DEBUG_OBJECT (self, "Searching for device with serial %s.", self->device_serial);
    GST_DEBUG_OBJECT (self, "Found %d devices.", dev_count);


    unsigned int i = 0;
    for (i; i < devs; ++i)
    {
        if (strcmp(infos[i].serial_number, self->device_serial) == 0)
        {
            GST_DEBUG_OBJECT (self, "Found device.");

            self->device = tcam_create_new_capture_device(&infos[i]);

            /* info = infos[i]; */
            break;
        }
    }

    tcam_destroy_device_index (index);

    if (self->device == NULL)
    {
        GST_ERROR("Unable to open device.");
        /* TODO add pipeline termination */
    }
}


static gboolean gst_tcam_start (GstBaseSrc *src)
{
    GstTcam* self = GST_TCAM(src);

    self->run = 1000;

    if (self->device == NULL)
        gst_tcam_init_camera(self);

    self->all_caps = gst_tcam_get_all_camera_caps (self);

    return TRUE;
}


gboolean gst_tcam_stop (GstBaseSrc* src)
{
    GstTcam* self = GST_TCAM(src);

    tcam_capture_device_stop_stream(self->device);

    if (self->all_caps != NULL)
    {
        gst_caps_unref (self->all_caps);
        self->all_caps = NULL;
    }

    GST_DEBUG_OBJECT (self, "Stopped acquisition");

    return TRUE;
}


static void gst_tcam_get_times (GstBaseSrc* basesrc,
                                GstBuffer* buffer,
                                GstClockTime* start,
                                GstClockTime* end)
{
    if (gst_base_src_is_live (basesrc))
    {
        GstClockTime timestamp = GST_BUFFER_PTS (buffer);

        if (GST_CLOCK_TIME_IS_VALID (timestamp))
        {
            GstClockTime duration = GST_BUFFER_DURATION (buffer);

            if (GST_CLOCK_TIME_IS_VALID (duration))
            {
                *end = timestamp + duration;
            }
            *start = timestamp;
        }
    }
    else
    {
        *start = -1;
        *end = -1;
    }
}


static GstFlowReturn gst_tcam_create (GstPushSrc* push_src,
                                      GstBuffer** buffer)
{
    guint64 timestamp_ns = 0;

    GstTcam* self = GST_TCAM (push_src);

    while (self->new_buffer == false)
    {
        usleep(500);
    }

    if (self->ptr == NULL)
        return GST_FLOW_ERROR;

    *buffer = gst_buffer_new_wrapped_full (0, self->ptr->pData, self->ptr->length,
                                           0, self->ptr->length, NULL, NULL);

    if (!gst_base_src_get_do_timestamp(GST_BASE_SRC(push_src))) {
        if (self->timestamp_offset == 0) {
            self->timestamp_offset = timestamp_ns;
            self->last_timestamp = timestamp_ns;
        }

        GST_BUFFER_DURATION (*buffer) = timestamp_ns - self->last_timestamp;

        self->last_timestamp = timestamp_ns;
    }

    return GST_FLOW_OK;
}


static GstCaps* gst_tcam_fixate_caps (GstBaseSrc* bsrc,
                                      GstCaps* caps)
{
    GstTcam* self = GST_TCAM(bsrc);

    GstStructure* structure;
    gint width = 0;
    gint height = 0;
    double frame_rate = 0.0;

    structure = gst_caps_get_structure (caps, 0);

    gst_structure_fixate_field_nearest_int (structure, "width", width);
    gst_structure_fixate_field_nearest_int (structure, "height", height);
    gst_structure_fixate_field_nearest_fraction (structure, "framerate", (double) (0.5 + frame_rate), 1);

    GST_DEBUG_OBJECT (self, "Fixate caps");

    return GST_BASE_SRC_CLASS(gst_tcam_parent_class)->fixate(bsrc, caps);
}


static void gst_tcam_init (GstTcam* self)
{

    gst_base_src_set_live (GST_BASE_SRC (self), TRUE);
    gst_base_src_set_format (GST_BASE_SRC (self), GST_FORMAT_TIME);

    self->device_serial = NULL;

    self->n_buffers = GST_TCAM_DEFAULT_N_BUFFERS;
    self->payload = 0;

    self->device = NULL;

    self->all_caps = NULL;
    self->fixed_caps = NULL;
}


static void gst_tcam_finalize (GObject* object)
{
    GstTcam* self = GST_TCAM (object);

    if (self->device != NULL)
    {
        self->device = NULL;
    }

    if (self->all_caps != NULL)
    {
        gst_caps_unref (self->all_caps);
        self->all_caps = NULL;
    }
    if (self->fixed_caps != NULL)
    {
        gst_caps_unref (self->fixed_caps);
        self->fixed_caps = NULL;
    }

    if (self->device_serial != NULL)
    {
        g_free (self->device_serial);
        self->device_serial = NULL;
    }

    G_OBJECT_CLASS (gst_tcam_parent_class)->finalize (object);
}


static void gst_tcam_set_property (GObject* object,
                                   guint prop_id,
                                   const GValue* value,
                                   GParamSpec* pspec)
{
    GstTcam* self = GST_TCAM (object);

    switch (prop_id)
    {
        case PROP_SERIAL:
            g_free (self->device_serial);
            self->device_serial = g_strdup (g_value_get_string (value));
            GST_LOG_OBJECT (self, "Set camera name to %s", self->device_serial);
            break;
        case PROP_NUM_BUFFERS:
            self->n_buffers = g_value_get_int (value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void gst_tcam_get_property (GObject* object,
                                   guint prop_id,
                                   GValue* value,
                                   GParamSpec* pspec)
{
    GstTcam* self = GST_TCAM (object);

    switch (prop_id)
    {
        case PROP_SERIAL:
            g_value_set_string (value, self->device_serial);
            break;
        case PROP_DEVICE:
            g_value_set_pointer (value, self->device);
            break;
        case PROP_NUM_BUFFERS:
            g_value_set_int (value, self->n_buffers);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void gst_tcam_class_init (GstTcamClass* klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GstElementClass *element_class = GST_ELEMENT_CLASS (klass);
    GstBaseSrcClass *gstbasesrc_class = GST_BASE_SRC_CLASS (klass);
    GstPushSrcClass *gstpushsrc_class = GST_PUSH_SRC_CLASS (klass);

    gobject_class->finalize = gst_tcam_finalize;
    gobject_class->set_property = gst_tcam_set_property;
    gobject_class->get_property = gst_tcam_get_property;

    g_object_class_install_property
        (gobject_class,
         PROP_SERIAL,
         g_param_spec_string ("serial",
                              "Camera serial",
                              "Serial of the camera",
                              NULL,
                              G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
    g_object_class_install_property
        (gobject_class,
         PROP_DEVICE,
         g_param_spec_pointer ("camera",
                              "Camera Object",
                              "Camera instance to retrieve additional information",
                              G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

    g_object_class_install_property
        (gobject_class,
         PROP_NUM_BUFFERS,
         g_param_spec_int ("num-buffers",
                           "Number of Buffers",
                           "Number of video buffers to allocate for video frames",
                           1, G_MAXINT, GST_TCAM_DEFAULT_N_BUFFERS,
                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

    GST_DEBUG_CATEGORY_INIT (tcam_debug, "tcamsrc", 0, "tcam interface");

    gst_element_class_set_details_simple (element_class,
                                          "Tcam Video Source",
                                          "Source/Video",
                                          "Tcam based source",
                                          "Edgar Thier <support@theimagingsource.com>");

    gst_element_class_add_pad_template (element_class,
                                        gst_static_pad_template_get (&tcam_src_template));

    gstbasesrc_class->get_caps = gst_tcam_get_caps;
    gstbasesrc_class->set_caps = gst_tcam_set_caps;
    gstbasesrc_class->fixate = gst_tcam_fixate_caps;
    gstbasesrc_class->start = gst_tcam_start;
    gstbasesrc_class->stop = gst_tcam_stop;

    gstbasesrc_class->get_times = gst_tcam_get_times;

    /* gstbasesrc_class-> = gst_tcam_fill; */

    gstpushsrc_class->create = gst_tcam_create;
}


static gboolean plugin_init (GstPlugin* plugin)
{
    return gst_element_register (plugin, "tcamsrc", GST_RANK_NONE, GST_TYPE_TCAM);
}

#ifndef PACKAGE
#define PACKAGE "tcam"
#endif


GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
                   GST_VERSION_MINOR,
                   tcamsrc,
                   "TCam Video Source",
                   plugin_init,
                   "1.0.0",
                   "LGPL",
                   "tcamsrc",
                   "theimagingsource.com")
