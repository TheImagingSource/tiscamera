
#include <gst-helper/gstcaps_dutils_interop.h>

#include <dutils_img_lib/dutils_gst_interop.h>
#include <gst-helper/gst_gvalue_helper.h>

#include <map>
#include <limits>
#include <algorithm>


std::optional<img::dim> gst_helper::get_gst_struct_image_dim(const GstStructure& structure)
{
    int width = 0;
    int height = 0;
    if (!gst_structure_get_int(&structure, "width", &width)
        || !gst_structure_get_int(&structure, "height", &height))
    {
        return {};
    }
    return img::dim { width, height };
}

img::fourcc gst_helper::get_gst_struct_fcc(const GstStructure& structure)
{
    const auto type = gst_structure_get_field_type(&structure, "format");

    assert(gst_structure_get_name(&structure) != nullptr);

    const char* format = nullptr;
    if (type == G_TYPE_STRING)
    {
        format = gst_structure_get_string(&structure, "format");
    }
    else if (type == GST_TYPE_LIST)
    {
        return img::fourcc::FCC_NULL;
    }
    else
    {
        return img::fourcc::FCC_NULL;
    }
    return img_lib::gst::gst_caps_string_to_fourcc(gst_structure_get_name(&structure),
                                                   format == nullptr ? "" : format);
}

img::img_type gst_helper::get_gst_struct_image_type(const GstStructure& structure)
{
    auto dim_opt = get_gst_struct_image_dim(structure);
    if (!dim_opt)
    {
        return {};
    }
    img::fourcc fcc = get_gst_struct_fcc(structure);
    if (fcc == img::fourcc::FCC_NULL)
    {
        return {};
    }
    return img::make_img_type(fcc, dim_opt.value());
}

auto gst_helper::get_img_type_from_fixated_gstcaps( const GstCaps& caps ) -> img::img_type
{
    auto struc = gst_caps_get_structure( &caps, 0 );
    if( struc == nullptr ) {
        return {};
    }
    return get_gst_struct_image_type( *struc );
}

std::optional<double> gst_helper::get_gst_struct_framerate(const GstStructure& structure)
{
    const GValue* frame_rate_field = gst_structure_get_value(&structure, "framerate");

    if (frame_rate_field == nullptr)
    {
        return {};
    }

    int fps_numerator = gst_value_get_fraction_numerator(frame_rate_field);
    int fps_denominator = gst_value_get_fraction_denominator(frame_rate_field);
    return (double)fps_numerator / (double)fps_denominator;
}

void gst_helper::set_gst_struct_framerate(GstStructure& structure, double framerate) noexcept
{
    GValue value = G_VALUE_INIT;
    g_value_init(&value, GST_TYPE_FRACTION);

    int fps_min_num = 0;
    int fps_min_den = 0;
    gst_util_double_to_fraction(framerate, &fps_min_num, &fps_min_den);
    gst_value_set_fraction(&value, fps_min_num, fps_min_den);
    gst_structure_take_value(&structure, "framerate", &value);
}

gst_helper::gst_ptr<GstCaps>    gst_helper::generate_caps_with_dim( const std::vector<img::fourcc>& fcc_list )
{
    GstCaps* caps = gst_caps_new_empty();

    std::map<std::string, std::vector<const char*>> simple_map;

    for( const auto fcc : fcc_list )
    {
        auto [prefix, format] = img_lib::gst::fourcc_to_gst_caps_descr( fcc );
        if( !prefix )
        {
            //GST_WARN( "Format has empty caps string. Ignoring %s", img::fcc_to_string( fcc ).c_str() );
            continue;
        }

        simple_map[prefix].push_back( format );
    }

    for( auto&& [struct_type, format_string_list] : simple_map )
    {
        GValue format_list = {};
        g_value_init( &format_list, GST_TYPE_LIST );
        for( auto&& format : format_string_list )
        {
            GValue tmp = {};
            g_value_init( &tmp, G_TYPE_STRING );
            g_value_set_string( &tmp, format );
            gst_value_list_append_and_take_value( &format_list, &tmp );
        }

        GstStructure* structure = gst_structure_new_empty( struct_type.c_str() );
        gst_structure_take_value( structure, "format", &format_list );

        GValue gvalue_width = G_VALUE_INIT;
        g_value_init( &gvalue_width, GST_TYPE_INT_RANGE );
        gst_value_set_int_range_step( &gvalue_width, 1, std::numeric_limits<gint>::max(), 1 );

        GValue gvalue_height = G_VALUE_INIT;
        g_value_init( &gvalue_height, GST_TYPE_INT_RANGE );
        gst_value_set_int_range_step( &gvalue_height, 1, std::numeric_limits<gint>::max(), 1 );

        gst_structure_take_value( structure, "width", &gvalue_width );
        gst_structure_take_value( structure, "height", &gvalue_height );
        //gst_structure_take_value( structure, "framerate", &gvalue_fps_range );  // takes ownership of gvalue_fps_range

        gst_caps_append_structure( caps, structure );
    }

    return gst_helper::make_wrap_ptr( caps );
}

auto gst_helper::convert_GstStructure_to_fcc_list( const GstStructure& strct ) -> std::vector<img::fourcc>
{
    auto struct_name_ = gst_structure_get_name( &strct );
    assert( struct_name_ != nullptr );
    if( struct_name_ == nullptr ) {
        GST_ERROR( "GstStructure with no name" );
        return {};
    }
    std::string_view format_type = struct_name_;


    auto gval = gst_structure_get_value( &strct, "format" );
    if( gval == nullptr ) {
        return {};
    }

    std::vector<img::fourcc> rval;
    auto add_fcc = [&rval,format_type]( const char* str ) {
        if( str != nullptr ) {
            auto fcc = img_lib::gst::gst_caps_string_to_fourcc( format_type, str );
            if( fcc != img::fourcc::FCC_NULL ) {
                rval.push_back( fcc );
            }
        }
    };

    if( G_VALUE_TYPE( gval ) == G_TYPE_STRING )
    {
        add_fcc( g_value_get_string( gval ) );
    }
    else if( G_VALUE_TYPE( gval ) == GST_TYPE_LIST || G_VALUE_TYPE( gval ) == GST_TYPE_ARRAY )
    {
        for( auto&& entry : gst_helper::gst_list_or_array_to_GValue_vector( *gval ) )
        {
            add_fcc( g_value_get_string( entry ) );
        }
    }
    else
    {
        return {};
    }
    return rval;
}

auto gst_helper::convert_GstCaps_to_fcc_list( const GstCaps& caps ) ->std::vector<img::fourcc>
{
    int struct_cnt = gst_caps_get_size( &caps );

    std::vector<img::fourcc> rval;
    for( int i = 0; i < struct_cnt; ++i ) {
        GstStructure* entry = gst_caps_get_structure( &caps, i );
        assert( entry != nullptr );
        if( entry == nullptr ) {
            return {};
        }
        for( auto&& new_fcc : convert_GstStructure_to_fcc_list( *entry ) )
        {
            const bool already_present = std::any_of( rval.begin(), rval.end(), [new_fcc]( auto fcc_in_rval ) { return fcc_in_rval == new_fcc; } );
            if( !already_present )
            {
                rval.push_back( new_fcc );
            }
        }
    }



    return rval;
}
