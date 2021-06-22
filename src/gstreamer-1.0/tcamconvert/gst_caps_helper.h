#pragma once

#include <dutils_img/dutils_img.h>
#include <gst/gst.h>
#include <optional>
#include <memory>

namespace gst_helper
{
    std::optional<img::dim> get_gst_struct_image_dim( const GstStructure* structure );
    img::fourcc                     get_gst_struct_fcc( const GstStructure* structure );

    std::optional<img::img_type>    get_gst_struct_image_type( const GstStructure* structure );
    std::optional<double>           get_gst_struct_framerate( const GstStructure* structure );

    inline void    set_gst_struct_framerate( GstStructure* structure, double framerate ) noexcept
    {
        GValue value = G_VALUE_INIT;
        g_value_init( &value, GST_TYPE_FRACTION );

        int fps_min_num = 0;
        int fps_min_den = 0;
        gst_util_double_to_fraction( framerate, &fps_min_num, &fps_min_den );
        gst_value_set_fraction( &value, fps_min_num, fps_min_den );
        gst_structure_set_value( structure, "framerate", &value );
        g_value_unset( &value );
    }

    inline std::string      caps_to_string( const GstCaps* structure )
    {
        gchar* tmp = ::gst_caps_to_string( structure );
        if( tmp == nullptr ) {
            return {};
        }
        std::string rval = tmp;
        g_free( tmp );
        return rval;
    }
}

