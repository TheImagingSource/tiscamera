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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include <stdbool.h>
#include <ctype.h>
#include <chrono>
#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include "gsttcamautofocus.h"

#include "tcam.h"
#include "tcamgstbase.h"
#include "tcamprop.h"

#include "tcamprop_impl_helper.h"

GST_DEBUG_CATEGORY_STATIC (gst_tcamautofocus_debug_category);
#define GST_CAT_DEFAULT gst_tcamautofocus_debug_category

static const int REGION_MIN_SIZE = 128;

enum
{
    PROP_0,
    PROP_AUTO,
    PROP_LEFT,
    PROP_TOP,
    PROP_WIDTH,
    PROP_HEIGHT,
};



/* prototypes */

static GstStateChangeReturn gst_tcamautofocus_change_state (GstElement* element,
                                                            GstStateChange trans);

static void gst_tcamautofocus_set_property (GObject* object,
                                            guint property_id,
                                            const GValue* value,
                                            GParamSpec* pspec);
static void gst_tcamautofocus_get_property (GObject* object,
                                            guint property_id,
                                            GValue* value,
                                            GParamSpec* pspec);
static void gst_tcamautofocus_finalize (GObject* object);

static GstFlowReturn gst_tcamautofocus_transform_ip (GstBaseTransform* trans,
                                                     GstBuffer* buf);


/* tcamprop interface*/

static GSList* gst_tcamautofocus_get_property_names (TcamProp* self);

static gchar* gst_tcamautofocus_get_property_type (TcamProp* self,
                                                   const gchar* name);

static gboolean gst_tcamautofocus_get_tcam_property (TcamProp* self,
                                                     const gchar* name,
                                                     GValue* value,
                                                     GValue* min,
                                                     GValue* max,
                                                     GValue* def,
                                                     GValue* step,
                                                     GValue* type,
                                                     GValue* flags,
                                                     GValue* category,
                                                     GValue* group);

static gboolean gst_tcamautofocus_set_tcam_property (TcamProp* self,
                                                     const gchar* name,
                                                     const GValue* value);

static GSList* gst_tcamautofocus_get_tcam_menu_entries (TcamProp* self,
                                                        const gchar* name);

static GSList* gst_tcamautofocus_get_device_serials (TcamProp* self);

static gboolean gst_tcamautofocus_get_device_info (TcamProp* self,
                                                   const char* serial,
                                                   char** name,
                                                   char** identifier,
                                                   char** connection_type);

static void gst_tcamautofocus_prop_init (TcamPropInterface* iface)
{
    iface->get_tcam_property_names = gst_tcamautofocus_get_property_names;
    iface->get_tcam_property_type = gst_tcamautofocus_get_property_type;
    iface->get_tcam_property = gst_tcamautofocus_get_tcam_property;
    iface->get_tcam_menu_entries = gst_tcamautofocus_get_tcam_menu_entries;
    iface->set_tcam_property = gst_tcamautofocus_set_tcam_property;
    iface->get_tcam_device_serials = gst_tcamautofocus_get_device_serials;
    iface->get_tcam_device_info = gst_tcamautofocus_get_device_info;
}



G_DEFINE_TYPE_WITH_CODE (GstTcamAutoFocus,
                         gst_tcamautofocus,
                         GST_TYPE_BASE_TRANSFORM,
                         G_IMPLEMENT_INTERFACE(TCAM_TYPE_PROP, gst_tcamautofocus_prop_init))




using namespace tcamprop_impl_helper;

static const prop_entry tcamautofocus_property_list[] =
{
    { PROP_AUTO,    "Focus Auto",       prop_types::button, "Lens", "Focus" },
    { PROP_LEFT,    "Focus ROI Left",   prop_types::integer, "Lens", "Focus" },
    { PROP_TOP,     "Focus ROI Top",    prop_types::integer, "Lens", "Focus" },
    { PROP_WIDTH,   "Focus ROI Width",  prop_types::integer, "Lens", "Focus" },
    { PROP_HEIGHT,  "Focus ROI Height", prop_types::integer, "Lens", "Focus" },
};



static const prop_entry* find_property_entry( guint id )
{
    for( const auto& e : tcamautofocus_property_list ) {
        if( e.prop_id == id ) {
            return &e;
        }
    }
    return nullptr;
}

static const prop_entry* find_property_entry( const char* str )
{
    for( const auto& e : tcamautofocus_property_list ) {
        if( g_strcmp0( e.prop_name, str ) == 0 ) {
            return &e;
        }
    }
    return nullptr;
}

static GSList* gst_tcamautofocus_get_property_names (TcamProp* self __attribute__((unused)))
{
    GSList* names = nullptr;

    names = g_slist_append(names, g_strdup( find_property_entry(PROP_AUTO)->prop_name));
    names = g_slist_append(names, g_strdup( find_property_entry(PROP_LEFT)->prop_name ));
    names = g_slist_append(names, g_strdup( find_property_entry(PROP_TOP)->prop_name ));
    names = g_slist_append(names, g_strdup( find_property_entry(PROP_WIDTH)->prop_name ));
    names = g_slist_append(names, g_strdup( find_property_entry(PROP_HEIGHT)->prop_name ));

    return names;
}

static gchar* gst_tcamautofocus_get_property_type( TcamProp* self __attribute__( (unused) ), const gchar* name )
{
    if( name == nullptr )
    {
        GST_ERROR( "Name is empty" );
        return nullptr;
    }

    const auto* entry = find_property_entry( name );
    if( entry == nullptr ) {
        return nullptr;
    }

    return strdup( to_string( entry->type ) );
}

static gboolean gst_tcamautofocus_get_tcam_property (TcamProp* prop,
                                                     const gchar* name,
                                                     GValue* value,
                                                     GValue* min,
                                                     GValue* max,
                                                     GValue* def,
                                                     GValue* step,
                                                     GValue* type,
                                                     GValue* flags,
                                                     GValue* category,
                                                     GValue* group)
{
    GstTcamAutoFocus* self = GST_TCAMAUTOFOCUS(prop);


    const auto* entry = find_property_entry( name );
    if( !entry ) {
        return false;
    }

    fill_gvalue( type, entry->type );
    fill_int( flags, 0 );
    fill_string( category, entry->category );
    fill_string( group, entry->group );

    if ( entry->prop_id == PROP_AUTO )
    {
        fill_bool( value, self->focus_active );
        fill_bool( min, false );
        fill_bool( max, true );
        fill_bool( def, true );
        fill_bool( step, true );
        return TRUE;
    }
    else if ( entry->prop_id == PROP_LEFT )
    {
        fill_int( value, self->roi_left );
        fill_int( min, 0 );
        fill_int( max, self->image_width - REGION_MIN_SIZE );
        fill_int( def, 0 );
        fill_int( step, 1 );
        return TRUE;
    }
    else if ( entry->prop_id == PROP_TOP)
    {
        fill_int( value, self->roi_top );
        fill_int( min, 0 );
        fill_int( max, self->image_height - REGION_MIN_SIZE );
        fill_int( def, 0 );
        fill_int( step, 1 );
        return TRUE;
    }
    else if ( entry->prop_id == PROP_WIDTH )
    {
        fill_int( value, self->roi_width );
        fill_int( min, REGION_MIN_SIZE );
        fill_int( max, self->image_width );
        fill_int( def, self->image_width );
        fill_int( step, 1 );
        return TRUE;
    }
    else if ( entry->prop_id ==  PROP_HEIGHT )
    {
        fill_int( value, self->roi_height );
        fill_int( min, REGION_MIN_SIZE );
        fill_int( max, self->image_height );
        fill_int( def, self->image_height );
        fill_int( step, 1 );
        return TRUE;
    }

    return FALSE;
}


static gboolean gst_tcamautofocus_set_tcam_property (TcamProp* self,
                                                     const gchar* name,
                                                     const GValue* value)
{
    auto entry = find_property_entry(name);

    if ( entry == nullptr )
    {
        // GST_WARNING("Unknown id for name '%s'", name);
        return FALSE;
    }

    gst_tcamautofocus_set_property(G_OBJECT(self),
                                   entry->prop_id,
                                   value, NULL);
    return TRUE;
}


static GSList* gst_tcamautofocus_get_tcam_menu_entries (TcamProp* self __attribute__((unused)),
                                                        const gchar* name __attribute__((unused)))
{
    GSList* ret = nullptr;

    return ret;
}


static GSList* gst_tcamautofocus_get_device_serials (TcamProp* self __attribute__((unused)))
{
    return FALSE;
}


static gboolean gst_tcamautofocus_get_device_info (TcamProp* self __attribute__((unused)),
                                                   const char* serial __attribute__((unused)),
                                                   char** name __attribute__((unused)),
                                                   char** identifier __attribute__((unused)),
                                                   char** connection_type __attribute__((unused)))
{
    return FALSE;
}


static const char* CAPS_STR = "video/x-raw,format={GRAY8,GRAY16_LE};"
    "video/x-bayer,format={rggb,bggr,grbg,gbrg}";


static GstStaticPadTemplate gst_tcamautofocus_sink_template =
    GST_STATIC_PAD_TEMPLATE("sink",
                            GST_PAD_SINK,
                            GST_PAD_ALWAYS,
                            GST_STATIC_CAPS(CAPS_STR));

static GstStaticPadTemplate gst_tcamautofocus_src_template =
    GST_STATIC_PAD_TEMPLATE("src",
                            GST_PAD_SRC,
                            GST_PAD_ALWAYS,
                            GST_STATIC_CAPS(CAPS_STR));


static void gst_tcamautofocus_class_init (GstTcamAutoFocusClass* klass)
{
    GObjectClass* gobject_class = G_OBJECT_CLASS (klass);
    GstBaseTransformClass* base_transform_class = GST_BASE_TRANSFORM_CLASS (klass);

    gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
                                        gst_static_pad_template_get(&gst_tcamautofocus_src_template));
    gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
                                        gst_static_pad_template_get(&gst_tcamautofocus_sink_template));

    gst_element_class_set_details_simple (GST_ELEMENT_CLASS(klass),
                                          "The Imaging Source auto focus Element",
                                          "Generic",
                                          "Adjusts the image focus by setting camera properties.",
                                          "The Imaging Source Europe GmbH <support@theimagingsource.com>");

    GST_ELEMENT_CLASS(klass)->change_state = gst_tcamautofocus_change_state;
    gobject_class->set_property = gst_tcamautofocus_set_property;
    gobject_class->get_property = gst_tcamautofocus_get_property;
    gobject_class->finalize = gst_tcamautofocus_finalize;
    base_transform_class->transform_ip = GST_DEBUG_FUNCPTR (gst_tcamautofocus_transform_ip);

    g_object_class_install_property(gobject_class,
                                    PROP_AUTO,
                                    g_param_spec_boolean("auto",
                                                         "Activate auto focus run",
                                                         "Automatically adjust camera focus",
                                                         FALSE,
                                                         static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));
    g_object_class_install_property(gobject_class,
                                    PROP_LEFT,
                                    g_param_spec_int("left",
                                                     "Left border of the focus region",
                                                     "Coordinate for focus region.",
                                                     0, G_MAXINT, 0,
                                                     static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));
    g_object_class_install_property(gobject_class,
                                    PROP_TOP,
                                    g_param_spec_int("top",
                                                     "Top border of the focus region",
                                                     "Coordinate for focus region.",
                                                     0, G_MAXINT, 0,
                                                     static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));
    g_object_class_install_property(gobject_class,
                                    PROP_WIDTH,
                                    g_param_spec_int("width",
                                                     "Width of focus region",
                                                     "Width of the focus region beginning at 'left'",
                                                     0, G_MAXINT, 0,
                                                     static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));
    g_object_class_install_property(gobject_class,
                                    PROP_HEIGHT,
                                    g_param_spec_int("height",
                                                     "Height of focus region",
                                                     "Height of the focus region beginning at 'top'.",
                                                     0, G_MAXINT, 0,
                                                     static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));


    GST_DEBUG_CATEGORY_INIT (gst_tcamautofocus_debug_category,
                             "tcamautofocus",
                             0,
                             "debug category for tcamautofocus element");

}


static void gst_tcamautofocus_init (GstTcamAutoFocus *self)
{
    self->focus = autofocus_create();
    self->focus_active = FALSE;
    self->cur_focus = 0;
    self->roi_top = 0;
    self->roi_left = 0;
    self->roi_width = 0;
    self->roi_height = 0;
    self->image_width = 0;
    self->image_height = 0;
    self->camera_src = NULL;
    self->init_focus = TRUE;
    tcam_image_size min_size = {REGION_MIN_SIZE, REGION_MIN_SIZE};

    self->roi = create_roi(&min_size, &min_size);
}


static GstStateChangeReturn gst_tcamautofocus_change_state (GstElement* element,
                                                            GstStateChange trans)
{
    GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;

    GstTcamAutoFocus* self = GST_TCAMAUTOFOCUS(element);

    switch (trans)
    {
        case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
        {

            if (self->camera_src == NULL)
            {
                self->camera_src = tcam_gst_find_camera_src(GST_ELEMENT(self));
                if (self->camera_src == nullptr)
                {
                    GST_ERROR("Could not find source element");
                    return GST_STATE_CHANGE_FAILURE;
                }
            }
            break;
        }
        default:
        {
            break;
        }
    }

    gst_element_set_locked_state(element, TRUE);
    ret = GST_ELEMENT_CLASS(gst_tcamautofocus_parent_class)->change_state(element, trans);
    gst_element_set_locked_state(element, FALSE);

    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        return ret;
    }

    switch (trans)
    {
        case GST_STATE_CHANGE_READY_TO_NULL:
        {
            if (self->camera_src)
            {
                gst_object_unref(self->camera_src);
                self->camera_src = NULL;
            }

            break;
        }
        default:
        {
            break;
        }
    }
    return ret;
}


static void gst_tcamautofocus_set_property (GObject* object,
                                            guint property_id,
                                            const GValue* value,
                                            GParamSpec* pspec)
{
    GstTcamAutoFocus* self = GST_TCAMAUTOFOCUS (object);

    switch (property_id)
    {
        case PROP_AUTO:
        {
            self->focus_active = g_value_get_boolean (value);
            if (self->focus_active == TRUE)
            {
                GST_DEBUG("focus_active is now TRUE");
            }
            break;
        }
        case PROP_LEFT:
        {
            self->roi_left = g_value_get_int(value);

            if (self->roi_width > (self->image_width - self->roi_left))
            {
                GST_WARNING("Requested ROI position does not allow the current ROI size. Reducing ROI width.");
                self->roi_width = (self->image_width - self->roi_left);
            }
            roi_set_left(self->roi, self->roi_left);
            break;
        }
        case PROP_TOP:
        {
            self->roi_top = g_value_get_int(value);

            if (self->roi_height > (self->image_height - self->roi_top))
            {
                GST_WARNING("Requested ROI position does not allow the current ROI size. Reducing ROI height.");
                self->roi_height = (self->image_height - self->roi_top);
            }

            roi_set_top(self->roi, self->roi_top);
            break;
        }
        case PROP_WIDTH:
        {
            self->roi_width = g_value_get_int(value);

            if (self->image_width != 0 &&
                self->roi_width > (self->image_width - self->roi_left))
            {
                GST_WARNING("Requested width was larger than resolution and focus region allow. Setting possible maximum.");
                self->roi_width = (self->image_width - self->roi_left);
            }

            roi_set_width(self->roi, self->roi_width);
            break;
        }
        case PROP_HEIGHT:
        {
            self->roi_height = g_value_get_int(value);

            if (self->image_height != 0 &&
                self->roi_height > (self->image_height - self->roi_top))
            {
                GST_WARNING("Requested height was larger than resolution and focus region allow. Setting possible maximum.");
                self->roi_height = (self->image_height - self->roi_top);
            }

            roi_set_height(self->roi, self->roi_height);

            break;
        }
        default:
        {
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
        }
    }
}


static void gst_tcamautofocus_get_property (GObject* object,
                                            guint property_id,
                                            GValue* value,
                                            GParamSpec* pspec)
{
    GstTcamAutoFocus* self = GST_TCAMAUTOFOCUS (object);

    switch (property_id)
    {
        case PROP_AUTO:
            g_value_set_boolean(value, self->focus_active);
            break;
        case PROP_LEFT:
            g_value_set_int(value, roi_left(self->roi));
            break;
        case PROP_TOP:
            g_value_set_int(value, roi_top(self->roi));
            break;
        case PROP_WIDTH:
            g_value_set_int(value, roi_width(self->roi));
            break;
        case PROP_HEIGHT:
            g_value_set_int(value, roi_height(self->roi));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}


static void gst_tcamautofocus_finalize (GObject* object)
{
    GstTcamAutoFocus* self = GST_TCAMAUTOFOCUS(object);

    if (self->camera_src)
    {
        gst_object_unref(self->camera_src);
    }

    autofocus_destroy(self->focus);
    destroy_roi(self->roi);
    self->roi = nullptr;
    G_OBJECT_CLASS(gst_tcamautofocus_parent_class)->finalize(object);
}

static void transform_tcam (GstTcamAutoFocus* self, GstBuffer* buf)
{
    // if (self->camera_src == nullptr)
    // {
    //     self->camera_src = tcam_gst_find_camera_src(GST_ELEMENT(self));
    // }

    if (self->camera_src == nullptr)
    {
        GST_ERROR("Failed to get camera_src");
        return;
    }

    GValue val = G_VALUE_INIT;
    GValue min = G_VALUE_INIT;
    GValue max = G_VALUE_INIT;
    tcam_prop_get_tcam_property(TCAM_PROP(self->camera_src), "Focus", &val,
                                &min, &max, nullptr, nullptr, nullptr,
                                nullptr, nullptr, nullptr);

    GstMapInfo info = {};
    gst_buffer_map(buf, &info, GST_MAP_READ);

    tcam_image_buffer image;
    gst_buffer_to_tcam_image_buffer(buf, nullptr, &image);

    image.format = self->fmt;
    image.pitch = calc_pitch(image.format.fourcc,
                             image.format.width);

    img::img_descriptor img_dsc = img::make_img_desc(image.pData,
                                                     image.format.fourcc,
                                                     {image.format.width,
                                                     image.format.height},
                                                     image.pitch,
                                                     image.length);

    int new_focus_value = g_value_get_int(&val);
    img::point p = {0, 0};

    // pixel dimensions
    // has to adjusted when binning etc is active
    // TODO implement that
    img::dim pixel_dim = {1, 1};

    if (self->init_focus)
    {
        self->params = {};
        self->params.is_run_cmd = true;
        self->params.is_end_cmd = false;

        self->params.run_cmd_params.roi = {
            self->roi_top, // top
            self->roi_left, // left
            self->roi_left + self->roi_width, // right
            self->roi_top + self->roi_height  // bottom
        };

        self->params.run_cmd_params.focus_range_min = g_value_get_int(&min);
        self->params.run_cmd_params.focus_range_max = g_value_get_int(&max);
        self->params.run_cmd_params.auto_step_divisor = 4;
        self->params.run_cmd_params.focus_device_speed = 500;
        self->params.run_cmd_params.suggest_sweep = false;
        self->params.run_cmd_params.focus_min_move_wait_in_ms = 500;
        self->init_focus = FALSE;
    }
    else
    {
        self->params.is_run_cmd = false;
    }

    self->params.device_focus_val = g_value_get_int(&val);

    autofocus_run(self->focus,
                  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count(),
                  img_dsc,
                  self->params,
                  p,
                  pixel_dim,
                  new_focus_value);

    if (autofocus_is_running(self->focus))
    {
        if (self->cur_focus != new_focus_value)
        {
            GST_DEBUG("Setting focus to %d", new_focus_value);

            GValue new_val = G_VALUE_INIT;
            g_value_init(&new_val, G_TYPE_INT);

            g_value_set_int(&new_val, new_focus_value);

            tcam_prop_set_tcam_property(TCAM_PROP(self->camera_src), "Focus", &new_val);

            self->cur_focus = new_focus_value;
        }
    }
    else
    {
        GST_DEBUG("Auto Focus ended with focus value %d", new_focus_value);
        self->focus_active = FALSE;
        self->init_focus = TRUE;
    }

    gst_buffer_unmap(buf, &info);
}


static gboolean find_image_values (GstTcamAutoFocus* self)
{
    GstPad* pad  = GST_BASE_TRANSFORM_SINK_PAD(self);
    GstCaps* caps = gst_pad_get_current_caps(pad);

    if( !gst_caps_to_tcam_video_format(caps, &self->fmt) ) {
        return FALSE;
    }

    self->image_width = self->fmt.width;
    self->image_height = self->fmt.height;

    if (self->roi_width == 0)
    {
        self->roi_width = self->image_width;
    }

    if (self->roi_height == 0)
    {
        self->roi_height = self->image_height;
    }

    roi_set_image_size(self->roi, {(unsigned int)self->image_width,
                                   (unsigned int)self->image_height});

    gst_caps_unref( caps );

    return TRUE;
}


static GstFlowReturn gst_tcamautofocus_transform_ip (GstBaseTransform* trans, GstBuffer* buf)
{
    GstTcamAutoFocus* self = GST_TCAMAUTOFOCUS (trans);

    if (self->image_width == 0 || self->image_height == 0)
    {
        if (!find_image_values(self))
        {
            return GST_FLOW_ERROR;
        }
    }

    if (self->focus_active) // entered if set to true with gst-launch
    {
        find_image_values(self);

        transform_tcam (self, buf);
        return GST_FLOW_OK;
    }

    autofocus_end(self->focus);
    self->focus_active = FALSE;
    return GST_FLOW_OK;
}


static gboolean plugin_init (GstPlugin* plugin)
{
    return gst_element_register(plugin,
                                "tcamautofocus",
                                GST_RANK_NONE,
                                GST_TYPE_TCAMAUTOFOCUS);
}


#ifndef VERSION
#define VERSION "0.0.1"
#endif
#ifndef PACKAGE
#define PACKAGE "tcamautofocus"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME tcamautofocus
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "https://github.com/TheImagingSource/tiscamera"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
                   GST_VERSION_MINOR,
                   PACKAGE_NAME,
                   "The Imaging Source auto exposure plugin",
                   plugin_init,
                   get_version(),
                   "Proprietary",
                   PACKAGE, GST_PACKAGE_ORIGIN)
