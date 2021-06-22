
#pragma once

#include <dutils_img/dutils_img.h>
#include <string>
#include <string_view>

namespace img_lib::gst
{
    std::string     fourcc_to_gst_caps_string( uint32_t fourcc );
    std::string     fourcc_to_gst_caps_string( img::fourcc fourcc );
    img::fourcc     gst_caps_string_to_fourcc( std::string_view format_type, std::string_view format_string );
}

