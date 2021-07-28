
#pragma once

#include <gst/gst.h>
#include <vector>
#include <string>
#include <string_view>

namespace gst_helper
{
	std::vector<std::string> gst_string_list_to_vector( const GValue& gst_list );

    std::string     gobject_get_string( gpointer obj, const char* property_name );
    /** Does the same as g_strdup.
     * @return [transfer:full] Must be freed via g_free
     */
    char*           g_strdup_string( std::string_view str ) noexcept;

    /**
     * Returns a GSList of 'char*' objects.
     * Note: The returned list must be deleted via g_slist_free or its elements deleted via g_slist_remove
     * Note: The contents is a char* pointer which must be deleted via g_free
     */
    GSList* gst_string_vector_to_GSList( const std::vector<std::string>& lst );
    //GSList* gst_string_vector_to_GSList( const std::vector<std::string_view>& lst );

}

