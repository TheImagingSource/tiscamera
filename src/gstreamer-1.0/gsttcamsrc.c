/*
 * Copyright 2014 The Imaging Source Europe GmbH
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

#include "gsttcamsrc.h"
#include "gsttcambase.h"

#include "tcamprop.h"
#include "tcam_c.h"
#include <unistd.h>
#include <stdlib.h>

#include <stdio.h>


#define GST_TCAM_DEFAULT_N_BUFFERS		10

GST_DEBUG_CATEGORY_STATIC (tcam_debug);
#define GST_CAT_DEFAULT tcam_debug

static
GSList* gst_tcam_src_get_property_names(TcamProp* self);
static
gchar *gst_tcam_src_get_property_type (TcamProp *self, gchar *name);
static
gboolean gst_tcam_src_get_tcam_property (TcamProp *self,
					 gchar *name,
					 GValue *value,
					 GValue *min,
					 GValue *max,
					 GValue *def,
					 GValue *step,
					 GValue *type);
static
gboolean gst_tcam_src_set_tcam_property (TcamProp *self,
					 gchar *name,
					 const GValue *value);
static
GSList *gst_tcam_src_get_device_serials (TcamProp *self);
static
gboolean gst_tcam_src_get_device_info (TcamProp *self,
				       const char *serial,
				       char **name,
				       char **identifier,
				       char **connection_type);

static void gst_tcam_src_prop_init (TcamPropInterface *iface)
{
	iface->get_property_names = gst_tcam_src_get_property_names;
	iface->get_property_type = gst_tcam_src_get_property_type;
	iface->get_property = gst_tcam_src_get_tcam_property;
	iface->set_property = gst_tcam_src_set_tcam_property;
	iface->get_device_serials = gst_tcam_src_get_device_serials;
	iface->get_device_info = gst_tcam_src_get_device_info;
}

G_DEFINE_TYPE_WITH_CODE (GstTcam, gst_tcam, GST_TYPE_PUSH_SRC,
                         G_IMPLEMENT_INTERFACE (TCAM_TYPE_PROP,
                                                gst_tcam_src_prop_init));


static gboolean
get_property_by_name (GstTcam *self, gchar *name,
		      struct tcam_device_property *prop)
{
    gboolean ret = FALSE;

    int count = tcam_capture_device_get_properties_count (self->device);
    if (count){
	struct tcam_device_property *props;

	props = g_malloc_n (sizeof (struct tcam_device_property), count);
	if (tcam_capture_device_get_properties (self->device,
						props,
						count) > 0){
	    int i;
	    for (i = 0; i < count; i++ ){
		if (!strcmp (props[i].name, name)){
		    memcpy (prop,
			    &props[i],
			    sizeof(struct tcam_device_property));
		    ret = TRUE;
		    break;
		}
	    }
	}
	g_free (props);
    }

    return ret;
}


struct property_type_map
{
    enum TCAM_PROPERTY_TYPE typecode;
    gchar *typename;
};

/**
 * gst_tcam_get_property_type:
 * @self: a #GstTcamProp
 * @name: a #char* identifying the property to query
 *
 * Return the type of a property
 *
 * Returns: (transfer full): A string describing the property type
 */
static gchar* gst_tcam_src_get_property_type (TcamProp* iface, gchar* name)
{
    gchar *ret = NULL;
    GstTcam *self = GST_TCAM (iface);
    struct tcam_device_property prop;
    struct property_type_map map[] = {
	{ TCAM_PROPERTY_TYPE_BOOLEAN, "boolean" },
	{ TCAM_PROPERTY_TYPE_INTEGER, "integer" },
	{ TCAM_PROPERTY_TYPE_DOUBLE,  "double" },
	{ TCAM_PROPERTY_TYPE_STRING,  "string" },
	{ TCAM_PROPERTY_TYPE_ENUMERATION, "enum" },
	{ TCAM_PROPERTY_TYPE_BUTTON,  "button" },
    };

    g_return_val_if_fail (self->device != NULL, NULL);

    g_return_val_if_fail (get_property_by_name (self,
						name,
						&prop), NULL);
    int i;

    for (i = 0; i < G_N_ELEMENTS (map); i++){
	if (prop.type == map[i].typecode){
	    ret = g_strdup (map[i].typename);
	    break;
	}
    }

    if (ret == NULL){
	ret = g_strdup ("unknown");
    }

    return ret;
}

/**
 * gst_tcam_src_get_property_names:
 * @self: a #GstTcam
 *
 * Return a list of property names
 *
 * Returns: (element-type utf8) (transfer full): list of property names
 */
static
GSList* gst_tcam_src_get_property_names(TcamProp* iface)
{
    GSList *ret = NULL;
    GstTcam *self = GST_TCAM (iface);

    g_return_val_if_fail (self->device != NULL, NULL);

    int count = tcam_capture_device_get_properties_count (self->device);
    if (count){
	struct tcam_device_property *props;

	props = g_malloc_n (sizeof (struct tcam_device_property), count);
	if (tcam_capture_device_get_properties (self->device,
						props,
						count) > 0){
	    int i;

	    for (i = 0; i < count; i++ ){
		ret = g_slist_append (ret,
				      g_strdup (props[i].name));
	    }
	}
	g_free (props);
    }

    return ret;
}


static
gboolean gst_tcam_src_get_tcam_property (TcamProp *iface,
					 gchar *name,
					 GValue *value,
					 GValue *min,
					 GValue *max,
					 GValue *def,
					 GValue *step,
					 GValue *type)
{
    gboolean ret = TRUE;
    GstTcam *self = GST_TCAM (iface);
    struct tcam_device_property prop;

    if (get_property_by_name (self, name, &prop) < 0){
	return FALSE;
    }

    if (tcam_capture_device_get_property (self->device,
					  &prop) < 0){
	return FALSE;
    }

    if (type){
	g_value_init (type, G_TYPE_GTYPE);
    }

    switch (prop.type){
    case TCAM_PROPERTY_TYPE_INTEGER:
    case TCAM_PROPERTY_TYPE_ENUMERATION:
	if (value){
	    g_value_init (value, G_TYPE_INT);
	    g_value_set_int (value, prop.value.i.value);
	}
	if (min){
	    g_value_init (min, G_TYPE_INT);
	    g_value_set_int (min, prop.value.i.min);
	}
	if (max){
	    g_value_init (max, G_TYPE_INT);
	    g_value_set_int (max, prop.value.i.max);
	}
	if (def){
	    g_value_init (def, G_TYPE_INT);
	    g_value_set_int (def, prop.value.i.default_value);
	}
	if (step){
	    g_value_init (step, G_TYPE_INT);
	    g_value_set_int (step, prop.value.i.step);
	}
	if (type){
	    g_value_set_gtype (type, G_TYPE_INT);
	}
	break;
    case TCAM_PROPERTY_TYPE_DOUBLE:
	if (value){
	    g_value_init (value, G_TYPE_DOUBLE);
	    g_value_set_int (value, prop.value.d.value);
	}
	if (min){
	    g_value_init (min, G_TYPE_DOUBLE);
	    g_value_set_int (min, prop.value.d.min);
	}
	if (max){
	    g_value_init (max, G_TYPE_DOUBLE);
	    g_value_set_int (max, prop.value.d.max);
	}
	if (def){
	    g_value_init (def, G_TYPE_DOUBLE);
	    g_value_set_int (def, prop.value.d.default_value);
	}
	if (step){
	    g_value_init (step, G_TYPE_DOUBLE);
	    g_value_set_int (step, prop.value.d.step);
	}
	if (type){
	    g_value_set_gtype (type, G_TYPE_DOUBLE);
	}
	break;
    case TCAM_PROPERTY_TYPE_STRING:
	if (value){
	    g_value_init (value, G_TYPE_STRING);
	    g_value_set_string (value, prop.value.s.value);
	}
	if (min){
	    g_value_init (min, G_TYPE_STRING);
	}
	if (max){
	    g_value_init (max, G_TYPE_STRING);
	}
	if (def){
	    g_value_init (def, G_TYPE_STRING);
	    g_value_set_string (def, prop.value.s.default_value);
	}
	if (step){
	    g_value_init (def, G_TYPE_STRING);
	}
	if (type){
	    g_value_set_gtype (type, G_TYPE_STRING);
	}
	break;
    case TCAM_PROPERTY_TYPE_BOOLEAN:
    case TCAM_PROPERTY_TYPE_BUTTON:
	if (value){
	    g_value_init (value, G_TYPE_BOOLEAN);
	    g_value_set_boolean (value, prop.value.b.value);
	}
	if (min){
	    g_value_init (min, G_TYPE_BOOLEAN);
	}
	if (max){
	    g_value_init (max, G_TYPE_BOOLEAN);
	    g_value_set_boolean (max, TRUE);
	}
	if (def){
	    g_value_init (def, G_TYPE_BOOLEAN);
	    g_value_set_boolean (def, prop.value.b.default_value);
	}
	if (step){
	    g_value_init (step, G_TYPE_BOOLEAN);
	}
	if (type){
	    g_value_set_gtype (type, G_TYPE_BOOLEAN);
	}
	break;
    default:
	if (value){
	    g_value_init (value, G_TYPE_INT);
	}
	ret = FALSE;
	break;
    }

    return ret;
}

static
gboolean gst_tcam_src_set_tcam_property (TcamProp *iface,
					 gchar *name,
					 const GValue *value)
{
    gboolean ret = TRUE;
    GstTcam *self = GST_TCAM (iface);
    struct tcam_device_property prop;

    if (get_property_by_name (self, name, &prop) < 0){
	return FALSE;
    }
    if (tcam_capture_device_get_property (self->device,
					  &prop) < 0){
	return FALSE;
    }

    switch (prop.type){
    case TCAM_PROPERTY_TYPE_INTEGER:
    case TCAM_PROPERTY_TYPE_ENUMERATION:
	prop.value.i.value = g_value_get_int (value);
	break;
    case TCAM_PROPERTY_TYPE_DOUBLE:
	prop.value.d.value = g_value_get_double (value);
	break;
    case TCAM_PROPERTY_TYPE_STRING:
	strcpy (prop.value.s.value, g_value_get_string (value));
	break;
    case TCAM_PROPERTY_TYPE_BOOLEAN:
    case TCAM_PROPERTY_TYPE_BUTTON:
	prop.value.b.value = g_value_get_boolean (value);
	break;
    }

    return tcam_capture_device_set_property (self->device,
					     &prop ) >= 0;
}

static
GSList *gst_tcam_src_get_device_serials (TcamProp *self)
{
    int count = tcam_device_index_get_device_count ();
    GSList *ret = NULL;
    struct tcam_device_info *info;

    if (count <= 0){
	return NULL;
    }

    info = g_new0 (struct tcam_device_info, count);
    if (info == NULL){
	return NULL;
    }

    if (tcam_device_index_get_device_infos (info,
					    count) <= 0){
	g_free (info);
	return NULL;
    }

    int i;
    for (i = 0; i < count; i++){
	ret = g_slist_append (ret,
			      g_strndup (info[i].serial_number,
					 sizeof(info[i].serial_number)));
    }

    g_free (info);

    return ret;
}

static
gboolean gst_tcam_src_get_device_info (TcamProp *self,
				       const char *serial,
				       char **name,
				       char **identifier,
				       char **connection_type)
{
    int count = tcam_device_index_get_device_count ();
    gboolean ret = FALSE;
    struct tcam_device_info *info;

    if (count <= 0){
	return FALSE;
    }

    info = g_new0 (struct tcam_device_info, count);
    if (info == NULL){
	return FALSE;
    }

    if (tcam_device_index_get_device_infos (info,
					    count) <= 0){
	g_free (info);
	return NULL;
    }

    int i;
    for (i = 0; i < count; i++){
	if (!strncmp (serial, info[i].serial_number,
		      sizeof (info[i].serial_number))){
	    ret = TRUE;
	    if (name){
		*name = g_strndup (info[i].name, sizeof (info[i].name));
	    }
	    if (identifier){
		*identifier = g_strndup (info[i].identifier,
					sizeof (info[i].identifier));
	    }
	    if (connection_type) {
		switch (info[i].type){
		case TCAM_DEVICE_TYPE_V4L2:
		    *connection_type = g_strdup ("v4l2");
		    break;
		case TCAM_DEVICE_TYPE_ARAVIS:
		    *connection_type = g_strdup ("aravis");
		    break;
		default:
		    *connection_type = g_strdup ("unknown");
		    break;
		}
	    }
	    break;
	}
    }

    g_free (info);

    return ret;
}



enum
{
    PROP_0,
    PROP_SERIAL,
    PROP_DEVICE,
    PROP_NUM_BUFFERS,
};


/* G_DEFINE_INTERFACE (GstTcam, gst_tcam, TCAM_TYPE_PROP); */


static GstStaticPadTemplate tcam_src_template = GST_STATIC_PAD_TEMPLATE ("src",
                                                                         GST_PAD_SRC,
                                                                         GST_PAD_ALWAYS,
                                                                         GST_STATIC_CAPS ("ANY"));

static GstCaps* gst_tcam_fixate_caps (GstBaseSrc* bsrc,
                                      GstCaps* caps);

static GstCaps* gst_tcam_get_all_camera_caps (GstTcam* self)
{
    GstCaps* caps;
    gint64* pixel_formats;
    double min_frame_rate, max_frame_rate;
    int min_height, min_width;
    int max_height, max_width;
    int n_pixel_formats;

    g_return_val_if_fail (GST_IS_TCAM (self), NULL);

    if (self->device == NULL)
        return NULL;

    int format_count = tcam_capture_device_get_image_format_descriptions_count(self->device);
    struct tcam_video_format_description format [format_count];

    n_pixel_formats = tcam_capture_device_get_image_format_descriptions (self->device,
                                                                         format,
                                                                         format_count);

    GST_DEBUG("Found %i pixel formats", n_pixel_formats);

    caps = gst_caps_new_empty ();
    unsigned int i;
    for (i = 0; i < n_pixel_formats; i++)
    {
        const char* caps_string = tcam_fourcc_to_gst_1_0_caps_string(format[i].fourcc);

        struct tcam_resolution_description res [format[i].resolution_count];

        int res_count = tcam_capture_device_get_format_resolution(self->device,
                                                                  &format[i],
                                                                  res,
                                                                  format[i].resolution_count);

        GST_DEBUG("Found %i resolutions", res_count);

        unsigned int j;
        for (j = 0; j < res_count; j++)
        {

            min_width  = res[j].min_size.width;
            min_height = res[j].min_size.height;

            max_width  = res[j].max_size.width;
            max_height = res[j].max_size.height;

            if (caps_string != NULL)
            {
                GstStructure* structure;

                structure = gst_structure_from_string (caps_string, NULL);

                if (min_width < max_width)
                {

                    gst_structure_set (structure,
                                       "width", GST_TYPE_INT_RANGE, min_width, max_width,
                                       "height", GST_TYPE_INT_RANGE, min_height, max_height,
                                       "framerate", GST_TYPE_FRACTION_RANGE,
                                       1, 1,
                                       120, 1,
                                       NULL);
                }
                else
                {

                    /* TODO how to handle framerates */

                    gst_structure_set (structure,
                                       "width", G_TYPE_INT, max_width,
                                       "height", G_TYPE_INT, max_height,
                                       "framerate", GST_TYPE_FRACTION_RANGE,
                                       1, 1,
                                       120, 1,
                                       NULL);

                    /* double framerates [res[j].framerate_count]; */

                    /* int ret = tcam_capture_device_get_resolution_framerate(self->device, */
                    /*                                                        &format[i], */
                    /*                                                        &res[j], */
                    /*                                                        framerates, */
                    /*                                                        res[j].framerate_count); */

                    /* unsigned int k; */
                    /* for (k = 0; k < ret; k++) */
                    /* { */
                    /*     GST_DEBUG_OBJECT(self, "999999=============================%f", framerates[k]); */

                    /*     int max_frame_rate_numerator; */
                    /*     int max_frame_rate_denominator; */
                    /*     gst_util_double_to_fraction(framerates[k], */
                    /*                                 &max_frame_rate_numerator, */
                    /*                                 &max_frame_rate_denominator); */


                    /*     gst_structure_set (structure, */
                    /*                        "framerate", GST_TYPE_FRACTION_RANGE, */
                    /*                        max_frame_rate_numerator, max_frame_rate_denominator, */
                    /*                        max_frame_rate_numerator, max_frame_rate_denominator, */
                    /*                        NULL); */

                    /*     gst_caps_append_structure (caps, structure); */

                    /* } */

                }
                gst_caps_append_structure (caps, structure);
            }
        }
    }

    GST_INFO(gst_caps_to_string(caps));

    return caps;
}


static gboolean gst_tcam_negotiate (GstBaseSrc* basesrc)
{

    GstCaps *thiscaps;
    GstCaps *caps = NULL;
    GstCaps *peercaps = NULL;
    gboolean result = FALSE;

    GstTcam* self = GST_TCAM(basesrc);
    /* obj = v4l2src->v4l2object; */


    /* /\* We don't allow renegotiation, just return TRUE in that case *\/ */
    /* if (GST_V4L2_IS_ACTIVE (obj)) */
    /*     return TRUE; */

    /* first see what is possible on our source pad */
    thiscaps = gst_pad_query_caps (GST_BASE_SRC_PAD (basesrc), NULL);
    GST_DEBUG_OBJECT (basesrc, "!!!!!caps of src: %" GST_PTR_FORMAT, thiscaps);

    // nothing or anything is allowed, we're done
    if (thiscaps == NULL || gst_caps_is_any (thiscaps))
    {
        goto no_nego_needed;
    }
    /* get the peer caps */
    peercaps = gst_pad_peer_query_caps (GST_BASE_SRC_PAD (basesrc), thiscaps);
    GST_DEBUG_OBJECT (basesrc, "=====-caps of peer: %" GST_PTR_FORMAT, peercaps);

    if (peercaps && !gst_caps_is_any (peercaps))
    {
        GstCaps *icaps = NULL;
        int i;

        /* Prefer the first caps we are compatible with that the peer proposed */
        for (i = 0; i < gst_caps_get_size (peercaps); i++)
        {
            /* get intersection */
            GstCaps *ipcaps = gst_caps_copy_nth (peercaps, i);

            GST_DEBUG_OBJECT (basesrc, "peer: %" GST_PTR_FORMAT, ipcaps);

            icaps = gst_caps_intersect (thiscaps, ipcaps);
            gst_caps_unref (ipcaps);

            if (!gst_caps_is_empty (icaps))
                break;

            gst_caps_unref (icaps);
            icaps = NULL;
        }
        GST_DEBUG_OBJECT (basesrc, "/////intersect: %" GST_PTR_FORMAT, icaps);

        if (icaps)
        {
            /* If there are multiple intersections pick the one with the smallest
             * resolution strictly bigger then the first peer caps */
            if (gst_caps_get_size (icaps) > 1)
            {
                GST_DEBUG_OBJECT(basesrc, "1111111111111");

                GstStructure *s = gst_caps_get_structure (peercaps, 0);
                int best = 0;
                int twidth, theight;
                int width = G_MAXINT, height = G_MAXINT;

                if (gst_structure_get_int (s, "width", &twidth)
                    && gst_structure_get_int (s, "height", &theight))
                {

                    /* Walk the structure backwards to get the first entry of the
                     * smallest resolution bigger (or equal to) the preferred resolution)
                     */
                    for (i = gst_caps_get_size (icaps) - 1; i >= 0; i--)
                    {
                        GstStructure *is = gst_caps_get_structure (icaps, i);
                        int w, h;

                        if (gst_structure_get_int (is, "width", &w)
                            && gst_structure_get_int (is, "height", &h))
                        {
                            if (w >= twidth && w <= width && h >= theight && h <= height)
                            {
                                width = w;
                                height = h;
                                best = i;
                            }
                        }
                    }
                }

                caps = gst_caps_copy_nth (icaps, best);
                gst_caps_unref (icaps);
            }
            else
            {
                // ensure that there is no range but a high resolution with adequate framerate

                GstStructure *s = gst_caps_get_structure (peercaps, 0);
                int best = 0;
                int twidth, theight;
                int width = G_MAXINT, height = G_MAXINT;



                /* Walk the structure backwards to get the first entry of the
                 * smallest resolution bigger (or equal to) the preferred resolution)
                 */
                for (i = 0; i >= gst_caps_get_size (icaps); i++)
                {
                    GstStructure *is = gst_caps_get_structure (icaps, i);
                    int w, h;
                    int max_width;

                    if (gst_structure_get_int (is, "width", &w)
                        && gst_structure_get_int (is, "height", &h))
                    {
                        if (w >= twidth && w <= width && h >= theight && h <= height)
                        {
                            width = w;
                            height = h;
                            best = i;
                        }
                    }
                }


                /* caps = icaps; */
                caps = gst_caps_copy_nth (icaps, best);

                GstStructure* structure;
                double frame_rate = 0.0;

                structure = gst_caps_get_structure (caps, 0);

                gst_structure_fixate_field_nearest_int (structure, "width", G_MAXINT);
                gst_structure_fixate_field_nearest_int (structure, "height", G_MAXINT);
                gst_structure_fixate_field_nearest_fraction (structure, "framerate", (double) (0.5 + frame_rate), 1);

                gst_caps_unref (icaps);

            }
        }
        gst_caps_unref (thiscaps);
    }
    else
    {
        /* no peer or peer have ANY caps, work with our own caps then */
        caps = thiscaps;
    }

    if (peercaps)
    {
        gst_caps_unref (peercaps);
    }



    if (caps)
    {
        caps = gst_caps_truncate (caps);

        /* now fixate */
        if (!gst_caps_is_empty (caps))
        {
            caps = gst_tcam_fixate_caps (basesrc, caps);
            GST_DEBUG_OBJECT (basesrc, "fixated to: %" GST_PTR_FORMAT, caps);

            if (gst_caps_is_any (caps))
            {
                /* hmm, still anything, so element can do anything and
                 * nego is not needed */
                result = TRUE;
            }
            else if (gst_caps_is_fixed (caps))
            {
                /* yay, fixed caps, use those then */
                result = gst_base_src_set_caps (basesrc, caps);
            }
        }
        gst_caps_unref (caps);
    }
    return result;

no_nego_needed:
    {
        GST_DEBUG_OBJECT (basesrc, "no negotiation needed");
        if (thiscaps)
            gst_caps_unref (thiscaps);
        return TRUE;
    }

    return TRUE;
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
    GST_ERROR( "Requested caps = %" GST_PTR_FORMAT, caps);

    tcam_capture_device_stop_stream(self->device);

    structure = gst_caps_get_structure (caps, 0);

    gst_structure_get_int (structure, "width", &width);
    gst_structure_get_int (structure, "height", &height);
    frame_rate = gst_structure_get_value (structure, "framerate");
    format_string = gst_structure_get_string (structure, "format");

    uint32_t fourcc = tcam_fourcc_from_gst_1_0_caps_string(gst_structure_get_name (structure), format_string);


    double framerate = (double) gst_value_get_fraction_numerator (frame_rate) /
        (double) gst_value_get_fraction_denominator (frame_rate);

    struct tcam_video_format format = {};

    format.fourcc = fourcc;
    format.width = width;
    format.height = height;
    format.framerate = framerate;

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

    self->streamobject = tcam_capture_device_start_stream(self->device, gst_tcam_callback, self);

    GST_INFO("successfully set caps to: %", GST_PTR_FORMAT, caps);

    return TRUE;
}


void gst_tcam_init_camera (GstTcam* self)
{
    GST_DEBUG_OBJECT (self, "Initializing device.");

    if (self->device != NULL)
        tcam_destroy_capture_device(self->device);

    int dev_count = tcam_device_index_get_device_count();

    struct tcam_device_info infos[dev_count];

    int devs = tcam_device_index_get_device_infos (infos,
                                                   dev_count);

    struct tcam_device_info info = {};


    GST_DEBUG_OBJECT (self, "Found %d devices.", dev_count);

    if (self->device_serial != NULL)
        GST_DEBUG_OBJECT (self, "Searching for device with serial %s.", self->device_serial);
    else
        GST_DEBUG_OBJECT (self, "No serial given. Opening first available device.");


    unsigned int i = 0;
    for (i; i < devs; ++i)
    {
        if (self->device_serial != NULL)
        {
            if (strcmp(infos[i].serial_number, self->device_serial) == 0)
            {
                GST_DEBUG_OBJECT (self, "Found device.");

                self->device = tcam_create_new_capture_device(&infos[i]);
                break;
            }
        }
        else
        {
            self->device = tcam_create_new_capture_device(&infos[i]);
            break;
        }
    }

    if (self->device == NULL)
    {
        GST_ERROR("Unable to open device.");
        /* TODO add pipeline termination */
    }
}


static void gst_tcam_close_camera (GstTcam* self)
{
    if (self->device != NULL)
    {
        tcam_capture_device_stop_stream(self->device);
        tcam_destroy_capture_device(self->device);
        self->device = NULL;
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
    {}

    self->new_buffer = false;
    if (self->ptr == NULL)
        return GST_FLOW_ERROR;

    *buffer = gst_buffer_new_wrapped_full (0, self->ptr->pData, self->ptr->length,
                                           0, self->ptr->length, NULL, NULL);

    if (!gst_base_src_get_do_timestamp(GST_BASE_SRC(push_src)))
    {
        if (self->timestamp_offset == 0)
        {
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

    GST_DEBUG_OBJECT (self, "Fixated caps to : %", GST_PTR_FORMAT, caps);

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

    gst_tcam_init_camera(self);
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
        {
            GstState state;
            gst_element_get_state(GST_ELEMENT(self), &state, NULL, 200);
            if (state == GST_STATE_NULL)
            {
                g_free (self->device_serial);
                self->device_serial = g_strdup (g_value_get_string (value));
                GST_LOG_OBJECT (self, "Set camera name to %s", self->device_serial);

                gst_tcam_close_camera(self);

                gst_tcam_init_camera(self);
            }
            break;
        }
        case PROP_NUM_BUFFERS:
        {
            self->n_buffers = g_value_get_int (value);
            break;
        }
        default:
        {
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
        }
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
                                          "The Imaging Source <support@theimagingsource.com>");

    gst_element_class_add_pad_template (element_class,
                                        gst_static_pad_template_get (&tcam_src_template));

    gstbasesrc_class->get_caps = gst_tcam_get_caps;
    gstbasesrc_class->set_caps = gst_tcam_set_caps;
    gstbasesrc_class->fixate = gst_tcam_fixate_caps;
    gstbasesrc_class->start = gst_tcam_start;
    gstbasesrc_class->stop = gst_tcam_stop;
    gstbasesrc_class->negotiate = gst_tcam_negotiate;
    gstbasesrc_class->get_times = gst_tcam_get_times;

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
                   "Proprietary",
                   "tcamsrc",
                   "theimagingsource.com")
