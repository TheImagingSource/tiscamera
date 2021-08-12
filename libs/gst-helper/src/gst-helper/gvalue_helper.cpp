
#include <gst-helper/gvalue_helper.h>

#include <gst/gst.h>

#include <cassert>

char* gvalue::g_strdup_string( std::string_view str ) noexcept
{
    if( str.size() == 0 )
    {
        return nullptr;
    }
    char* rval =static_cast<char*>(g_malloc( str.size() + 1 ));
    if( rval == nullptr )
    {
        return nullptr;
    }
    memcpy( rval, str.data(), str.size() );
    rval[str.size()] = '\0';
    return rval;
}
