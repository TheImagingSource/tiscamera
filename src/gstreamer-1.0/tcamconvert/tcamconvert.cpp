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

#include "tcamconvert.h"

#include <gst/video/gstvideometa.h>

#include <vector>

//#include <gst_helper/gvalue_helper.h>
//#include <gst_helper/gst_caps_helper.h>
#include <gst_caps_helper.h>

#include <dutils_img/image_transform_base.h>
#include <dutils_img/image_bayer_pattern.h>

#include "../../lib/dutils_image/src/dutils_img_filter/transform/fcc1x_packed/fcc1x_packed_to_fcc.h"
#include <dutils_img/dutils_cpu_features.h>
#include <dutils_img/fcc_to_string.h>

#include <dutils_img_lib/dutils_gst_interop.h>


namespace gst_helper
{
inline std::vector<std::string> gst_string_list_to_vector(const GValue* gst_list)
{
    if (!GST_VALUE_HOLDS_LIST(gst_list))
    {
        GST_ERROR("Given GValue is not a list.");
        return {};
    }

    unsigned int gst_list_size = gst_value_list_get_size(gst_list);

    std::vector<std::string> ret;
    ret.reserve(gst_list_size);
    for (unsigned int i = 0; i < gst_list_size; ++i)
    {
        const GValue* val = gst_value_list_get_value(gst_list, i);
        if (G_VALUE_TYPE(val) == G_TYPE_STRING)
        {
            ret.push_back(g_value_get_string(val));
        }
        else
        {
            GST_ERROR("List does not only contain strings.");
        }
    }
    return ret;
}
}

enum
{
    PROP_0,
};

GST_DEBUG_CATEGORY_STATIC(gst_tcamconvert_debug_category);
#define GST_CAT_DEFAULT gst_tcamconvert_debug_category

#define gst_tcamconvert_parent_class parent_class
G_DEFINE_TYPE(GstTCamConvert, gst_tcamconvert, GST_TYPE_BASE_TRANSFORM)


static void gst_tcamconvert_set_property (GObject* object,
                                         guint prop_id,
                                         const GValue* value,
                                         GParamSpec* pspec);
static void gst_tcamconvert_get_property (GObject* object,
                                         guint prop_id,
                                         GValue* value,
                                         GParamSpec* pspec);

static gboolean gst_tcamconvert_set_caps (GstBaseTransform* filter,
                                         GstCaps* incaps,
                                         GstCaps* outcaps);

static GstFlowReturn gst_tcamconvert_transform (GstBaseTransform* base,
                                               GstBuffer* inbuf,
                                               GstBuffer* outbuf);

static void gst_tcamconvert_finalize (GObject* object);


static GstCaps *gst_tcamconvert_transform_caps (GstBaseTransform* base,
                                               GstPadDirection direction,
                                               GstCaps* caps,
                                               GstCaps* filter);

static gboolean gst_tcamconvert_get_unit_size (GstBaseTransform* base,
                                              GstCaps* caps,
                                              gsize* size);

static gboolean gst_tcamconvert_transform_size( GstBaseTransform* trans,
                                                  GstPadDirection direction,
                                                  GstCaps* caps, gsize size,
                                                  GstCaps* othercaps, gsize* othersize );



static GstCaps*    generate_caps_struct( const std::vector<img::fourcc>& fcc_list )
{
    GstCaps* caps = gst_caps_new_empty();   // this is returned and the caller must take ownership

    for( const auto fcc : fcc_list )
    {
        std::string caps_string = img_lib::gst::fourcc_to_gst_caps_string( fcc );
        if( caps_string.empty() )
        {
            //GST_WARN( "Format has empty caps string. Ignoring %s", img::fcc_to_string( fcc ).c_str() );
            continue;
        }

        GstStructure* structure = gst_structure_from_string( caps_string.c_str(), NULL );

        int min_cx = 1;
        // This does not seem to work, so we are currently skipping this
        //if( img::get_bits_per_pixel( fcc ) == 10 )
        //{
        //    min_cx = 4;
        //}
        //else if( img::get_bits_per_pixel( fcc ) == 12 )
        //{
        //    min_cx = 2;
        //}

        GValue gvalue_width = G_VALUE_INIT;
        g_value_init( &gvalue_width, GST_TYPE_INT_RANGE );
        gst_value_set_int_range_step( &gvalue_width, min_cx, std::numeric_limits<gint>::max(), min_cx );

        GValue gvalue_height = G_VALUE_INIT;
        g_value_init( &gvalue_height, GST_TYPE_INT_RANGE );
        gst_value_set_int_range_step( &gvalue_height, 1, std::numeric_limits<gint>::max(), 1 );

        gst_structure_take_value( structure, "width", &gvalue_width );          // takes ownership of gvalue_width
        gst_structure_take_value( structure, "height", &gvalue_height );        // takes ownership of gvalue_height
        //gst_structure_take_value( structure, "framerate", &gvalue_fps_range );  // takes ownership of gvalue_fps_range
        gst_caps_append_structure( caps, structure );                           // takes ownership of structure
    }
    return caps;
}

static std::vector<img::fourcc>   tcamconvert_get_all_input_fccs()
{
    return
    {
        img::fourcc::MONO8,
        img::fourcc::GRBG8,
        img::fourcc::RGGB8,
        img::fourcc::GBRG8,
        img::fourcc::BGGR8,

        img::fourcc::MONO10,
        img::fourcc::GRBG10,
        img::fourcc::RGGB10,
        img::fourcc::GBRG10,
        img::fourcc::BGGR10,

        img::fourcc::MONO10_SPACKED,
        img::fourcc::GRBG10_SPACKED,
        img::fourcc::RGGB10_SPACKED,
        img::fourcc::GBRG10_SPACKED,
        img::fourcc::BGGR10_SPACKED,

        img::fourcc::MONO10_MIPI_PACKED,
        img::fourcc::GRBG10_MIPI_PACKED,
        img::fourcc::RGGB10_MIPI_PACKED,
        img::fourcc::GBRG10_MIPI_PACKED,
        img::fourcc::BGGR10_MIPI_PACKED,

        img::fourcc::MONO12,
        img::fourcc::GRBG12,
        img::fourcc::RGGB12,
        img::fourcc::GBRG12,
        img::fourcc::BGGR12,

        img::fourcc::MONO12_PACKED,
        img::fourcc::GRBG12_PACKED,
        img::fourcc::RGGB12_PACKED,
        img::fourcc::GBRG12_PACKED,
        img::fourcc::BGGR12_PACKED,

        img::fourcc::MONO12_SPACKED,
        img::fourcc::GRBG12_SPACKED,
        img::fourcc::RGGB12_SPACKED,
        img::fourcc::GBRG12_SPACKED,
        img::fourcc::BGGR12_SPACKED,

        img::fourcc::MONO12_MIPI_PACKED,
        img::fourcc::GRBG12_MIPI_PACKED,
        img::fourcc::RGGB12_MIPI_PACKED,
        img::fourcc::GBRG12_MIPI_PACKED,
        img::fourcc::BGGR12_MIPI_PACKED,
    };
}

static std::vector<img::fourcc>   tcamconvert_get_all_output_fccs()
{
    return {
        img::fourcc::GRBG8, img::fourcc::RGGB8, img::fourcc::GBRG8, img::fourcc::BGGR8,
        img::fourcc::GRBG16, img::fourcc::RGGB16, img::fourcc::GBRG16, img::fourcc::BGGR16,
        img::fourcc::MONO8, img::fourcc::MONO16
    };
}

static std::vector<img::fourcc>   tcamconvert_get_supported_input_fccs( img::fourcc dst_fcc )
{
    switch( dst_fcc )
    {
    case img::fourcc::BGGR8:  return { img::fourcc::BGGR10_SPACKED, img::fourcc::BGGR10_MIPI_PACKED, img::fourcc::BGGR12_PACKED, img::fourcc::BGGR12_SPACKED, img::fourcc::BGGR12_MIPI_PACKED, };
    case img::fourcc::GBRG8:  return { img::fourcc::GBRG10_SPACKED, img::fourcc::GBRG10_MIPI_PACKED, img::fourcc::GBRG12_PACKED, img::fourcc::GBRG12_SPACKED, img::fourcc::GBRG12_MIPI_PACKED, };
    case img::fourcc::RGGB8:  return { img::fourcc::RGGB10_SPACKED, img::fourcc::RGGB10_MIPI_PACKED, img::fourcc::RGGB12_PACKED, img::fourcc::RGGB12_SPACKED, img::fourcc::RGGB12_MIPI_PACKED, };
    case img::fourcc::GRBG8:  return { img::fourcc::GRBG10_SPACKED, img::fourcc::GRBG10_MIPI_PACKED, img::fourcc::GRBG12_PACKED, img::fourcc::GRBG12_SPACKED, img::fourcc::GRBG12_MIPI_PACKED, };

    case img::fourcc::BGGR16: return { img::fourcc::BGGR10_SPACKED, img::fourcc::BGGR10_MIPI_PACKED, img::fourcc::BGGR12_PACKED, img::fourcc::BGGR12_SPACKED, img::fourcc::BGGR12_MIPI_PACKED, };
    case img::fourcc::GBRG16: return { img::fourcc::GBRG10_SPACKED, img::fourcc::GBRG10_MIPI_PACKED, img::fourcc::GBRG12_PACKED, img::fourcc::GBRG12_SPACKED, img::fourcc::GBRG12_MIPI_PACKED, };
    case img::fourcc::RGGB16: return { img::fourcc::RGGB10_SPACKED, img::fourcc::RGGB10_MIPI_PACKED, img::fourcc::RGGB12_PACKED, img::fourcc::RGGB12_SPACKED, img::fourcc::RGGB12_MIPI_PACKED, };
    case img::fourcc::GRBG16: return { img::fourcc::GRBG10_SPACKED, img::fourcc::GRBG10_MIPI_PACKED, img::fourcc::GRBG12_PACKED, img::fourcc::GRBG12_SPACKED, img::fourcc::GRBG12_MIPI_PACKED, };

    case img::fourcc::MONO8: return { img::fourcc::MONO10_MIPI_PACKED, img::fourcc::MONO12_PACKED, img::fourcc::MONO12_SPACKED, img::fourcc::MONO12_MIPI_PACKED, };
    case img::fourcc::MONO16: return { img::fourcc::MONO10_MIPI_PACKED, img::fourcc::MONO12_PACKED, img::fourcc::MONO12_SPACKED, img::fourcc::MONO12_MIPI_PACKED, };
    default:
        return {};
    }
}

static std::vector<img::fourcc>   tcamconvert_get_supported_output_fccs( img::fourcc src_fcc )
{
    switch( src_fcc )
    {
    case img::fourcc::MONO10:
    case img::fourcc::MONO10_MIPI_PACKED:
    case img::fourcc::MONO12:
    case img::fourcc::MONO12_PACKED:
    case img::fourcc::MONO12_SPACKED:
    case img::fourcc::MONO12_MIPI_PACKED:
        return { img::fourcc::MONO8, img::fourcc::MONO16 };
    default:
        break;
    }

    using namespace img::by_transform;
    auto pattern = img::by_transform::convert_bayer_fcc_to_pattern( src_fcc );
    switch( pattern )
    {
    case by_pattern::BG:    return { img::fourcc::BGGR8, img::fourcc::BGGR16 };
    case by_pattern::GB:    return { img::fourcc::GBRG8, img::fourcc::GBRG16 };
    case by_pattern::RG:    return { img::fourcc::RGGB8, img::fourcc::RGGB16 };
    case by_pattern::GR:    return { img::fourcc::GRBG8, img::fourcc::GRBG16 };
    default:
        return {};
    }
}

static void gst_tcamconvert_class_init (GstTCamConvertClass* klass)
{
    GObjectClass* gobject_class = (GObjectClass*) klass;
    GstElementClass* gstelement_class = (GstElementClass*) klass;

    gobject_class->set_property = gst_tcamconvert_set_property;
    gobject_class->get_property = gst_tcamconvert_get_property;
    gobject_class->finalize = gst_tcamconvert_finalize;


    gst_element_class_set_static_metadata(gstelement_class,
                                          "BY10/12/16 -> BY8 or BY16",
                                          "Filter/Converter/Video",
                                          "Converts Bayer 10/12/16 to Bayer8 or Bayer16 images",
                                          "The Imaging Source <support@theimagingsource.com>");


    GstCaps* src_caps = generate_caps_struct( tcamconvert_get_all_output_fccs() );


    gst_element_class_add_pad_template(gstelement_class,
                                       gst_pad_template_new("src",
                                                             GST_PAD_SRC, GST_PAD_ALWAYS,
                                                             src_caps ));

    
    GstCaps* sink_caps = generate_caps_struct( tcamconvert_get_all_input_fccs() );

    gst_element_class_add_pad_template(gstelement_class,
                                       gst_pad_template_new("sink",
                                                            GST_PAD_SINK, GST_PAD_ALWAYS,
                                                            sink_caps));

    GST_BASE_TRANSFORM_CLASS(klass)->transform_size = GST_DEBUG_FUNCPTR(gst_tcamconvert_transform_size);
    GST_BASE_TRANSFORM_CLASS(klass)->transform_caps = GST_DEBUG_FUNCPTR(gst_tcamconvert_transform_caps);
    GST_BASE_TRANSFORM_CLASS(klass)->get_unit_size = GST_DEBUG_FUNCPTR(gst_tcamconvert_get_unit_size);
    GST_BASE_TRANSFORM_CLASS(klass)->set_caps = GST_DEBUG_FUNCPTR(gst_tcamconvert_set_caps);
    GST_BASE_TRANSFORM_CLASS(klass)->transform = GST_DEBUG_FUNCPTR(gst_tcamconvert_transform);

    GST_DEBUG_CATEGORY_INIT(gst_tcamconvert_debug_category,
                            "tcamconvert", 0,
                            "tcamconvert element");
}

static void gst_tcamconvert_init( GstTCamConvert* self )
{
    //self->src_type = {};
    //self->dst_type = {};

    gst_base_transform_set_in_place( GST_BASE_TRANSFORM( self ), FALSE );
}

/* No properties are implemented, so only a warning is produced */
static void gst_tcamconvert_set_property (GObject* object __attribute__((unused)),
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

static void gst_tcamconvert_get_property (GObject* object __attribute__((unused)),
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



static img::img_type caps_to_img_type( const GstCaps* caps )
{
    GstStructure* structure = gst_caps_get_structure( caps, 0 );

    auto dim_opt = gst_helper::get_gst_struct_image_dim( structure );
    if( !dim_opt ) {
        return {};
    }
    const auto fcc = gst_helper::get_gst_struct_fcc( structure );
    if( fcc == img::fourcc::FCC_NULL ) {
        return {};
    }
    return img::make_img_type( fcc, dim_opt.value() );
}

static gboolean gst_tcamconvert_set_caps( GstBaseTransform* base, GstCaps* incaps, GstCaps* outcaps )
{
    GstTCamConvert* self = GST_TCAMCONVERT( base );
    if( !self ) {
        return FALSE;
    }

    auto src = caps_to_img_type(incaps);
    auto dst = caps_to_img_type(outcaps);

    if (src.type == 0)
    {
        return FALSE;
    }
    if (dst.type == 0)
    {
        return FALSE;
    }

    self->dst_type = dst;
    self->src_type = src;

    return TRUE;
}

static void gst_tcamconvert_finalize( GObject* object )
{
    G_OBJECT_CLASS( gst_tcamconvert_parent_class )->finalize( object );
}

static void create_fmt( GstCaps* res_caps, const GstStructure* structure, const std::string& fmt, GstPadDirection direction )
{
    const GValue* w = gst_structure_get_value( structure, "width" );
    const GValue* h = gst_structure_get_value( structure, "height" );
    const GValue* frt = gst_structure_get_value( structure, "framerate" );

    const img::fourcc fourcc = img_lib::gst::gst_caps_string_to_fourcc( gst_structure_get_name( structure ), fmt );

    std::vector<img::fourcc> vec;
    if( direction == GST_PAD_SRC )
    {
        vec = tcamconvert_get_supported_input_fccs( fourcc );
    }
    else
    {
        vec = tcamconvert_get_supported_output_fccs( fourcc );
    }

    for( const auto& fcc : vec )
    {
        std::string cur_caps_str = img_lib::gst::fourcc_to_gst_caps_string( fcc );
        if( cur_caps_str.empty() )
        {
            continue;
        }
        GstCaps* caps_to_add = gst_caps_from_string( cur_caps_str.c_str() );
        if( caps_to_add == nullptr ) {
            continue;
        }

        if( w && h )
        {
            // NOTE: we copy this to encompass value ranges

            GValue width = G_VALUE_INIT;
            GValue height = G_VALUE_INIT;
            g_value_init( &width, G_VALUE_TYPE( w ) );
            g_value_init( &height, G_VALUE_TYPE( h ) );

            g_value_copy( w, &width );
            g_value_copy( h, &height );

            gst_caps_set_value( caps_to_add, "width", &width );
            gst_caps_set_value( caps_to_add, "height", &height );

            g_value_unset( &width );
            g_value_unset( &height );
        }
        if( frt ) {
            gst_caps_set_value( caps_to_add, "framerate", frt );
        }
        gst_caps_append( res_caps, caps_to_add );
    }
}

static GstCaps* transform_caps( GstCaps* caps, GstPadDirection direction )
{
    GstCaps* res_caps = gst_caps_new_empty();

    unsigned int caps_count = gst_caps_get_size( caps );
    for( unsigned int i = 0; i < caps_count; i++ )
    {
        GstStructure* structure = gst_caps_get_structure( caps, i );

        std::vector<std::string> fmt_vec;

        if( gst_structure_get_field_type( structure, "format" ) == GST_TYPE_LIST )
        {
            fmt_vec = gst_helper::gst_string_list_to_vector( gst_structure_get_value( structure, "format" ) );
        }
        else if( gst_structure_get_field_type( structure, "format" ) == G_TYPE_STRING )
        {
            fmt_vec.push_back( gst_structure_get_string( structure, "format" ) );
        }

        if( fmt_vec.empty() )
        {
            GST_ERROR( "No format given." );
            continue;
        }

        // for every entry in fmt_vec create a GstCaps that is appended to res_caps
        for( auto&& fcc_string : fmt_vec )
        {
            create_fmt( res_caps, structure, fcc_string, direction );
        }
    }

    res_caps = gst_caps_simplify( res_caps );
    
    if( direction == GST_PAD_SRC ) {
        GST_DEBUG( "Returning INPUT: %s", gst_helper::caps_to_string( res_caps ).c_str() );
    } else {
        GST_DEBUG( "Returning OUTPUT: %s", gst_helper::caps_to_string( res_caps ).c_str() );
    }
    return res_caps;
}

static GstCaps* gst_tcamconvert_transform_caps( GstBaseTransform* /*base*/, GstPadDirection direction, GstCaps* caps, GstCaps* filter )
{
    auto dir_to_string = []( GstPadDirection dir )
    {
        return dir == GST_PAD_SRC ? "GST_PAD_SRC" : "GST_PAD_SINK";
    };

    GstCaps* res_caps = transform_caps( caps, direction );

    if( filter )
    {
        GstCaps* tmp_caps = res_caps;
        res_caps = gst_caps_intersect_full( filter,
            tmp_caps,
            GST_CAPS_INTERSECT_FIRST );
        gst_caps_unref( tmp_caps );
    }

    GST_DEBUG( "dir=%s transformed %s into %s", dir_to_string( direction ),
               gst_helper::caps_to_string( caps ).c_str(),
               gst_helper::caps_to_string( res_caps ).c_str() );

    return res_caps;
}


static gboolean gst_tcamconvert_get_unit_size( GstBaseTransform* trans, GstCaps* caps, gsize* size )
{
    GstStructure* structure = gst_caps_get_structure( caps, 0 );
    auto dim_opt = gst_helper::get_gst_struct_image_dim( structure );
    if( !dim_opt )
    {
        GST_ELEMENT_ERROR( trans, CORE, NEGOTIATION, (NULL), ("Incomplete caps, some required field missing") );
        return FALSE;
    }

    const auto fcc = gst_helper::get_gst_struct_fcc( structure );
    if( fcc == img::fourcc::FCC_NULL )
    {
        GST_ELEMENT_ERROR( trans, CORE, NEGOTIATION, (NULL), ("Incomplete caps, format missing or unknown") );
        return FALSE;
    }
    size_t img_size = img::calc_minimum_img_size( fcc, dim_opt.value() );
    if( img_size == 0 )
    {
        GST_ELEMENT_ERROR( trans, CORE, NEGOTIATION, (NULL), ("Incomplete caps, unable to compute image size from format and dim") );
        return FALSE;
    }

    *size = img_size;
    return TRUE;
}

static gboolean gst_tcamconvert_transform_size(GstBaseTransform* trans,
                              GstPadDirection /*direction*/,
                              GstCaps* /*caps*/, gsize /*size*/,
                              GstCaps* othercaps, gsize* othersize)
{
    return gst_tcamconvert_get_unit_size( trans, othercaps, othersize );
}

static int         get_mapped_stride( GstBuffer* buffer_ ) noexcept
{
    auto video_meta_ptr = reinterpret_cast<GstVideoMeta*>(gst_buffer_get_meta( buffer_, gst_video_meta_api_get_type() ));
    if( video_meta_ptr != nullptr && video_meta_ptr->stride[0] != 0 ) {
        return video_meta_ptr->stride[0];
    }
    return 0;
}


static GstFlowReturn gst_tcamconvert_transform (GstBaseTransform* base,
                                                 GstBuffer* inbuf,
                                                 GstBuffer* outbuf)
{
    auto self = GST_TCAMCONVERT(base);

    GstMapInfo map_in;
    if (!gst_buffer_map(inbuf, &map_in, GST_MAP_READ))
    {
        GST_ERROR("Input buffer could not be mapped");
        return GST_FLOW_OK;
    }

    GstMapInfo map_out;
    if( !gst_buffer_map( outbuf, &map_out, GST_MAP_WRITE ) || map_out.data == nullptr )
    {
        gst_buffer_unmap( inbuf, &map_in );

        GST_ERROR("Output buffer could not be mapped");
        return GST_FLOW_OK;
    }

    img::img_descriptor src = {};
    if( int gst_meta_stride = get_mapped_stride( inbuf ); gst_meta_stride ) {
        src = img::make_img_desc_raw( self->src_type, img::img_plane{ map_in.data, gst_meta_stride } );
    } else {
        src = img::make_img_desc_from_linear_memory( self->src_type, map_in.data );
    }
    auto dst = img::make_img_desc_from_linear_memory( self->dst_type, map_out.data );

    img_filter::transform_function_type func = nullptr;

#if defined DUTILS_ARCH_ARM
    func = img_filter::transform::fcc1x_packed::get_transform_fcc10or12_packed_to_fcc8_neon_v0( self->dst_type, self->src_type );
    if( !func ) {
        func = img_filter::transform::fcc1x_packed::get_transform_fcc10or12_packed_to_fcc16_neon_v0( self->dst_type, self->src_type );
    }
#else
    func = img_filter::transform::fcc1x_packed::get_transform_fcc10or12_packed_to_fcc8_ssse3(self->dst_type, self->src_type);
    if (!func)
    {
        func = img_filter::transform::fcc1x_packed::get_transform_fcc10or12_packed_to_fcc16_ssse3(self->dst_type, self->src_type);
    }
#endif
    if( !func ) {
        func = img_filter::transform::fcc1x_packed::get_transform_fcc10or12_packed_to_fcc8_c( self->dst_type, self->src_type );
    }
    if( !func ) {
        func = img_filter::transform::fcc1x_packed::get_transform_fcc10or12_packed_to_fcc16_c( self->dst_type, self->src_type );
    }

    if( func == nullptr )
    {
        GST_ERROR_OBJECT(  self, "Failed to find conversion from %s to %s", img::fcc_to_string( src.type ).c_str(), img::fcc_to_string( dst.type ).c_str() );
    }
    else
    {
        func( dst, src );
    }

    gst_buffer_unmap(outbuf, &map_out);
    gst_buffer_unmap(inbuf, &map_in);

    return GST_FLOW_OK;
}


static gboolean plugin_init (GstPlugin* plugin)
{
    return gst_element_register(plugin,
                                "tcamconvert", GST_RANK_NONE, GST_TYPE_TCAMCONVERT);
}

#ifndef VERSION
#define VERSION "0.0.1"
#endif
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
                  VERSION,
                  "Proprietary",
                  PACKAGE_NAME, GST_PACKAGE_ORIGIN)
