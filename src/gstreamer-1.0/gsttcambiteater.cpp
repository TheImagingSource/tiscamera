/*
 * Copyright 2017 The Imaging Source Europe GmbH
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

#include "gsttcambiteater.h"
#include <gst/video/video.h>

#include "tcamgstbase.h"
#include "tcamgststrings.h"


//
// Do not error on format warnings. They should happen only in debug statements anyway
//
#pragma GCC diagnostic ignored "-Wformat"


enum
{
    PROP_0,
};

GST_DEBUG_CATEGORY_STATIC(gst_tcambiteater_debug_category);
#define GST_CAT_DEFAULT gst_tcambiteater_debug_category



#define gst_tcambiteater_parent_class parent_class
G_DEFINE_TYPE(GstTcamBitEater, gst_tcambiteater, GST_TYPE_BASE_TRANSFORM)


static void gst_tcambiteater_set_property (GObject* object,
                                         guint prop_id,
                                         const GValue* value,
                                         GParamSpec* pspec);
static void gst_tcambiteater_get_property (GObject* object,
                                         guint prop_id,
                                         GValue* value,
                                         GParamSpec* pspec);

static gboolean gst_tcambiteater_set_caps (GstBaseTransform* filter,
                                         GstCaps* incaps,
                                         GstCaps* outcaps);

static GstFlowReturn gst_tcambiteater_transform (GstBaseTransform* base,
                                               GstBuffer* inbuf,
                                               GstBuffer* outbuf);
static void gst_tcambiteater_reset (GstTcamBitEater* filter);

static void gst_tcambiteater_finalize (GObject* object);


static GstCaps *gst_tcambiteater_transform_caps (GstBaseTransform* base,
                                               GstPadDirection direction,
                                               GstCaps* caps,
                                               GstCaps* filter);

static gboolean gst_tcambiteater_get_unit_size (GstBaseTransform* base,
                                              GstCaps* caps,
                                              gsize* size);


static void gst_tcambiteater_class_init (GstTcamBitEaterClass* klass)
{
    GObjectClass* gobject_class;
    GstElementClass* gstelement_class;

    gobject_class = (GObjectClass*) klass;
    gstelement_class = (GstElementClass*) klass;

    gobject_class->set_property = gst_tcambiteater_set_property;
    gobject_class->get_property = gst_tcambiteater_get_property;
    gobject_class->finalize = gst_tcambiteater_finalize;


    gst_element_class_set_static_metadata(gstelement_class,
                                          "Bit depth reduction module",
                                          "Filter/Converter/Video",
                                          "Converts 64bit rgb to 32 bit",
                                          "The Imaging Source <support@theimagingsource.com>");


#define	SRC_CAPS                                                        \
    GST_VIDEO_CAPS_MAKE("{ BGRx }")


    gst_element_class_add_pad_template(gstelement_class,
                                       gst_pad_template_new("src", GST_PAD_SRC, GST_PAD_ALWAYS,
                                                            gst_caps_from_string(SRC_CAPS)));

    GstCaps* caps = gst_caps_from_string("video/x-raw,format=(string)RGBx64," \
                                         "width=(int)[1,MAX],height=(int)[1,MAX],framerate=(fraction)[0/1,MAX];");

    gst_element_class_add_pad_template(gstelement_class,
                                       gst_pad_template_new("sink",
                                                            GST_PAD_SINK, GST_PAD_ALWAYS,
                                                            caps));

    GST_BASE_TRANSFORM_CLASS(klass)->transform_caps = GST_DEBUG_FUNCPTR(gst_tcambiteater_transform_caps);
    GST_BASE_TRANSFORM_CLASS(klass)->get_unit_size = GST_DEBUG_FUNCPTR(gst_tcambiteater_get_unit_size);
    GST_BASE_TRANSFORM_CLASS(klass)->set_caps = GST_DEBUG_FUNCPTR(gst_tcambiteater_set_caps);
    GST_BASE_TRANSFORM_CLASS(klass)->transform = GST_DEBUG_FUNCPTR(gst_tcambiteater_transform);

    GST_DEBUG_CATEGORY_INIT(gst_tcambiteater_debug_category,
                            "tcambiteater", 0,
                            "tcambiteater element");
}


static void gst_tcambiteater_init (GstTcamBitEater* self)
{
    //self->dutils = new struct dutils_state();

    gst_tcambiteater_reset(self);
    gst_base_transform_set_in_place(GST_BASE_TRANSFORM(self), TRUE);
}


/* No properties are implemented, so only a warning is produced */
static void gst_tcambiteater_set_property (GObject* object __attribute__((unused)),
                                           guint prop_id,
                                           const GValue* value __attribute__((unused)),
                                           GParamSpec* pspec)
{
    switch (prop_id)
    {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}


static void gst_tcambiteater_get_property (GObject* object __attribute__((unused)),
                                           guint prop_id,
                                           GValue* value __attribute__((unused)),
                                           GParamSpec* pspec)
{

    switch (prop_id)
    {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}


static gboolean gst_tcambiteater_set_caps (GstBaseTransform* base,
                                           GstCaps* incaps,
                                           GstCaps* outcaps)
{
    auto self = GST_TCAMBITEATER(base);

    struct tcam_video_format in = {};
    struct tcam_video_format out = {};

    gst_caps_to_tcam_video_format(incaps, &in);
    gst_caps_to_tcam_video_format(outcaps, &out);

    self->buffer_template_in.format = in;
    self->buffer_template_out.format = out;

    tcam::biteater::init_meta(self->be_meta, in, out);

    return TRUE;
}


static void gst_tcambiteater_reset (GstTcamBitEater* self __attribute__((unused)))
{
}


static void gst_tcambiteater_finalize (GObject* object)
{
    G_OBJECT_CLASS(gst_tcambiteater_parent_class)->finalize(object);
}


static GstCaps* gst_tcambiteater_transform_caps (GstBaseTransform* base __attribute__((unused)),
                                                 GstPadDirection direction,
                                                 GstCaps* caps,
                                                 GstCaps* filter)
{
    GstCaps* res_caps = gst_caps_copy(caps);
    size_t caps_size = gst_caps_get_size(res_caps);

    for (guint i = 0; i < caps_size; i++)
    {
        GstStructure* structure = gst_caps_get_structure (res_caps, i);
        if (direction == GST_PAD_SINK)
        {
            gst_structure_set_name(structure, "video/x-raw");
            // gst_structure_remove_field (structure, "format");
            gst_structure_set(structure, "format", G_TYPE_STRING, "BGRx", NULL);
        }
        else
        {
            gst_structure_set_name(structure, "video/x-raw");
            gst_structure_set(structure, "format", G_TYPE_STRING, "RGBx64", NULL);

            // gst_structure_remove_fields (structure, "format", "colorimetry",
                                         // "chroma-site", NULL);
        }
    }
    if (filter)
    {
        GstCaps* tmp_caps = res_caps;
        res_caps = gst_caps_intersect_full(filter,
                                           tmp_caps,
                                           GST_CAPS_INTERSECT_FIRST);
        gst_caps_unref(tmp_caps);
    }
    GST_DEBUG("transformed %" GST_PTR_FORMAT " into %"
              GST_PTR_FORMAT, static_cast<void*>(caps), static_cast<void*>(res_caps));
    return res_caps;
}


static gboolean gst_tcambiteater_get_unit_size (GstBaseTransform* base,
                                                GstCaps* caps,
                                                gsize* size)
{
    GstStructure* structure;
    int width;
    int height;
    structure = gst_caps_get_structure(caps, 0);

    if (gst_structure_get_int(structure, "width", &width) &&
        gst_structure_get_int(structure, "height", &height))
    {
        auto type = gst_structure_get_field_type(structure, "format");
        const char* format = nullptr;

        if (type == G_TYPE_STRING)
        {
            format = gst_structure_get_string(structure, "format");
        }
        else if (type == GST_TYPE_LIST)
        {
            const GValue* value = gst_structure_get_value(structure, "format");
            format = gst_value_serialize(value);

            GST_ERROR("format is a list not a single entry");
        }
        else
        {
            GST_ERROR("%s", g_type_name(type));
        }

        auto s = tcam::get_image_size(tcam_fourcc_from_gst_1_0_caps_string(gst_structure_get_name(structure), format), width, height);

        if (s != 0)
        {
            *size = s;
            return TRUE;
        }
        else
        {
            /* For output, calculate according to format (always 32 bits) */
            *size = width * height * 4;
            return TRUE;
        }

    }
    GST_ELEMENT_ERROR (base, CORE, NEGOTIATION, (NULL),
                       ("Incomplete caps, some required field missing"));
    return FALSE;
}


static GstFlowReturn gst_tcambiteater_transform (GstBaseTransform* base,
                                                 GstBuffer* inbuf,
                                                 GstBuffer* outbuf)
{
    auto self = GST_TCAMBITEATER(base);
    GstMapInfo map_in;
    GstMapInfo map_out;

    if (!gst_buffer_map(inbuf, &map_in, GST_MAP_READ))
    {
        GST_ERROR("Input buffer could not be mapped");
        goto map_failed;
    }
    if (!gst_buffer_map(outbuf, &map_out, GST_MAP_WRITE))
    {
        GST_ERROR("Output buffer could not be mapped");
        goto map_failed;
    }

    // scope required to prevent label error from goto
    {
        struct tcam_image_buffer in = self->buffer_template_in;

        in.pData = map_in.data;
        in.length = map_in.size;

        struct tcam_image_buffer out = self->buffer_template_out;

        out.pData = map_out.data;
        out.length = map_out.size;

        tcam::biteater::transform(&in, &out, self->be_meta);

        gst_buffer_unmap(outbuf, &map_out);
        gst_buffer_unmap(inbuf, &map_in);
    }
    return GST_FLOW_OK;

map_failed:
    GST_ERROR("Could not map buffer, skipping");
    return GST_FLOW_OK;
}


static gboolean plugin_init (GstPlugin* plugin)
{
    return gst_element_register(plugin,
                                "tcambiteater",
                                GST_RANK_NONE,
                                GST_TYPE_TCAMBITEATER);
}

#ifndef VERSION
#define VERSION "0.0.1"
#endif
#ifndef PACKAGE
#define PACKAGE "tcambiteater"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "tcambiteater"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "https://github.com/TheImagingSource/tiscamera"
#endif


GST_PLUGIN_DEFINE(GST_VERSION_MAJOR,
                  GST_VERSION_MINOR,
                  tcambiteater,
                  "The Imaging Source biteater plugin",
                  plugin_init,
                  VERSION,
                  "Proprietary",
                  PACKAGE_NAME, GST_PACKAGE_ORIGIN)
