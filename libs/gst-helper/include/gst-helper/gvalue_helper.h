
#pragma once

#include <string_view>

namespace gvalue
{
    /** Does the same as g_strdup.
     * @return [transfer:full] Must be freed via g_free
     */
    char*           g_strdup_string( std::string_view str ) noexcept;
}

