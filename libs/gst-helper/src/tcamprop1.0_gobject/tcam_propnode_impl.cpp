
#include "tcam_propnode_impl.h"

#include <tcamprop1.0_gobject/tcam_gerror.h>

void tcamprop1_gobj::impl::fill_GError( GError** gerr_loc, tcamprop1::status errc )
{
    tcamprop1_gobj::set_gerror( gerr_loc, errc );
}

void tcamprop1_gobj::impl::fill_GError( const std::error_code& errc, GError** gerr_loc )
{
    tcamprop1_gobj::set_gerror( gerr_loc, errc );
}

void tcamprop1_gobj::impl::fill_GError_device_lost( GError** gerr_loc )
{
    tcamprop1_gobj::set_gerror( gerr_loc, TCAM_ERROR_DEVICE_LOST, "device-closed" );
}

std::string tcamprop1_gobj::impl::number_to_hexstr( uint64_t w )
{
    size_t hex_len = 16;
    static const char* digits = "0123456789ABCDEF";
    std::string rc( hex_len, '0' );
    for( size_t i = 0, j = (hex_len - 1) * 4; i < hex_len; ++i, j -= 4 ) {
        rc[i] = digits[(w >> j) & 0x0f];
    }
    return rc;
}

std::string tcamprop1_gobj::impl::make_module_unique_name( std::string_view base, gpointer module_static_pointer )
{
    return std::string( base ) + "_" + number_to_hexstr( reinterpret_cast<uint64_t>(module_static_pointer) );
}
