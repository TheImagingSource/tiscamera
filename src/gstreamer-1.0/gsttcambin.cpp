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

#include "gsttcambin.h"

#include <unistd.h>

#define gst_tcambin_parent_class parent_class

GST_DEBUG_CATEGORY_STATIC(gst_tcambin_debug);
#define GST_CAT_DEFAULT gst_tcambin_debug

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
G_DEFINE_TYPE(GstTcamBin, gst_tcambin, GST_TYPE_BIN);

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
    GstStructure* structure = gst_caps_get_structure(wanted, 0);

    struct required_modules modules = {FALSE, FALSE, FALSE, nullptr};

    guint fourcc = 0;

    if (gst_structure_get_field_type (structure, "format") == G_TYPE_STRING)
    {
        const char *string = gst_structure_get_string (structure, "format");
        fourcc = GST_STR_FOURCC (string);
    }

    for (unsigned int i = 0; i < gst_caps_get_size(available_caps); ++i)
    {
        GstCaps* single_caps = gst_caps_copy_nth(available_caps, i);
        gst_caps_unref(single_caps);
    }

    GstCaps* input = gst_caps_copy(wanted);
    GstCaps* intersection = gst_caps_intersect(wanted, available_caps);

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
            GST_INFO("Unable to determine caps for source.");

            return modules;
        }
        else
        {
            GST_INFO("Using source caps '%s'", gst_caps_to_string(intersection));

            modules.caps = gst_caps_copy(intersection);
        }

        structure = gst_caps_get_structure(caps, 0);
        if (gst_structure_get_field_type (structure, "format") == G_TYPE_STRING)
        {
            const char *string = gst_structure_get_string (structure, "format");
            fourcc = GST_STR_FOURCC (string);
        }
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
        modules.bayer = TRUE;
        modules.whitebalance = TRUE;
    }
    else
    {
        modules.bayer = TRUE;
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

    g_object_set(self->pipeline_caps, "caps", self->modules.caps, NULL);

    gst_bin_add(GST_BIN(self), self->pipeline_caps);

    self->exposure = gst_element_factory_make("tcamautoexposure", "exposure");
    gst_bin_add(GST_BIN(self), self->exposure);

    if (self->modules.bayer)
    {
        self->whitebalance = gst_element_factory_make("tcamwhitebalance", "whitebalance");
        gst_bin_add(GST_BIN(self), self->whitebalance);

        self->debayer = gst_element_factory_make("bayer2rgb", "debayer");
        gst_bin_add(GST_BIN(self), self->debayer);
    }

    if (self->modules.convert)
    {
        self->convert = gst_element_factory_make("videoconvert", "convert");
        gst_bin_add(GST_BIN(self), self->convert);
    }

    if (self->whitebalance != nullptr)
    {
        gst_element_link(self->src,
                         self->pipeline_caps
                         );
        gst_element_link(self->pipeline_caps,
                         self->whitebalance
                         );
        gst_element_link(self->whitebalance,
                         self->exposure
                         );
        gst_element_link(self->exposure,
                         self->debayer
            );

        // if (self->modules.convert)
        // {
        //     gst_element_link_pads_full(self->debayer, "src",
        //                                self->convert, "sink",
        //                                GST_PAD_LINK_CHECK_NOTHING);
        //     self->target_pad = gst_element_get_static_pad(self->convert, "src");
        //     GST_DEBUG("Using videoconvert as exit element for ghost pad");
        // }
        // else
        // {
            GST_DEBUG("Using bayer2rgb as exit element for ghost pad");
            self->target_pad = gst_element_get_static_pad(self->debayer, "src");
        // }
    }
    else
    {
        gst_element_link(self->src,
                         self->pipeline_caps
            );
        gst_element_link(self->pipeline_caps,
                         self->exposure
            );


        self->target_pad = gst_element_get_static_pad(self->exposure, "src");
        GST_DEBUG("Using exposure as exit element for ghost pad");

    }

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
                GST_ERROR("caps of src: %s", gst_caps_to_string(self->target_caps));

                self->modules = gst_tcambin_generate_src_caps(src_caps, self->target_caps);
            }

            if (! gst_tcambin_create_elements(self))
            {
                GST_ERROR("Error while creating elements");
            }
            self->elements_created = TRUE;

            if (self->target_set == FALSE)
            {
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
                  NULL, "reset ====================");
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
