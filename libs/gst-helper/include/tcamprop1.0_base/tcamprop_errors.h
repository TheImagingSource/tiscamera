#pragma once

#include <system_error>

namespace tcamprop1
{
    enum class status : int
    {
        success = 0x0,	        // generic success

        unknown,

        property_is_not_implemented,
        property_is_not_available,
        property_is_locked,
        parameter_type_incompatible,
        parameter_out_ot_range,
        property_is_readonly,
        property_default_not_available,
        enumeration_property_list_error, // failed to match an enum_entry name to an property_range_enumeration entry

        device_not_opened,
        device_closed,
        
        parameter_null,
    };

    std::error_code         make_error_code( tcamprop1::status e );
    std::error_category&    error_category();

    const char* to_string( tcamprop1::status e );
}

namespace std
{
    template <>
    struct is_error_code_enum<tcamprop1::status> : true_type {};      // overload std::error_code::error_code( pi_mipi_cam::status v )
}
