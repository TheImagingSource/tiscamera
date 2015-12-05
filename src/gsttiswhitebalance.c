/*
 * Copyright 2013 The Imaging Source Europe GmbH
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

/**
 * SECTION:element-gsttiswhitebalance
 *
 * The tiswhitebalance element analyzes the color temperatures of the incomming buffers and applies a whitebalancing.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v fakesrc ! tiswhitebalance ! bayer ! fakesink
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include "gsttiswhitebalance.h"
#include "image_sampling.h"

GST_DEBUG_CATEGORY_STATIC (gst_tiswhitebalance_debug_category);
#define GST_CAT_DEFAULT gst_tiswhitebalance_debug_category

/* prototypes */

static void gst_tiswhitebalance_set_property (GObject* object,
                                              guint property_id,
                                              const GValue* value,
                                              GParamSpec* pspec);
static void gst_tiswhitebalance_get_property (GObject* object,
                                              guint property_id,
                                              GValue* value,
                                              GParamSpec* pspec);
static void gst_tiswhitebalance_finalize (GObject* object);

static GstFlowReturn gst_tiswhitebalance_transform_ip (GstBaseTransform* trans, GstBuffer* buf);
static GstCaps* gst_tiswhitebalance_transform_caps (GstBaseTransform* trans,
                                                    GstPadDirection direction,
                                                    GstCaps* caps);

static void gst_tiswhitebalance_fixate_caps (GstBaseTransform* base,
                                             GstPadDirection direction,
                                             GstCaps* caps,
                                             GstCaps* othercaps);

enum
{
    PROP_0,
    PROP_GAIN_RED,
    PROP_GAIN_GREEN,
    PROP_GAIN_BLUE,
    PROP_AUTO_ENABLED,
    PROP_WHITEBALANCE_ENABLED,
    PROP_CAMERA_WB,
};

/* pad templates */

static GstStaticPadTemplate gst_tiswhitebalance_sink_template =
    GST_STATIC_PAD_TEMPLATE ("sink",
                             GST_PAD_SINK,
                             GST_PAD_ALWAYS,
                             GST_STATIC_CAPS ("video/x-raw-bayer,format=(string){bggr,grbg,gbrg,rggb},framerate=(fraction)[0/1,MAX],width=[1,MAX],height=[1,MAX]")
        );

static GstStaticPadTemplate gst_tiswhitebalance_src_template =
    GST_STATIC_PAD_TEMPLATE ("src",
                             GST_PAD_SRC,
                             GST_PAD_ALWAYS,
                             GST_STATIC_CAPS ("video/x-raw-bayer,format=(string){bggr,grbg,gbrg,rggb},framerate=(fraction)[0/1,MAX],width=[1,MAX],height=[1,MAX]")
        );


/* class initialization */

G_DEFINE_TYPE_WITH_CODE (GstTisWhiteBalance,
                         gst_tiswhitebalance,
                         GST_TYPE_BASE_TRANSFORM,
                         GST_DEBUG_CATEGORY_INIT (gst_tiswhitebalance_debug_category,
                                                  "tiswhitebalance", 0,
                                                  "debug category for tiswhitebalance element"));


static gboolean gst_tiswhitebalance_device_set_whiteblance (GstTisWhiteBalance* self)
{
    gst_debug_log (gst_tiswhitebalance_debug_category,
                   GST_LEVEL_INFO,
                   "tiswhitebalance",
                   "",
                   __LINE__,
                   NULL,
                   "Applying white balance to device with values: R:%d G:%d B:%d", self->res.color.rgb.R, self->res.color.rgb.G,
                   self->res.color.rgb.B);

    return device_set_rgb(&self->res);
}


static void gst_tiswhitebalance_class_init (GstTisWhiteBalanceClass * klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GstBaseTransformClass *base_transform_class = GST_BASE_TRANSFORM_CLASS (klass);

    gst_element_class_add_static_pad_template (GST_ELEMENT_CLASS(klass),
                                               &gst_tiswhitebalance_src_template);
    gst_element_class_add_static_pad_template (GST_ELEMENT_CLASS(klass),
                                               &gst_tiswhitebalance_sink_template);

    gst_element_class_set_details_simple (GST_ELEMENT_CLASS(klass),
                                          "The Imaging Source White Balance Element",
                                          "Generic",
                                          "Adjusts white balancing of video data buffers",
                                          "Edgar Thier <edgarthier@gmail.com>");

    gobject_class->set_property = gst_tiswhitebalance_set_property;
    gobject_class->get_property = gst_tiswhitebalance_get_property;
    gobject_class->finalize = gst_tiswhitebalance_finalize;
    base_transform_class->transform_ip = GST_DEBUG_FUNCPTR (gst_tiswhitebalance_transform_ip);
    base_transform_class->transform_caps = GST_DEBUG_FUNCPTR (gst_tiswhitebalance_transform_caps);
    base_transform_class->fixate_caps = GST_DEBUG_FUNCPTR (gst_tiswhitebalance_fixate_caps);

    g_object_class_install_property (gobject_class,
                                     PROP_GAIN_RED,
                                     g_param_spec_int ("red", "Red",
                                                       "Value for red",
                                                       0, 255, 0,
                                                       G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    g_object_class_install_property (gobject_class,
                                     PROP_GAIN_GREEN,
                                     g_param_spec_int ("green", "Green Gain",
                                                       "Value for red gain",
                                                       0, 255, 0,
                                                       G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    g_object_class_install_property (gobject_class,
                                     PROP_GAIN_BLUE,
                                     g_param_spec_int ("blue", "Blue Gain",
                                                       "Value for blue gain",
                                                       0, 255, 0,
                                                       G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    g_object_class_install_property (gobject_class,
                                     PROP_AUTO_ENABLED,
                                     g_param_spec_boolean ("auto", "Auto Value Adjustment",
                                                           "Automatically adjust white balance values",
                                                           TRUE,
                                                           G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    g_object_class_install_property (gobject_class,
                                     PROP_CAMERA_WB,
                                     g_param_spec_boolean ("camera-whitebalance", "Device whitebalance settings",
                                                           "Adjust white balance values in the camera",
                                                           FALSE,
                                                           G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    g_object_class_install_property (gobject_class,
                                     PROP_WHITEBALANCE_ENABLED,
                                     g_param_spec_boolean ("module-enabled", "Enable/Disable White Balance Module",
                                                           "Disable entire module",
                                                           TRUE,
                                                           G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}


static void init_wb_values (GstTisWhiteBalance *self)
{
    self->rgb = (rgb_tripel){WB_IDENTITY, WB_IDENTITY, WB_IDENTITY};
    self->red = WB_IDENTITY;
    self->green = WB_IDENTITY;
    self->blue = WB_IDENTITY;
}


static void gst_tiswhitebalance_init (GstTisWhiteBalance *self)
{
	init_wb_values(self);
    self->auto_wb = TRUE;
    self->res.source_element = NULL;
}


void gst_tiswhitebalance_set_property (GObject* object,
                                       guint property_id,
                                       const GValue* value,
                                       GParamSpec* pspec)
{
    GstTisWhiteBalance *tiswhitebalance = GST_TISWHITEBALANCE (object);

    switch (property_id)
    {
        case PROP_GAIN_RED:
            tiswhitebalance->red = g_value_get_int (value);
            break;
        case PROP_GAIN_GREEN:
            tiswhitebalance->green = g_value_get_int (value);
            break;
        case PROP_GAIN_BLUE:
            tiswhitebalance->blue = g_value_get_int (value);
            break;
        case PROP_AUTO_ENABLED:
            tiswhitebalance->auto_wb = g_value_get_boolean (value);
            break;
        case PROP_CAMERA_WB:
            tiswhitebalance->force_hardware_wb = g_value_get_boolean (value);
            tiswhitebalance->res.color.has_whitebalance = g_value_get_boolean (value);
            break;
        case PROP_WHITEBALANCE_ENABLED:
            tiswhitebalance->auto_enabled = g_value_get_boolean (value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}


void gst_tiswhitebalance_get_property (GObject* object,
                                       guint property_id,
                                       GValue* value,
                                       GParamSpec* pspec)
{
    GstTisWhiteBalance *tiswhitebalance = GST_TISWHITEBALANCE (object);

    switch (property_id)
    {
        case PROP_GAIN_RED:
            g_value_set_int (value, tiswhitebalance->red);
            break;
        case PROP_GAIN_GREEN:
            g_value_set_int (value, tiswhitebalance->green);
            break;
        case PROP_GAIN_BLUE:
            g_value_set_int (value, tiswhitebalance->blue);
            break;
        case PROP_AUTO_ENABLED:
            g_value_set_boolean (value, tiswhitebalance->auto_wb);
            break;
        case PROP_CAMERA_WB:
            g_value_set_boolean (value, tiswhitebalance->force_hardware_wb);
            /* g_value_set_boolean (value, tiswhitebalance->force_hardware_wb); */
            break;
        case PROP_WHITEBALANCE_ENABLED:
            g_value_set_boolean (value, tiswhitebalance->auto_enabled);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}


void gst_tiswhitebalance_finalize (GObject* object)
{
    G_OBJECT_CLASS (gst_tiswhitebalance_parent_class)->finalize (object);
}


static GstCaps* gst_tiswhitebalance_transform_caps (GstBaseTransform* trans,
                                                    GstPadDirection direction,
                                                    GstCaps* caps)
{
    GstCaps *outcaps = gst_caps_copy (caps);

    return outcaps;
}


static void gst_tiswhitebalance_fixate_caps (GstBaseTransform* base,
                                             GstPadDirection direction,
                                             GstCaps* incoming,
                                             GstCaps* outgoing)
{
    GstTisWhiteBalance* self = GST_TISWHITEBALANCE(base);

    GstStructure* ins;
    GstStructure* outs;
    gint width, height;
    g_return_if_fail (gst_caps_is_fixed (incoming));

    GST_DEBUG_OBJECT (base, "trying to fixate outgoing caps %" GST_PTR_FORMAT
                      " based on caps %" GST_PTR_FORMAT, outgoing, incoming);

    ins = gst_caps_get_structure (incoming, 0);
    outs = gst_caps_get_structure (outgoing, 0);

    if (gst_structure_get_int (ins, "width", &width))
    {
        if (gst_structure_has_field (outs, "width"))
        {
            gst_structure_fixate_field_nearest_int (outs, "width", width);
        }
        self->width = width;
    }

    if (gst_structure_get_int (ins, "height", &height))
    {
        if (gst_structure_has_field (outs, "height"))
        {
            gst_structure_fixate_field_nearest_int (outs, "height", height);
        }
        self->height = height;
    }

    const char* p = gst_structure_get_name (ins);
    guint fourcc;
    if (g_strcmp0(p, "video/x-raw-bayer") == 0)
    {
        if (gst_structure_get_field_type (ins, "format") == G_TYPE_STRING)
        {
            const char *string;
            string = gst_structure_get_string (ins, "format");
            fourcc = GST_STR_FOURCC (string);
        }
        else if (gst_structure_get_field_type (ins, "format") == GST_TYPE_FOURCC)
        {
            gst_structure_get_fourcc (ins, "format", &fourcc);
        }
        else
            fourcc = 0;

        if (fourcc == 0)
        {
            gst_debug_log (gst_tiswhitebalance_debug_category,
                           GST_LEVEL_ERROR,
                           "gst_tiswhitebalance",
                           "gst_tiswhitebalance_fixate_caps",
                           0,
                           NULL,
                           "Unable to determine video format.");
            return;
        }

        if (fourcc == MAKE_FOURCC ('g','r','b','g'))
        {
            self->pattern = GR;
        }
        else if (fourcc == MAKE_FOURCC ('r', 'g', 'g', 'b'))
        {
            self->pattern = RG;
        }
        else if (fourcc == MAKE_FOURCC ('g', 'b', 'r', 'g'))
        {
            self->pattern = GB;
        }
        else if (fourcc == MAKE_FOURCC ('b', 'g', 'g', 'r'))
        {
            self->pattern = BG;
        }
        else
        {
            gst_debug_log (gst_tiswhitebalance_debug_category,
                           GST_LEVEL_ERROR,
                           "gst_tiswhitebalance",
                           "gst_tiswhitebalance_fixate_caps",
                           0,
                           NULL,
                           "Unable to determine bayer pattern.");
            return;
        }

        gst_debug_log (gst_tiswhitebalance_debug_category,
                       GST_LEVEL_INFO,
                       "gst_tiswhitebalance",
                       "gst_tiswhitebalance_fixate_caps",
                       0,
                       NULL,
                       "Using bayer format %s for whitebalancing.", bayer_to_string(self->pattern));

    }
    else
    {
        gst_debug_log (gst_tiswhitebalance_debug_category,
                       GST_LEVEL_INFO,
                       "gst_tiswhitebalance",
                       "gst_tiswhitebalance_fixate_caps",
                       0,
                       NULL,
                       "Not a bayer format. White balance will be disabled.");
    }

}


static guint clip (guint x, guint max)
{
    if ( x > max )
        return max;
    return x;
}


guint calc_brightness_from_clr_avg (guint r, guint g, guint b)
{
    return (r * r_factor + g * g_factor + b * b_factor) >> 8;
}


gboolean is_near_gray (guint r, guint g, guint b)
{
    guint brightness = calc_brightness_from_clr_avg( r, g, b );
    if ( brightness < NEARGRAY_MIN_BRIGHTNESS ) return FALSE;
    if ( brightness > NEARGRAY_MAX_BRIGHTNESS ) return FALSE;

    guint deltaR = abs( (gint)r - (gint)brightness );
    guint deltaG = abs( (gint)g - (gint)brightness );
    guint deltaB = abs( (gint)b - (gint)brightness );

    float devR = deltaR / (float)brightness;
    float devG = deltaG / (float)brightness;
    float devB = deltaB / (float)brightness;

    return ((devR < NEARGRAY_MAX_COLOR_DEVIATION) &&
            (devG < NEARGRAY_MAX_COLOR_DEVIATION) &&
            (devB < NEARGRAY_MAX_COLOR_DEVIATION));
}


rgb_tripel simulate_whitebalance (const auto_sample_points* data, const rgb_tripel* wb, gboolean enable_near_gray)
{
    rgb_tripel result = { 0, 0, 0 };
    rgb_tripel result_near_gray = { 0, 0, 0 };
    unsigned int count_near_gray = 0;

    guint i;
    for (i = 0; i < data->cnt; ++i)
    {
        unsigned int r = clip( data->samples[i].r * wb->R / WB_IDENTITY, WB_MAX );
        unsigned int g = clip( data->samples[i].g * wb->G / WB_IDENTITY, WB_MAX );
        unsigned int b = clip( data->samples[i].b * wb->B / WB_IDENTITY, WB_MAX );

        result.R += r;
        result.G += g;
        result.B += b;

        if ( is_near_gray( r, g, b ) )
        {
            result_near_gray.R += r;
            result_near_gray.G += g;
            result_near_gray.B += b;
            count_near_gray += 1;
        }
    }

    float near_gray_amount = count_near_gray / (float)data->cnt;

    if ((near_gray_amount < NEARGRAY_REQUIRED_AMOUNT) || !enable_near_gray)
    {
        result.R /= data->cnt;
        result.G /= data->cnt;
        result.B /= data->cnt;
        return result;
    }
    else
    {
        result_near_gray.R /= count_near_gray;
        result_near_gray.G /= count_near_gray;
        result_near_gray.B /= count_near_gray;
        return result_near_gray;
    }
}


rgb_tripel average_color_cam (const auto_sample_points* data )
{
    rgb_tripel result = { 0, 0, 0 };

	guint i;
    for (i = 0; i < data->cnt; ++i)
    {
        unsigned int r =  data->samples[i].r ;
        unsigned int g = data->samples[i].g ;
        unsigned int b = data->samples[i].b ;

        result.R += r;
        result.G += g;
        result.B += b;
    }


	result.R /= data->cnt;
	result.G /= data->cnt;
	result.B /= data->cnt;
	return result;
}



gboolean wb_auto_step (rgb_tripel* clr, rgb_tripel* wb )
{
    unsigned int avg = ((clr->R + clr->G + clr->B) / 3);
    int dr = (int)avg - clr->R;
    int dg = (int)avg - clr->G;
    int db = (int)avg - clr->B;

    if (abs(dr) < BREAK_DIFF && abs(dg) < BREAK_DIFF && abs(db) < BREAK_DIFF)
    {
        wb->R = clip( wb->R, WB_MAX );
        wb->G = clip( wb->G, WB_MAX );
        wb->B = clip( wb->B, WB_MAX );

        return TRUE;
    }

    if ((clr->R > avg) && (wb->R > WB_IDENTITY))
    {
        wb->R -= 1;
    }

    if ((clr->G > avg) && (wb->G > WB_IDENTITY))
    {
        wb->G -= 1;
    }

    if ((clr->B > avg) && (wb->B > WB_IDENTITY))
    {
        wb->B -= 1;
    }

    if ((clr->R < avg) && (wb->R < WB_MAX))
    {
        wb->R += 1;
    }

    if ((clr->G < avg) && (wb->G < WB_MAX))
    {
        wb->G += 1;
    }

    if ((clr->B < avg) && (wb->B < WB_MAX))
    {
        wb->B += 1;
    }

    if ((wb->R > WB_IDENTITY) && (wb->G > WB_IDENTITY) && (wb->B > WB_IDENTITY))
    {
        wb->R -= 1;
        wb->G -= 1;
        wb->B -= 1;
    }

    return FALSE;
}


gboolean auto_whitebalance (const auto_sample_points* data, rgb_tripel* wb, guint* resulting_brightness)
{
    rgb_tripel old_wb = *wb;
    if (wb->R < WB_IDENTITY)
        wb->R = WB_IDENTITY;
    if (wb->G < WB_IDENTITY)
        wb->G = WB_IDENTITY;
    if (wb->B < WB_IDENTITY)
        wb->B = WB_IDENTITY;
    if (old_wb.R != wb->R || old_wb.G != wb->G || old_wb.B != wb->B)
        return FALSE;

    while ((wb->R > WB_IDENTITY) && (wb->G > WB_IDENTITY) && (wb->B > WB_IDENTITY))
    {
        wb->R -= 1;
        wb->G -= 1;
        wb->B -= 1;
    }

    unsigned int steps = 0;
    while (steps++ < MAX_STEPS)
    {
        rgb_tripel tmp = simulate_whitebalance( data, wb, TRUE );

        // Simulate white balance once more, this time always on the whole image
        rgb_tripel tmp2 = simulate_whitebalance( data, wb, FALSE );
        *resulting_brightness = calc_brightness_from_clr_avg( tmp2.R, tmp2.G, tmp2.B );

        if (wb_auto_step(&tmp, wb))
        {
            return TRUE;
        }
    }
    wb->R = clip( wb->R, WB_MAX );
    wb->G = clip( wb->G, WB_MAX );
    wb->B = clip( wb->B, WB_MAX );

    return FALSE;
}

gboolean auto_whitebalance_cam (const auto_sample_points* data, rgb_tripel* wb )
{
    rgb_tripel old_wb = *wb;

    if (wb->R < WB_IDENTITY)
        wb->R = WB_IDENTITY;
    if (wb->G < WB_IDENTITY)
        wb->G = WB_IDENTITY;
    if (wb->B < WB_IDENTITY)
        wb->B = WB_IDENTITY;
    if (old_wb.R != wb->R || old_wb.G != wb->G || old_wb.B != wb->B)
        return FALSE;

    while ((wb->R > WB_IDENTITY) && (wb->G > WB_IDENTITY) && (wb->B > WB_IDENTITY))
    {
        wb->R -= 1;
        wb->G -= 1;
        wb->B -= 1;
    }

   rgb_tripel averageColor = average_color_cam( data);
   if(wb_auto_step(&averageColor, wb ) )
   {
	   return TRUE;
   }


    wb->R = clip( wb->R, WB_MAX );
    wb->G = clip( wb->G, WB_MAX );
    wb->B = clip( wb->B, WB_MAX );

    gst_debug_log (gst_tiswhitebalance_debug_category,
                   GST_LEVEL_INFO,
                   "tiswhitebalance",
                   "",
                   __LINE__,
                   NULL,
                   "Calculated white balance R:%d G:%d B:%d", wb->R, wb->G, wb->B);

    return FALSE;
}




byte wb_pixel_c (byte pixel, byte wb_r, byte wb_g, byte wb_b, tBY8Pattern pattern)
{
    unsigned int val = pixel;
	int divisor = 64;
    switch (pattern)
    {
        case BG:
            val = (val * wb_b) / divisor;
            break;
        case GB:
            val = (val * wb_g) / divisor;
            break;
        case GR:
            val = (val * wb_g) / divisor;
            break;
        case RG:
            val = (val * wb_r) / divisor;
            break;
    };
    return ( val > 0xFF ? 0xFF : (byte)(val));
}


static void wb_line_c (byte* dest_line,
                       byte* src_line,
                       unsigned int dim_x,
                       byte wb_r, byte wb_g, byte wb_b,
                       tBY8Pattern pattern)
{
    const tBY8Pattern even_pattern = pattern;
    const tBY8Pattern odd_pattern = next_pixel(pattern);
    guint x;
    for (x = 0; x < dim_x; x += 2)
    {
        unsigned int v0 = wb_pixel_c( src_line[x], wb_r, wb_g, wb_b,even_pattern );
        unsigned int v1 = wb_pixel_c( src_line[x+1], wb_r, wb_g, wb_b, odd_pattern );
        *((guint16*)(dest_line + x)) = (guint16)(v1 << 8 | v0);
    }

    if (x == (dim_x - 1))
    {
        dest_line[x] = wb_pixel_c( src_line[x], wb_r, wb_g, wb_b, even_pattern );
    }
}


static void	wb_image_c (GstTisWhiteBalance* self, GstBuffer* buf, byte wb_r, byte wb_g, byte wb_b)
{
    guint8 *data = (guint8*)GST_BUFFER_DATA (buf);

    unsigned int dim_x = self->width;
    unsigned int dim_y = self->height;

    guint pitch = 8 * dim_x / 8;

    tBY8Pattern odd = next_line(self->pattern);

    guint y;
    for (y = 0 ; y < (dim_y - 1); y += 2)
    {
        byte* line0 = data + y * pitch;
        byte* line1 = data + (y + 1) * pitch;

        wb_line_c(line0, line0, dim_x, wb_r, wb_g, wb_b, self->pattern);
        wb_line_c(line1, line1, dim_x, wb_r, wb_g, wb_b, odd);
    }

    if (y == (dim_y - 1))
    {
        byte* line = data + y * pitch;
        wb_line_c(line, line, dim_x, wb_r, wb_g, wb_b, self->pattern);
    }
}


void apply_wb_by8_c ( GstTisWhiteBalance* self, GstBuffer* buf, byte wb_r, byte wb_g, byte wb_b)
{
    gst_debug_log (gst_tiswhitebalance_debug_category,
                   GST_LEVEL_INFO,
                   "tiswhitebalance",
                   "",
                   850,
                   NULL,
                   "Applying white balance with values: R:%d G:%d B:%d", wb_r, wb_g, wb_b);

    wb_image_c( self, buf, wb_r, wb_g, wb_b);
}


static void whitebalance_buffer (GstTisWhiteBalance* self, GstBuffer* buf)
{

    auto_sample_points points;

    if (self->res.color.has_whitebalance)
    {
        self->rgb = self->res.color.rgb;
    }

    rgb_tripel rgb = self->rgb;

    get_sampling_points (buf, &points, self->pattern);

    guint resulting_brightness = 0;

	if (self->res.color.has_whitebalance)
    {
		auto_whitebalance_cam(&points, &rgb);
    }
	else
    {
		auto_whitebalance(&points, &rgb, &resulting_brightness);
    }

    /* we prefer to set our own values */
    if (self->auto_wb == FALSE)
    {
        rgb.R = self->red;
        rgb.G = self->green;
        rgb.B = self->blue;
    }
    else /* update the permanent values to represent the current adjustments */
    {
        self->red = rgb.R;
        self->green = rgb.G;
        self->blue = rgb.B;

        self->rgb = rgb;
    }

    if (self->res.color.has_whitebalance)
    {
        self->res.color.rgb = rgb;
        gst_tiswhitebalance_device_set_whiteblance(self);
    }
    else
    {
        apply_wb_by8_c(self, buf, rgb.R, rgb.G, rgb.B);
    }
}


/* Entry point */
static GstFlowReturn gst_tiswhitebalance_transform_ip (GstBaseTransform* trans, GstBuffer* buf)
{
    GstTisWhiteBalance* self = GST_TISWHITEBALANCE (trans);

    if (self->res.source_element == NULL)
    {
        gst_debug_log (gst_tiswhitebalance_debug_category,
                       GST_LEVEL_INFO,
                       "gst_tiswhitebalance",
                       "gst_tiswhitebalance_fixate_caps",
                       __LINE__,
                       NULL,
                       "Searching for source");

        self->res = find_source(GST_ELEMENT(self));

        if (self->force_hardware_wb)
        {
            self->res.color.has_whitebalance = TRUE;
        }

		if (self->res.color.has_whitebalance)
		{
			WB_MAX = self->res.color.max;
			WB_IDENTITY = self->res.color.default_value;

			init_wb_values(self);
		}
    }

    /* auto is completely disabled */
    if (!self->auto_enabled)
    {
        return GST_FLOW_OK;
    }

    whitebalance_buffer(self, buf);

    return GST_FLOW_OK;
}


static gboolean plugin_init (GstPlugin* plugin)
{
    return gst_element_register (plugin, "tiswhitebalance", GST_RANK_NONE, GST_TYPE_TISWHITEBALANCE);
}

#ifndef VERSION
#define VERSION "0.0.1"
#endif
#ifndef PACKAGE
#define PACKAGE "tiswhitebalance"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "tiswhitebalance"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "https://github.com/TheImagingSource/tiscamera"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
                   GST_VERSION_MINOR,
                   "tiswhitebalance",
                   "The Imaging Source white balance plugin",
                   plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)
