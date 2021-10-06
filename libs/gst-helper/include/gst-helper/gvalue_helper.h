
#pragma once

#include <string_view>
#include <string>
#include <vector>

typedef struct _GSList GSList;

namespace gvalue
{
    /** Does the same as g_strdup.
     * @return [transfer:full] Must be freed via g_free
     */
    char*           g_strdup_string( std::string_view str ) noexcept;

    /**
     * Converts the passed in GSList to a std::vector<std::string> and frees the list and its elements.
     * The passed in list must contain char* elements allocated via g_alloc.
     * @param lst: If == nullptr a empty list is returned.
     */
    std::vector<std::string>         convert_GSList_to_string_vector_consume( GSList* lst );
}

