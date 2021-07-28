
#include <gst-helper/gvalue_helper.h>

#include <cassert>

std::vector<std::string> gst_helper::gst_string_list_to_vector( const GValue& gst_list_tmp )
{
	auto gst_list = &gst_list_tmp;

	if( !GST_VALUE_HOLDS_LIST( gst_list ) )
	{
		GST_ERROR( "Given GValue is not a list." );
		return {};
	}

	unsigned int gst_list_size = gst_value_list_get_size( gst_list );

	std::vector<std::string> ret;
	ret.reserve( gst_list_size );
	for( unsigned int i = 0; i < gst_list_size; ++i )
	{
		const GValue* val = gst_value_list_get_value( gst_list, i );
		if( G_VALUE_TYPE( val ) == G_TYPE_STRING )
		{
			ret.push_back( g_value_get_string( val ) );
		}
		else
		{
            assert( G_VALUE_TYPE( val ) == G_TYPE_STRING );
		}
	}
	return ret;
}

std::string gst_helper::gobject_get_string( gpointer obj, const char* property_name )
{
    gchar* tmp = nullptr;
    g_object_get( obj, property_name, &tmp, nullptr );
    if( tmp == nullptr ) {
        return {};
    }
    std::string rval = tmp;
    g_free( tmp );
    return rval;
}

GSList* gst_helper::gst_string_vector_to_GSList( const std::vector<std::string>& lst )
{
    GSList* ret = nullptr;
    for( const auto& v : lst )
    {
        ret = g_slist_append( ret, g_strdup( v.c_str() ) );
    }
    return ret;
}

char* gst_helper::g_strdup_string( std::string_view str ) noexcept
{
    if( str.size() == 0 )
    {
        return nullptr;
    }
    char* rval = (char*)g_malloc( str.size() + 1 );
    if( rval == nullptr )
    {
        return nullptr;
    }
    memcpy( rval, str.data(), str.size() );
    rval[str.size()] = '\0';
    return rval;
}

//
//GSList* gst_helper::gst_string_vector_to_GSList( const std::vector<std::string_view>& lst )
//{
//    GSList* ret = nullptr;
//    for( const auto& v : lst )
//    {
//        ret = g_slist_append( ret, g_strdup( v.c_str() ) );
//    }
//    return ret;
//}
