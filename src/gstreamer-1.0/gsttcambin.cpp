/*
 * Copyright 2016 The Imaging Source Europe GmbH
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

#include "gsttcambase.h"
#include "gsttcambin.h"
// #include <girepository.h>

#include "tcamprop.h"

#include <unistd.h>

#include <vector>
#include <string>

#define gst_tcambin_parent_class parent_class

GST_DEBUG_CATEGORY_STATIC(gst_tcambin_debug);
#define GST_CAT_DEFAULT gst_tcambin_debug


//
// introspection interface
//



static GSList* gst_tcam_bin_get_property_names (TcamProp* self);

static gchar *gst_tcam_bin_get_property_type (TcamProp* self, gchar* name);

static gboolean gst_tcam_bin_get_tcam_property (TcamProp* self,
                                                gchar* name,
                                                GValue* value,
                                                GValue* min,
                                                GValue* max,
                                                GValue* def,
                                                GValue* step,
                                                GValue* type,
                                                GValue* category,
                                                GValue* group);

static gboolean gst_tcam_bin_set_tcam_property (TcamProp* self,
                                                gchar* name,
                                                const GValue* value);

static GSList* gst_tcam_bin_get_tcam_menu_entries (TcamProp* self,
                                                   const gchar* name);

static GSList* gst_tcam_bin_get_device_serials (TcamProp* self);

static gboolean gst_tcam_bin_get_device_info (TcamProp* self,
                                              const char* serial,
                                              char** name,
                                              char** identifier,
                                              char** connection_type);

static void gst_tcam_bin_prop_init (TcamPropInterface* iface)
{
    iface->get_property_names = gst_tcam_bin_get_property_names;
    iface->get_property_type = gst_tcam_bin_get_property_type;
    iface->get_property = gst_tcam_bin_get_tcam_property;
    iface->get_menu_entries = gst_tcam_bin_get_tcam_menu_entries;
    iface->set_property = gst_tcam_bin_set_tcam_property;
    iface->get_device_serials = gst_tcam_bin_get_device_serials;
    iface->get_device_info = gst_tcam_bin_get_device_info;
}


G_DEFINE_TYPE_WITH_CODE (GstTcamBin, gst_tcambin, GST_TYPE_BIN,
                         G_IMPLEMENT_INTERFACE (TCAM_TYPE_PROP,
                                                gst_tcam_bin_prop_init));


// struct property_type_map
// {
//     enum TCAM_PROPERTY_TYPE typecode;
//     const gchar* type_name;
// };

/**
 * gst_tcam_get_property_type:
 * @self: a #GstTcamBin
 * @name: a #char* identifying the property to query
 *
 * Return the type of a property
 *
 * Returns: (transfer full): A string describing the property type
 */
static gchar* gst_tcam_bin_get_property_type (TcamProp* iface, gchar* name)
{
    gchar* ret = NULL;


    GstTcamBin* self = GST_TCAMBIN (iface);

    if (self->src != NULL)
    {
        ret = tcam_prop_get_tcam_property_type(TCAM_PROP(self->src), name);

        if (ret != nullptr)
        {
            return ret;
        }
    }

    if (self->whitebalance != NULL)
    {
        ret = tcam_prop_get_tcam_property_type(TCAM_PROP(self->whitebalance), name);

        if (ret != nullptr)
        {
            return ret;
        }
    }

    if (self->exposure != NULL)
    {
        ret = tcam_prop_get_tcam_property_type(TCAM_PROP(self->exposure), name);

        if (ret != nullptr)
        {
            return ret;
        }
    }

    return ret;
}


/**
 * gst_tcam_bin_get_property_names:
 * @self: a #GstTcamBin
 *
 * Return a list of property names
 *
 * Returns: (element-type utf8) (transfer full): list of property names
 */
static GSList* gst_tcam_bin_get_property_names (TcamProp* iface)
{
    GSList* ret = NULL;
    GstTcamBin* self = GST_TCAMBIN(iface);

    if (self->src != nullptr)
    {
        GSList* src_prop_names = tcam_prop_get_tcam_property_names(TCAM_PROP(self->src));

        // special case
        // when our src return no properties we have an invalid device and abort everything
        if (src_prop_names == nullptr)
        {
            return nullptr;
        }

        for (unsigned int i = 0; i < g_slist_length(src_prop_names); i++)
        {
            ret = g_slist_append(ret, g_strdup((char*)g_slist_nth(src_prop_names, i)->data));
        }
    }

    if (self->whitebalance != nullptr)
    {
        GSList* wb_prop_names = tcam_prop_get_tcam_property_names(TCAM_PROP(self->whitebalance));

        for (unsigned int i = 0; i < g_slist_length(wb_prop_names); i++)
        {
            ret = g_slist_append(ret, g_strdup((char*)g_slist_nth(wb_prop_names, i)->data));
        }
    }

    if (self->exposure != nullptr)
    {
        GSList* exp_prop_names = tcam_prop_get_tcam_property_names(TCAM_PROP(self->exposure));

        for (unsigned int i = 0; i < g_slist_length(exp_prop_names); i++)
        {
            ret = g_slist_append(ret, g_strdup((char*)g_slist_nth(exp_prop_names, i)->data));
        }
    }

    return ret;
}


static gboolean gst_tcam_bin_get_tcam_property (TcamProp* iface,
                                                gchar* name,
                                                GValue* value,
                                                GValue* min,
                                                GValue* max,
                                                GValue* def,
                                                GValue* step,
                                                GValue* type,
                                                GValue* category,
                                                GValue* group)
{
    GstTcamBin *self = GST_TCAMBIN(iface);

    if (self->src != nullptr)
    {
        if (tcam_prop_get_tcam_property(TCAM_PROP(self->src),
                                        name, value,
                                        min, max,
                                        def, step,
                                        type,
                                        category, group))
        {
            return TRUE;
        }
    }

    if (self->whitebalance != nullptr)
    {
        if (tcam_prop_get_tcam_property(TCAM_PROP(self->whitebalance),
                                        name, value,
                                        min, max,
                                        def, step,
                                        type,
                                        category, group))
        {
            return TRUE;
        }
    }

    if (self->exposure != nullptr)
    {
        if (tcam_prop_get_tcam_property(TCAM_PROP(self->exposure),
                                        name, value,
                                        min, max,
                                        def, step,
                                        type,
                                        category, group))
        {
            return TRUE;
        }
    }

    return FALSE;
}


/**
 * gst_tcam_bin_get_tcam_manu_entrie:
 * @self: a #GstTcamBin
 * @name: a #char*
 *
 * Return a list of property names
 *
 * Returns: (element-type utf8) (transfer full): a #GSList
 */
static GSList* gst_tcam_bin_get_tcam_menu_entries (TcamProp* self,
                                                   const gchar* name)
{
    if (GST_TCAMBIN(self)->src != nullptr)
    {
        return tcam_prop_get_tcam_menu_entries(TCAM_PROP(GST_TCAMBIN(self)->src), name);
    }
    return nullptr;
}

static gboolean gst_tcam_bin_set_tcam_property (TcamProp* iface,
                                                gchar* name,
                                                const GValue* value)
{
    gboolean ret = FALSE;
    GstTcamBin* self = GST_TCAMBIN(iface);
    struct device_state* ds = (struct device_state*)self->device;

    if (self->src != nullptr)
    {
        if (tcam_prop_set_tcam_property(TCAM_PROP(self->src), name, value))
        {
            return TRUE;
        }
    }

    if (self->whitebalance != nullptr)
    {
        if (tcam_prop_set_tcam_property(TCAM_PROP(self->whitebalance), name, value))
        {
            return TRUE;
        }
    }

    if (self->exposure != nullptr)
    {
        if (tcam_prop_set_tcam_property(TCAM_PROP(self->exposure), name, value))
        {
            return TRUE;
        }
    }

    return FALSE;
}


static GSList* gst_tcam_bin_get_device_serials (TcamProp* self)
{
    if (GST_TCAMBIN(self)->src != nullptr)
    {
        return tcam_prop_get_device_serials(TCAM_PROP(GST_TCAMBIN(self)->src));
    }
    else
    {
        return nullptr;
    }
}


static gboolean gst_tcam_bin_get_device_info (TcamProp* self,
                                              const char* serial,
                                              char** name,
                                              char** identifier,
                                              char** connection_type)
{
    if (GST_TCAMBIN(self)->src != nullptr)
    {
        return tcam_prop_get_device_info(TCAM_PROP(GST_TCAMBIN(self)->src),
                                         serial,
                                         name,
                                         identifier,
                                         connection_type);
    }
    else
    {
        return FALSE;
    }
}


//
// gstreamer module
//

static void gst_tcambin_class_init (GstTcamBinClass* klass);
static void gst_tcambin_init (GstTcamBin* klass);

enum
{
    PROP_0,
    PROP_SERIAL,
    PROP_CAPS,
};

static GstStaticCaps raw_caps = GST_STATIC_CAPS("ANY");

static GstStaticPadTemplate src_template = GST_STATIC_PAD_TEMPLATE("src",
                                                                   GST_PAD_SRC,
                                                                   GST_PAD_ALWAYS,
                                                                   GST_STATIC_CAPS_ANY);


static void gst_tcambin_clear_kid (GstTcamBin* src)
{
    if (src->kid)
    {
        gst_element_set_state (src->kid, GST_STATE_NULL);
        gst_bin_remove (GST_BIN (src), src->kid);
        src->kid = NULL;
        /* Don't loose SOURCE flag */
        GST_OBJECT_FLAG_SET (src, GST_ELEMENT_FLAG_SOURCE);
    }
}


gboolean gst_tcam_is_fourcc_bayer (const guint fourcc)
{
    if (fourcc == GST_MAKE_FOURCC('g', 'b', 'r', 'g')
        || fourcc == GST_MAKE_FOURCC('g', 'r', 'b', 'g')
        || fourcc == GST_MAKE_FOURCC('r', 'g', 'g', 'b')
        || fourcc == GST_MAKE_FOURCC('b', 'g', 'g', 'r'))
    {
        return TRUE;
    }
    return FALSE;
}


gboolean gst_tcam_is_fourcc_rgb (const guint fourcc)
{
    if (fourcc == GST_MAKE_FOURCC('R', 'G', 'B', 'x')
        || fourcc == GST_MAKE_FOURCC('x', 'R', 'G', 'B')
        || fourcc == GST_MAKE_FOURCC('B', 'G', 'R', 'x')
        || fourcc == GST_MAKE_FOURCC('x', 'B', 'G', 'R')
        || fourcc == GST_MAKE_FOURCC('R', 'G', 'B', 'A')
        || fourcc == GST_MAKE_FOURCC('A', 'R', 'G', 'B')
        || fourcc == GST_MAKE_FOURCC('B', 'G', 'R', 'A')
        || fourcc == GST_MAKE_FOURCC('A', 'B', 'G', 'R'))
    {
        return TRUE;
    }

    return FALSE;
}


static required_modules gst_tcambin_generate_src_caps (const GstCaps* available_caps,
                                                       const GstCaps* wanted)
{
    GST_DEBUG("Generating source caps and list of required modules");

    GstStructure* structure = gst_caps_get_structure(wanted, 0);

    struct required_modules modules = {FALSE, FALSE, FALSE, nullptr};

    guint fourcc = 0;

    for (unsigned int i = 0; i < gst_caps_get_size(available_caps); ++i)
    {
        GstCaps* single_caps = gst_caps_copy_nth(available_caps, i);
        gst_caps_unref(single_caps);
    }

    GstCaps* input = gst_caps_copy(wanted);

    GST_DEBUG("Comparing '%s' <==  to  ==> '%s'", gst_caps_to_string(wanted), gst_caps_to_string(available_caps));

    GstCaps* intersection = gst_caps_intersect(wanted, available_caps);

    bool conversion_needed = false;
    if (gst_caps_is_empty(intersection))
    {
        GST_INFO("No intersecting caps found. Trying caps with conversion.");

        gst_caps_unref(intersection);

        gint width = 0;
        gint height = 0;
        double framerate = 0.0;

        gst_structure_get_int (structure, "width", &width);
        gst_structure_get_int (structure, "height", &height);
        const GValue* fps = gst_structure_get_value (structure, "framerate");
        int frame_rate_numerator;
        int frame_rate_denominator;
        gst_util_double_to_fraction(framerate,
                                    &frame_rate_numerator,
                                    &frame_rate_denominator);


        GstCaps* caps = gst_caps_new_simple ("video/x-bayer",
                                             "framerate", GST_TYPE_FRACTION,
                                             gst_value_get_fraction_numerator (fps),
                                             gst_value_get_fraction_denominator (fps),
                                             "width", G_TYPE_INT, width,
                                             "height", G_TYPE_INT, height,
                                             NULL);
        GST_INFO("Testing caps: '%s'", gst_caps_to_string(caps));

        intersection = gst_caps_intersect(caps, available_caps);


        if (gst_caps_is_empty(intersection))
        {
            GST_ERROR("Unable to determine caps for source.");

            return modules;
        }
        else
        {
            GST_INFO("Using caps for source element: '%s'", gst_caps_to_string(intersection));

            modules.caps = gst_caps_copy(intersection);
        }

        structure = gst_caps_get_structure(caps, 0);
        if (gst_structure_get_field_type (structure, "format") == G_TYPE_STRING)
        {
            const char *string = gst_structure_get_string (structure, "format");
            fourcc = GST_STR_FOURCC (string);
        }
        conversion_needed = true;
    }
    else
    {
        GST_INFO("Device has caps. No conversion needed.");
        structure = gst_caps_get_structure(intersection, 0);

        if (gst_structure_get_field_type (structure, "format") == G_TYPE_STRING)
        {
            const char *string = gst_structure_get_string (structure, "format");
            fourcc = GST_STR_FOURCC (string);
        }
        modules.caps = gst_caps_copy(intersection);
        conversion_needed = false;
    }

    if (fourcc == GST_MAKE_FOURCC('G', 'R', 'A', 'Y'))
    {
        modules.bayer = FALSE;
        modules.whitebalance = FALSE;
        modules.convert = FALSE;
    }
    else if (gst_tcam_is_fourcc_bayer(fourcc))
    {
        modules.whitebalance = TRUE;
    }
    else if (gst_tcam_is_fourcc_rgb(fourcc))
    {
        if (conversion_needed)
        {
            modules.bayer = TRUE;
        }
        modules.whitebalance = TRUE;
    }
    else
    {
        if (conversion_needed)
        {
            modules.bayer = TRUE;
        }
        modules.whitebalance = TRUE;
        modules.convert = TRUE;
    }

    return modules;
}


static gboolean gst_tcambin_create_source (GstTcamBin* self)
{

    GST_DEBUG("Creating source...");

    self->src = gst_element_factory_make("tcamsrc", "source");
    gst_bin_add(GST_BIN(self), self->src);

    if (self->device_serial != nullptr)
    {
        GST_INFO("Setting source serial to %s", self->device_serial);
        g_object_set(G_OBJECT(self->src), "serial", self->device_serial);
    }

    return TRUE;
}


static gboolean gst_tcambin_create_elements (GstTcamBin* self)
{

    if (self->elements_created)
    {
        return TRUE;
    }
    GST_INFO("creating elements");

    if (self->src == nullptr)
    {
        gst_tcambin_create_source(self);
    }

    if (self->target_caps == NULL)
    {
        GST_ERROR("Unknown target caps. Aborting.");
        return FALSE;
    }

    self->pipeline_caps = gst_element_factory_make("capsfilter", "src_caps");

    if (self->pipeline_caps == nullptr)
    {
        GST_ERROR("Could not create internal pipeline caps. Aborting");
        return FALSE;
    }

    g_object_set(self->pipeline_caps, "caps", self->modules.caps, NULL);

    gst_bin_add(GST_BIN(self), self->pipeline_caps);

    gst_element_link(self->src,
                     self->pipeline_caps);

    std::string pipeline_description = "tcamscr ! ";
    pipeline_description += gst_caps_to_string(self->modules.caps);

    GstElement* previous_element = self->pipeline_caps;

    if (tcam_prop_get_tcam_property_type(TCAM_PROP(self->src), "Exposure Auto") == nullptr)
    {
        GST_DEBUG("Adding autoexposure to pipeline");
        self->exposure = gst_element_factory_make("tcamautoexposure", "exposure");
        gst_bin_add(GST_BIN(self), self->exposure);
    }

    if (self->modules.bayer)
    {
        // use this to see if the device already has the feature
        if (tcam_prop_get_tcam_property_type(TCAM_PROP(self->src), "Whitebalance Auto") == nullptr)
        {
            self->whitebalance = gst_element_factory_make("tcamwhitebalance", "whitebalance");
            if (self->whitebalance)
            {
                GST_DEBUG("Adding whitebalance to pipeline");
                gst_bin_add(GST_BIN(self), self->whitebalance);

                pipeline_description += " ! tcamwhitebalance";
                gst_element_link(previous_element, self->whitebalance);
                previous_element = self->whitebalance;
            }
            else
            {
                GST_ERROR("Could not create whitebalance element. Aborting.");
                return FALSE;
            }
        }

        self->debayer = gst_element_factory_make("bayer2rgb", "debayer");
        if (self->debayer)
        {
            GST_DEBUG("Adding bayer2rgb to pipeline");
            gst_bin_add(GST_BIN(self), self->debayer);
            pipeline_description += " ! bayer2rgb";

            gst_element_link(previous_element, self->debayer);
            previous_element = self->debayer;

        }
        else
        {
            GST_ERROR("Could not create bayer2rgb element. Aborting.");
            return FALSE;
        }
    }
    if (self->modules.convert)
    {
        self->convert = gst_element_factory_make("videoconvert", "convert");
        if (self->convert)
        {
            // TODO: convert gets added to often.
            // do more checks to prevent unneccessary plugins

            GST_DEBUG("Adding videoconvert to pipeline.");
            gst_bin_add(GST_BIN(self), self->convert);
            pipeline_description += " ! videoconvert";
            gst_element_link(previous_element, self->convert);
            previous_element = self->convert;
        }
        else
        {
            GST_ERROR("Could not create videoconvert element. Aborting.");
            return FALSE;
        }
    }

    GST_INFO("Using %s as exit element for internal pipeline", gst_element_get_name(previous_element));
    self->target_pad = gst_element_get_static_pad(previous_element, "src");
    GST_INFO("Internal pipeline: %s", pipeline_description.c_str());

    return TRUE;
}


static GstStateChangeReturn gst_tcambin_change_state (GstElement* element,
                                                      GstStateChange trans)
{
    GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;

    GstTcamBin* self = GST_TCAMBIN(element);

    switch (trans)
    {
        case GST_STATE_CHANGE_NULL_TO_READY:
        {
            GST_INFO("NULL_TO_READY");

            GstPad* sinkpad = gst_pad_get_peer(self->pad);

            self->target_caps = gst_pad_query_caps (sinkpad, NULL);
            GST_ERROR("caps of sink: %s", gst_caps_to_string(self->target_caps));

            if (self->src == nullptr)
            {
                gst_tcambin_create_source(self);

                GstCaps* src_caps = gst_pad_query_caps(gst_element_get_static_pad(self->src, "src"), NULL);
                GST_ERROR("caps of src: %s", gst_caps_to_string(src_caps));

                self->modules = gst_tcambin_generate_src_caps(src_caps, self->target_caps);
            }

            if (! gst_tcambin_create_elements(self))
            {
                GST_ERROR("Error while creating elements");
            }
            self->elements_created = TRUE;

            if (self->target_set == FALSE)
            {
                if (self->target_pad == nullptr)
                {
                    GST_ERROR("target_pad not defined");
                }

                gst_ghost_pad_set_target(GST_GHOST_PAD(self->pad), self->target_pad);

                self->target_set = TRUE;

                GST_DEBUG("Using caps '%s' for external linkage.", gst_caps_to_string(gst_pad_get_current_caps(self->target_pad)));
            }

            break;
        }
        case GST_STATE_CHANGE_PAUSED_TO_READY:
        {
            GST_INFO("PAUSED_TO_READY");
            break;
        }
        case GST_STATE_CHANGE_READY_TO_PAUSED:
        {
            GST_INFO("READY_TO_PAUSED");

            break;
        }
        default:
        {
            break;
        }
    }

    ret = GST_ELEMENT_CLASS(parent_class)->change_state(element, trans);

    gst_element_sync_state_with_parent(GST_ELEMENT(self));

    return ret;
}


static void gst_tcambin_dispose (GObject* object)
{
    GstTcamBin* self = GST_TCAMBIN(object);
}


static void gst_tcambin_get_property (GObject* object,
                                      guint prop_id,
                                      GValue* value,
                                      GParamSpec* pspec)
{
    GstTcamBin* self = GST_TCAMBIN(object);
    switch (prop_id)
    {
        case PROP_SERIAL:
        {
            g_value_set_string(value, self->device_serial);
            break;
        }
        case PROP_CAPS:
        {
            gst_value_set_caps(value, self->user_caps);
            break;
        }
        default:
        {
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
        }
    }
}


static void gst_tcambin_set_property (GObject* object,
                                      guint prop_id,
                                      const GValue* value,
                                      GParamSpec* pspec)
{
    GstTcamBin* self = GST_TCAMBIN(object);

    switch (prop_id)
    {
        case PROP_SERIAL:
        {
            self->device_serial = g_strdup(g_value_get_string(value));
            break;
        }
        case PROP_CAPS:
        {
            self->user_caps = gst_caps_copy(gst_value_get_caps(value));
            break;
        }
        default:
        {
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
        }
    }
}


static void gst_tcambin_src_reset (GstTcamBin* self)
{

    gst_debug_log(gst_tcambin_debug,
                  GST_LEVEL_ERROR,
                  __FILE__, "reset", __LINE__,
                  NULL, "reset");
    GstPad* targetpad;

    GstTcamBin* src = self;
}


static void gst_tcambin_init (GstTcamBin* self)
{
    GST_DEBUG("init");

    self->pad = gst_ghost_pad_new_no_target("sink", GST_PAD_SRC);
    gst_element_add_pad(GST_ELEMENT(self), self->pad);

    self->caps = gst_static_caps_get(&raw_caps);

    GST_OBJECT_FLAG_SET(self, GST_ELEMENT_FLAG_SOURCE);
}


static void gst_tcambin_finalize (GObject* object)
{
    G_OBJECT_CLASS (parent_class)->finalize(object);
}


static void gst_tcambin_clear_elements (GstTcamBin* self)
{
    if (self->debayer)
    {
        gst_element_set_state(self->debayer, GST_STATE_NULL);
        gst_bin_remove(GST_BIN(self), self->debayer);
        self->debayer = NULL;
    }
}


static void gst_tcambin_dispose (GstTcamBin* self)
{
    GST_DEBUG("dispose");
    if (self->caps)
    {
        gst_caps_unref(self->caps);
    }
    self->caps = NULL;

    G_OBJECT_CLASS(parent_class)->dispose((GObject*) self);
}


static void gst_tcambin_class_init (GstTcamBinClass* klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS (klass);
    GstElementClass* element_class = GST_ELEMENT_CLASS (klass);

    object_class->dispose = (GObjectFinalizeFunc) gst_tcambin_dispose;
    object_class->finalize = gst_tcambin_finalize;
    object_class->set_property = gst_tcambin_set_property;
    object_class->get_property = gst_tcambin_get_property;

    element_class->change_state = GST_DEBUG_FUNCPTR(gst_tcambin_change_state);

    g_object_class_install_property(object_class,
                                    PROP_SERIAL,
                                    g_param_spec_string("serial",
                                                        "Camera serial",
                                                        "Serial of the camera that shall be used",
                                                        NULL,
                                                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
    // not yet used
    // g_object_class_install_property(object_class,
    //                                 PROP_CAPS,
    //                                 g_param_spec_boxed("filter-caps",
    //                                                    "Filter caps",
    //                                                    "Filter src caps",
    //                                                    GST_TYPE_CAPS,
    //                                                    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

    gst_element_class_add_pad_template (element_class,
                                        gst_static_pad_template_get (&src_template));

    gst_element_class_set_details_simple (element_class,
                                          "Tcam Video Bin",
                                          "Source/Video",
                                          "Tcam based bin",
                                          "The Imaging Source <support@theimagingsource.com>");
}


static gboolean plugin_init (GstPlugin* plugin)
{
    GST_DEBUG_CATEGORY_INIT(gst_tcambin_debug, "tcambin", 0, "TcamBin");

    return gst_element_register(plugin, "tcambin", GST_RANK_NONE, GST_TYPE_TCAMBIN);
}

#ifndef TCAMBIN_VERSION
#define TCAMBIN_VERSION "1.0.0"
#endif


#ifndef PACKAGE
#define PACKAGE "tcam"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
                   GST_VERSION_MINOR,
                   tcambin,
                   "Tcam Video Bin",
                   plugin_init,
                   TCAMBIN_VERSION,
                   "Proprietary",
                   "tcambin",
                   "theimagingsource.com")
