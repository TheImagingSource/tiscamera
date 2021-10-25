
#include <gst-helper/gst_gvalue_helper.h>
#include <gst-helper/gvalue_helper.h>

#include <cassert>

std::vector<std::string> gst_helper::gst_string_list_to_vector( const GValue& gst_list )
{
    auto gval_list = gst_list_or_array_to_GValue_vector( gst_list );

	std::vector<std::string> ret;
	ret.reserve( gval_list.size() );
	for( auto&& gval : gval_list )
	{
		if( G_VALUE_TYPE( gval ) == G_TYPE_STRING )
		{
			ret.push_back( g_value_get_string( gval ) );
		}
		else
		{
            assert( G_VALUE_TYPE( gval ) == G_TYPE_STRING );
		}
	}
	return ret;
}


std::vector<const GValue*> gst_helper::gst_list_or_array_to_GValue_vector( const GValue& gst_list )
{
    auto gst_list_ptr = &gst_list;

    if( GST_VALUE_HOLDS_LIST( gst_list_ptr ) )
    {
        unsigned int gst_list_size = gst_value_list_get_size( gst_list_ptr );

        std::vector<const GValue*> ret;
        ret.reserve( gst_list_size );
        for( unsigned int i = 0; i < gst_list_size; ++i )
        {
            const GValue* val = gst_value_list_get_value( gst_list_ptr, i );
            assert( val != nullptr );
            if( val == nullptr ) {
                GST_ERROR( "List entry at index %u is a nullptr", i );
                continue;
            }
            ret.push_back( val );
        }
        return ret;
    }
    else if( GST_VALUE_HOLDS_ARRAY( gst_list_ptr ) )
    {
        unsigned int gst_list_size = gst_value_array_get_size( gst_list_ptr );

        std::vector<const GValue*> ret;
        ret.reserve( gst_list_size );
        for( unsigned int i = 0; i < gst_list_size; ++i )
        {
            const GValue* val = gst_value_array_get_value( gst_list_ptr, i );
            assert( val != nullptr );
            if( val == nullptr ) {
                GST_ERROR( "List entry at index %u is a nullptr", i );
                continue;
            }
            ret.push_back( val );
        }
        return ret;
    }
    
    GST_ERROR( "Failed to find array or list in passed in GValue" );
    return {};
}

std::vector<std::string>    gst_helper::convert_GSList_to_string_vector_consume( GSList* lst )
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

std::optional<std::string> gst_helper::gobject_get_string_opt(gpointer obj,
                                                              const char* property_name)
{
    if( !gobject_has_property( obj, property_name, G_TYPE_STRING ) ) {
        return {};
    }
    return gobject_get_string( obj, property_name );
}

bool gst_helper::gobject_has_property(gpointer obj, const char* property_name, GType type /*= G_TYPE_NONE*/ )
{
    auto param_desc = g_object_class_find_property( G_OBJECT_GET_CLASS( obj ), property_name );
    if( param_desc == nullptr ) {
        return false;
    }
    if (type != G_TYPE_NONE)
    {
        return param_desc->value_type == type;
    }
    return true;
}

GSList* gst_helper::gst_string_vector_to_GSList( const std::vector<std::string>& lst )
{
    GSList* ret = nullptr;
    for( const auto& v : lst )
    {
        ret = g_slist_append( ret, gvalue::g_strdup_string( v ) );
    }
    return ret;
}
