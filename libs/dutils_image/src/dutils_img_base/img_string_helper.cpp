

#include <dutils_img_lib/img_string_helper.h>
#include <dutils_img/fcc_to_string.h>

#include <cstring>

std::string     img_lib::to_string( const img::img_type& type )
{
    char buf[128] = {};
    sprintf( buf, "[%s] (%d/%d)", img::fcc_to_string( type.fourcc_type() ).c_str(), type.dim.cx, type.dim.cy );
    return buf;
}

std::string     img_lib::to_string( const img::img_descriptor& cfg )
{
    auto type = cfg.to_img_type();
    return to_string( type ) + " [" + std::to_string( cfg.data_length ) + "]";
}

