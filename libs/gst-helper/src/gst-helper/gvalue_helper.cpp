
#include <gst-helper/gvalue_helper.h>

#include <gst/gst.h>

#include <cassert>

char* gvalue::g_strdup_string( std::string_view str ) noexcept
{
    if( str.empty() )
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

std::vector<std::string> gvalue::convert_GSList_to_string_vector_consume( GSList* lst )
{
    if( lst == nullptr ) {
        return {};
    }
    std::vector<std::string> rval;

    for( auto ptr = lst; ptr != nullptr; ptr = ptr->next )
    {
        rval.push_back( static_cast<const char*>(ptr->data) );
    }

    g_slist_free_full( lst, g_free );

    return rval;
}
