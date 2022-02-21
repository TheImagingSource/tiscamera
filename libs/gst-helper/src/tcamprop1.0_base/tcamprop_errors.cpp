
#include <tcamprop1.0_base/tcamprop_errors.h>

namespace
{
    struct status_code_map_entry
    {
        const char* desc = nullptr;
        std::errc   default_map = static_cast<std::errc>( 0 );
    };

    constexpr status_code_map_entry to_entry( int ev ) noexcept
    {
        using namespace tcamprop1;

        auto code = static_cast<tcamprop1::status>(ev);
        switch( code )
        {
        case status::success:                                   return { "Success" };
        case status::unknown:                                   return { "Unknown", std ::errc::not_supported };
        case status::property_is_not_implemented:               return { "Property is not implemented", std::errc::no_such_file_or_directory };
        case status::property_is_not_available:                 return { "Property is not available", std::errc::no_such_file_or_directory };
        case status::property_is_locked:                        return { "Property is locked", std::errc::no_such_file_or_directory };
        case status::parameter_type_incompatible:               return { "Parameter type incompatible", std::errc::invalid_argument };
        case status::parameter_out_ot_range:                    return { "Parameter out of range", std::errc::invalid_argument };
        case status::device_closed:                             return { "Device closed", std::errc::connection_reset };
        case status::device_not_opened:                         return { "Device not opened", std::errc::no_such_device };
        case status::parameter_null:                            return { "Passed pointer is null", std::errc::no_such_device };
        case status::property_is_readonly:                      return { "Property is read-only", std::errc::invalid_argument };
        case status::property_default_not_available:            return { "Property-default is not available", std::errc::invalid_argument };
        case status::enumeration_property_list_error:           return { "Failed to find a enumeration entry in internal range", std::errc::protocol_error };
        }
        return {};
    }

    class tcamprop1_error_category : public std::error_category
    {
    public:
        const char* name() const noexcept final
        {
            return "tcamprop1.0_helper Error";
        }

        std::string message( int err ) const final
        {
            auto str = to_entry( err ).desc;
            if( str == nullptr ) {
                return "Unknown Error";
            }
            return str;
        }

        std::error_condition    default_error_condition( int err ) const noexcept final
        {
            if( err == 0 ) {    // 0 ^= success so use the default ctor of error_condition to build a efficient success value
                return {};
            }
            auto str = to_entry( err );
            if( str.desc == nullptr ) { // when desc is empty, we just copy our category into the condition
                return std::error_condition( err, *this );
            }
            return str.default_map;     // if we have a map, use that
        }
    };

    static tcamprop1_error_category    error_cat_;
}


std::error_category& tcamprop1::error_category()
{
    return error_cat_;
}

const char* tcamprop1::to_string( tcamprop1::status e )
{
    auto tmp = to_entry( static_cast<int>( e ) ).desc;
    if( tmp == nullptr ) {
        return "Unknown Error";
    }
    return tmp;
}

std::error_code     tcamprop1::make_error_code( tcamprop1::status e )
{
    return { static_cast<int>(e), error_cat_ };
}
