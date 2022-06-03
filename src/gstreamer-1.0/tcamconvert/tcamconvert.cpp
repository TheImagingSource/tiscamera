/*
 * Copyright 2021 The Imaging Source Europe GmbH
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

#include "tcamconvert.h"

#include "../../version.h"
#include "tcamconvert_context.h"

#include <dutils_img/dutils_img.h>
#include <dutils_img/fcc_to_string.h>
#include <dutils_img_lib/dutils_gst_interop.h>
#include <gst-helper/gst_gvalue_helper.h>
#include <gst-helper/gstcaps_dutils_interop.h>
#include <gst-helper/helper_functions.h>
#include <gst/video/gstvideometa.h>
#include <vector>

enum
{
    PROP_0,
};

GST_DEBUG_CATEGORY_STATIC(gst_tcamconvert_debug_category);
#define GST_CAT_DEFAULT gst_tcamconvert_debug_category

#define gst_tcamconvert_parent_class parent_class
G_DEFINE_TYPE(GstTCamConvert, gst_tcamconvert, GST_TYPE_BASE_TRANSFORM)


static tcamconvert::tcamconvert_context_base& get_gst_elem_reference(GstTCamConvert* iface)
{
    GstTCamConvert* self = GST_TCAMCONVERT(iface);
    assert(self != nullptr);

    return *self->context_;
}

/* No properties are implemented, so only a warning is produced */
static void gst_tcamconvert_set_property(GObject* object __attribute__((unused)),
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

static void gst_tcamconvert_get_property(GObject* object __attribute__((unused)),
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

static gboolean gst_tcamconvert_set_caps(GstBaseTransform* base, GstCaps* incaps, GstCaps* outcaps)
{
    GstTCamConvert* self = GST_TCAMCONVERT(base);
    if (!self || !incaps || !outcaps)
    {
        return FALSE;
    }

    auto& elem = get_gst_elem_reference(self);

    auto src = gst_helper::get_img_type_from_fixated_gstcaps(*incaps);
    if (src.empty())
    {
        return FALSE;
    }
    auto dst = gst_helper::get_img_type_from_fixated_gstcaps(*outcaps);
    if (dst.empty())
    {
        return FALSE;
    }

    if (!elem.setup(src, dst))
    {
        GST_ELEMENT_ERROR(self,
                          STREAM,
                          FORMAT,
                          ("Failed to find conversion from %s to %s",
                           img::fcc_to_string(src.type).c_str(),
                           img::fcc_to_string(dst.type).c_str()),
                          (NULL));
        return FALSE;
    }
    return TRUE;
}


static void create_fmt(GstCaps* res_caps,
                       const GstStructure* structure,
                       img::fourcc fourcc,
                       GstPadDirection direction)
{
    std::vector<img::fourcc> vec;
    if (direction == GST_PAD_SRC)
    {
        vec = tcamconvert::tcamconvert_get_supported_input_fccs(fourcc);
    }
    else
    {
        vec = tcamconvert::tcamconvert_get_supported_output_fccs(fourcc);
    }

    for (const auto& fcc : vec)
    {
        auto caps_fmt = img_lib::gst::fourcc_to_gst_caps_descr(fcc);

        if (!caps_fmt.gst_struct_name)
        {
            continue;
        }

        // copy the incoming structure
        // and replace name and (if used) format
        // this way all additional information (width, fps, binning, etc) are preserved
        // we do not touch these information as we do not resize, etc the GstBuffer

        GstStructure* tmp_struc = gst_structure_copy(structure);

        gst_structure_set_name(tmp_struc, caps_fmt.gst_struct_name);

        if (caps_fmt.format_entry)
        {
            gst_structure_set(tmp_struc, "format", G_TYPE_STRING, caps_fmt.format_entry, nullptr);
        }

        // gst_caps_new_full takes ownership of tmp_struc
        GstCaps* caps_to_add = gst_caps_new_full(tmp_struc, nullptr);

        gst_caps_append(res_caps, caps_to_add);
    }
}

static GstCaps* transform_caps(GstCaps* caps, GstPadDirection direction)
{
    GstCaps* res_caps = gst_caps_new_empty();

    unsigned int caps_count = gst_caps_get_size(caps);
    for (unsigned int i = 0; i < caps_count; i++)
    {
        GstStructure* structure = gst_caps_get_structure(caps, i);

        auto fcc_vec = gst_helper::convert_GstStructure_to_fcc_list(*structure);

        // for every entry in fcc_vec create a GstCaps that is appended to res_caps
        for (auto&& fcc : fcc_vec) { create_fmt(res_caps, structure, fcc, direction); }
    }

    // res_caps = gst_caps_simplify(res_caps); // This seems to simplify in a 'curious' way, so we should not use this here

    if (direction == GST_PAD_SRC)
    {
        GST_DEBUG("Returning INPUT: %s", gst_helper::to_string(*res_caps).c_str());
    }
    else
    {
        GST_DEBUG("Returning OUTPUT: %s", gst_helper::to_string(*res_caps).c_str());
    }
    return res_caps;
}

static GstCaps* gst_tcamconvert_transform_caps(GstBaseTransform* base,
                                               GstPadDirection direction,
                                               GstCaps* caps,
                                               GstCaps* filter)
{
    auto dir_to_string = [](GstPadDirection dir)
    {
        return dir == GST_PAD_SRC ? "GST_PAD_SRC" : "GST_PAD_SINK";
    };

    GstCaps* res_caps = transform_caps(caps, direction);
    if (filter)
    {
        GstCaps* tmp_caps = res_caps;
        res_caps = gst_caps_intersect_full(filter, tmp_caps, GST_CAPS_INTERSECT_FIRST);
        gst_caps_unref(tmp_caps);
    }

    GST_DEBUG_OBJECT(base,
                     "dir=%s transformed %s into %s",
                     dir_to_string(direction),
                     gst_helper::to_string(*caps).c_str(),
                     gst_helper::to_string(*res_caps).c_str());

    if (gst_caps_is_empty(caps) || gst_caps_is_empty(res_caps))
    {
        GST_ELEMENT_ERROR(
            base, STREAM, FORMAT, (("Unable to convert between caps formats")), (NULL));
    }

    return res_caps;
}

static gboolean gst_tcamconvert_get_unit_size(GstBaseTransform* trans, GstCaps* caps, gsize* size)
{
    GstStructure* structure = gst_caps_get_structure(caps, 0);

    auto type = gst_helper::get_gst_struct_image_type(*structure);
    if (type.empty())
    {
        GST_ELEMENT_ERROR(trans,
                          CORE,
                          NEGOTIATION,
                          ("Incomplete caps, format/dimensions missing or unknown"),
                          (NULL));
        return FALSE;
    }
    size_t img_size = img::calc_minimum_img_size(type.fourcc_type(), type.dim);
    if (img_size == 0)
    {
        GST_ELEMENT_ERROR(trans,
                          CORE,
                          NEGOTIATION,
                          ("Incomplete caps, unable to compute image size from format and dim"),
                          (NULL));
        return FALSE;
    }

    *size = img_size;
    return TRUE;
}

static gboolean gst_tcamconvert_transform_size(GstBaseTransform* trans,
                                               GstPadDirection /*direction*/,
                                               GstCaps* /*caps*/,
                                               gsize /*size*/,
                                               GstCaps* othercaps,
                                               gsize* othersize)
{
    return gst_tcamconvert_get_unit_size(trans, othercaps, othersize);
}

static int get_mapped_stride(GstBuffer* buffer_) noexcept
{
    auto video_meta_ptr = reinterpret_cast<GstVideoMeta*>(
        gst_buffer_get_meta(buffer_, gst_video_meta_api_get_type()));
    if (video_meta_ptr != nullptr && video_meta_ptr->stride[0] != 0)
    {
        return video_meta_ptr->stride[0];
    }
    return 0;
}

static img::img_descriptor make_img_desc_from_input_buffer(const img::img_type& src_type,
                                                           guint8* map_in_data,
                                                           GstBuffer* inbuf)
{
    if (int actual_src_stride = get_mapped_stride(inbuf); actual_src_stride)
    {
        return img::make_img_desc_raw(src_type, img::img_plane { map_in_data, actual_src_stride });
    }
    return img::make_img_desc_from_linear_memory(
        src_type, map_in_data); // no explicit stride mentioned, so assume linear memory
}

static GstFlowReturn gst_tcamconvert_transform(GstBaseTransform* base,
                                               GstBuffer* inbuf,
                                               GstBuffer* outbuf)
{
    auto self = GST_TCAMCONVERT(base);
    auto& elem = get_gst_elem_reference(self);

    GstMapInfo map_in;
    if (!gst_buffer_map(inbuf, &map_in, GST_MAP_READ))
    {
        GST_ERROR_OBJECT(self, "Input buffer could not be mapped");
        return GST_FLOW_OK;
    }

    GstMapInfo map_out;
    if (!gst_buffer_map(outbuf, &map_out, GST_MAP_WRITE) || map_out.data == nullptr)
    {
        gst_buffer_unmap(inbuf, &map_in);

        GST_ERROR_OBJECT(self, "Output buffer could not be mapped");
        return GST_FLOW_OK;
    }

    auto src = make_img_desc_from_input_buffer(elem.src_type_, map_in.data, inbuf);
    auto dst = img::make_img_desc_from_linear_memory(elem.dst_type_, map_out.data);

    elem.transform(src, dst);

    gst_buffer_unmap(outbuf, &map_out);
    gst_buffer_unmap(inbuf, &map_in);

    return GST_FLOW_OK;
}

static GstFlowReturn gst_tcamconvert_transform_ip(GstBaseTransform* base, GstBuffer* inbuf)
{
    auto& elem = get_gst_elem_reference(GST_TCAMCONVERT(base));

    GstMapInfo map_in;
    if (!gst_buffer_map(inbuf, &map_in, GST_MAP_READWRITE))
    {
        GST_ERROR_OBJECT(base, "Input buffer could not be mapped");
        return GST_FLOW_OK;
    }

    auto src = make_img_desc_from_input_buffer(elem.src_type_, map_in.data, inbuf);

    elem.filter(src);

    gst_buffer_unmap(inbuf, &map_in);

    return GST_FLOW_OK;
}

/**
 * Helper function for copy_metadata
 */
static gboolean foreach_metadata(GstBuffer* inbuf, GstMeta** meta, gpointer user_data)
{
    GstBuffer* outbuf = static_cast<GstBuffer*>(user_data);

    const GstMetaInfo* info = (*meta)->info;

    // check memory tag
    // This metadata stays relevant as long as memory layout is unchanged.
    // copying thus invalidates the meta data
    // https://gstreamer.freedesktop.org/documentation/gstreamer/gstmeta.html?gi-language=c#GST_META_TAG_MEMORY_STR
    if (!gst_meta_api_type_has_tag(info->api, _gst_meta_tag_memory))
    {
        // ignore narrowing warning for -1
        // gstreamer documentation says it has to be -1 for off
#pragma GCC diagnostic ignored "-Wnarrowing"
        GstMetaTransformCopy copy_data = { FALSE, 0, -1 };
#pragma GCC diagnostic push "-Wnarrowing"

        info->transform_func(outbuf, *meta, inbuf, _gst_meta_transform_copy, &copy_data);
    }
    return TRUE;
}


static gboolean gst_tcamconvert_copy_metadata(GstBaseTransform* base,
                                              GstBuffer* inbuf,
                                              GstBuffer* outbuf)
{
    /* now copy the metadata */
    // GST_DEBUG_OBJECT(base, "copying metadata");

    /* this should not happen, buffers allocated from a pool or with
     * new_allocate should always be writable. */
    if (!gst_buffer_is_writable(outbuf))
    {
        GST_WARNING_OBJECT(base, "buffer %p not writable", (void*)outbuf);
        return FALSE;
    }

    /* when we get here, the metadata should be writable */
    gst_buffer_copy_into(outbuf,
                         inbuf,
                         (GstBufferCopyFlags)(GST_BUFFER_COPY_FLAGS | GST_BUFFER_COPY_TIMESTAMPS),
                         0,
                         -1);

    gst_buffer_foreach_meta(inbuf, foreach_metadata, outbuf);

    return TRUE;
}

static GstStateChangeReturn gst_tcamconvert_change_state(GstElement* element, GstStateChange trans)
{
    auto& elem = get_gst_elem_reference(GST_TCAMCONVERT(element));

    GstStateChangeReturn ret = GST_ELEMENT_CLASS(parent_class)->change_state(element, trans);
    switch (trans)
    {
        case GST_STATE_CHANGE_NULL_TO_READY:
        {
            elem.try_connect_to_source(false);
            break;
        }
        default:
            break;
    }
    return ret;
}


static void gst_tcamdutils_sink_pad_linked(GstPad* /*self*/, GstPad* /*peer*/, gpointer user_data)
{
    static_cast<GstTCamConvert*>(user_data)->context_->on_input_pad_linked();
}

static void gst_tcamdutils_sink_pad_unlinked(GstPad* /*self*/, GstPad* /*peer*/, gpointer user_data)
{
    static_cast<GstTCamConvert*>(user_data)->context_->on_input_pad_unlinked();
}

static void gst_tcamconvert_init(GstTCamConvert* self)
{
    self->context_ = new tcamconvert::tcamconvert_context_base(self);

    gst_base_transform_set_in_place(GST_BASE_TRANSFORM(self), FALSE);

    auto sink_pad = gst_helper::get_static_pad(*GST_ELEMENT(self), "sink");
    g_signal_connect(sink_pad.get(), "linked", G_CALLBACK(gst_tcamdutils_sink_pad_linked), self);
    g_signal_connect(
        sink_pad.get(), "unlinked", G_CALLBACK(gst_tcamdutils_sink_pad_unlinked), self);
}

static void gst_tcamconvert_dispose(GObject* object)
{
    auto* self = GST_TCAMCONVERT(object);
    {
        auto sink_pad = gst_helper::get_static_pad(*GST_ELEMENT(self), "sink");
        /*int res =*/g_signal_handlers_disconnect_by_data(sink_pad.get(), self);
        //assert(res == 2);
    }
    G_OBJECT_CLASS(gst_tcamconvert_parent_class)->dispose(object);
}

static void gst_tcamconvert_finalize(GObject* object)
{
    delete GST_TCAMCONVERT(object)->context_;
    G_OBJECT_CLASS(gst_tcamconvert_parent_class)->finalize(object);
}

static void gst_tcamconvert_class_init(GstTCamConvertClass* klass)
{
    GObjectClass* gobject_class = (GObjectClass*)klass;
    GstElementClass* gstelement_class = (GstElementClass*)klass;
    GstBaseTransformClass* gst_base_transform_class = GST_BASE_TRANSFORM_CLASS(klass);

    gobject_class->set_property = gst_tcamconvert_set_property;
    gobject_class->get_property = gst_tcamconvert_get_property;
    gobject_class->dispose = gst_tcamconvert_dispose;
    gobject_class->finalize = gst_tcamconvert_finalize;


    gst_element_class_set_static_metadata(
        gstelement_class,
        "The Imaging Source TCamConvert gstreamer element",
        "Filter/Converter/Video",
        "Converts Mono/Bayer-10/12/16 bit formats to Mono/Bayer-8/16 bit images",
        "The Imaging Source <support@theimagingsource.com>");


    auto src_caps =
        gst_helper::generate_caps_with_dim(tcamconvert::tcamconvert_get_all_output_fccs());

    gst_element_class_add_pad_template(
        gstelement_class, gst_pad_template_new("src", GST_PAD_SRC, GST_PAD_ALWAYS, src_caps.get()));


    auto sink_caps =
        gst_helper::generate_caps_with_dim(tcamconvert::tcamconvert_get_all_input_fccs());

    gst_element_class_add_pad_template(
        gstelement_class,
        gst_pad_template_new("sink", GST_PAD_SINK, GST_PAD_ALWAYS, sink_caps.get()));

    gst_base_transform_class->transform_size = GST_DEBUG_FUNCPTR(gst_tcamconvert_transform_size);
    gst_base_transform_class->transform_caps = GST_DEBUG_FUNCPTR(gst_tcamconvert_transform_caps);
    gst_base_transform_class->get_unit_size = GST_DEBUG_FUNCPTR(gst_tcamconvert_get_unit_size);
    gst_base_transform_class->set_caps = GST_DEBUG_FUNCPTR(gst_tcamconvert_set_caps);
    gst_base_transform_class->transform = GST_DEBUG_FUNCPTR(gst_tcamconvert_transform);
    gst_base_transform_class->transform_ip = GST_DEBUG_FUNCPTR(gst_tcamconvert_transform_ip);
    gst_base_transform_class->copy_metadata = GST_DEBUG_FUNCPTR(gst_tcamconvert_copy_metadata);
    gstelement_class->change_state = GST_DEBUG_FUNCPTR(gst_tcamconvert_change_state);

    // Mark this transform element as 'calling tranform_ip when src and sink caps are the same
    gst_base_transform_class->passthrough_on_same_caps = TRUE;
    //gst_base_transform_class->transform_ip_on_passthrough = TRUE;

    GST_DEBUG_CATEGORY_INIT(
        gst_tcamconvert_debug_category, "tcamconvert", 0, "tcamconvert element");
}


static gboolean plugin_init(GstPlugin* plugin)
{
    return gst_element_register(plugin, "tcamconvert", GST_RANK_NONE, GST_TYPE_TCAMCONVERT);
}

#ifndef PACKAGE
#define PACKAGE "tcamconvert"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "tcamconvert"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "https://github.com/TheImagingSource/tiscamera"
#endif


GST_PLUGIN_DEFINE(GST_VERSION_MAJOR,
                  GST_VERSION_MINOR,
                  tcamconvert,
                  "The Imaging Source tcamconvert plugin",
                  plugin_init,
                  get_version(),
                  "Proprietary",
                  PACKAGE_NAME,
                  GST_PACKAGE_ORIGIN)
