
#include "consumer_prop_impl.h"

#include <gst-helper/gvalue_helper.h>
#include <tcamprop1.0_base/tcamprop_errors.h>

namespace
{
    constexpr auto transform_TcamPropertyEnum( TcamPropertyIntRepresentation val ) noexcept
    {
        switch( val )
        {
        case TCAM_PROPERTY_INTREPRESENTATION_LINEAR:        return tcamprop1::IntRepresentation_t::Linear;
        case TCAM_PROPERTY_INTREPRESENTATION_LOGARITHMIC:   return tcamprop1::IntRepresentation_t::Logarithmic;
        case TCAM_PROPERTY_INTREPRESENTATION_PURENUMBER:    return tcamprop1::IntRepresentation_t::PureNumber;
        case TCAM_PROPERTY_INTREPRESENTATION_HEXNUMBER:     return tcamprop1::IntRepresentation_t::HexNumber;
        }
        return tcamprop1::IntRepresentation_t::Linear;
    }
    constexpr auto transform_TcamPropertyEnum( TcamPropertyVisibility val ) noexcept
    {
        switch( val )
        {
        case TCAM_PROPERTY_VISIBILITY_BEGINNER:     return tcamprop1::Visibility_t::Beginner;
        case TCAM_PROPERTY_VISIBILITY_EXPERT:       return tcamprop1::Visibility_t::Expert;
        case TCAM_PROPERTY_VISIBILITY_GURU:         return tcamprop1::Visibility_t::Guru;
        case TCAM_PROPERTY_VISIBILITY_INVISIBLE:    return tcamprop1::Visibility_t::Invisible;
        }
        return tcamprop1::Visibility_t::Invisible;
    }
    constexpr auto transform_TcamPropertyEnum( TcamPropertyFloatRepresentation val ) noexcept
    {
        switch( val )
        {
        case TCAM_PROPERTY_FLOATREPRESENTATION_LINEAR:      return tcamprop1::FloatRepresentation_t::Linear;
        case TCAM_PROPERTY_FLOATREPRESENTATION_LOGARITHMIC: return tcamprop1::FloatRepresentation_t::Logarithmic;
        case TCAM_PROPERTY_FLOATREPRESENTATION_PURENUMBER:  return tcamprop1::FloatRepresentation_t::PureNumber;
        }
        return tcamprop1::FloatRepresentation_t::Linear;
    }
}

auto tcamprop1_consumer::impl::convert_GError_to_error_code_consumer( GError* err ) -> std::error_code
{
    if( err == nullptr ) {
        return {};
    }
    if( err->domain != tcam_error_quark() ) {
        g_error_free( err );
        return std::make_error_code( std::errc::protocol_error );
    }

    auto code = static_cast<TcamError>(err->code);
    //std::string msg = err->message;

    g_error_free( err );

    switch( code )
    {
    case TCAM_ERROR_SUCCESS: return tcamprop1::status::success;
    case TCAM_ERROR_UNKNOWN: return tcamprop1::status::unknown;
    case TCAM_ERROR_TIMEOUT: return tcamprop1::status::unknown;
    case TCAM_ERROR_NOT_IMPLEMENTED: return tcamprop1::status::property_is_not_implemented;

    case TCAM_ERROR_PARAMETER_INVALID: return tcamprop1::status::parameter_null;

    case TCAM_ERROR_PROPERTY_NOT_IMPLEMENTED: return tcamprop1::status::property_is_not_implemented;
    case TCAM_ERROR_PROPERTY_NOT_AVAILABLE: return tcamprop1::status::property_is_not_available;
    case TCAM_ERROR_PROPERTY_NOT_WRITEABLE: return tcamprop1::status::property_is_locked;
    case TCAM_ERROR_PROPERTY_TYPE_INCOMPATIBLE: return tcamprop1::status::parameter_type_incompatible;
    case TCAM_ERROR_PROPERTY_VALUE_OUT_OF_RANGE: return tcamprop1::status::parameter_out_ot_range;
    case TCAM_ERROR_PROPERTY_DEFAULT_NOT_AVAILABLE: return tcamprop1::status::property_default_not_available;

    case TCAM_ERROR_DEVICE_NOT_OPENED: return tcamprop1::status::device_not_opened;
    case TCAM_ERROR_DEVICE_LOST: return tcamprop1::status::device_closed;
    case TCAM_ERROR_DEVICE_NOT_ACCESSIBLE: return tcamprop1::status::device_closed;
    }
    return tcamprop1::status::unknown;
}

auto tcamprop1_consumer::impl::fetch_prop_static_info_str( TcamPropertyBase* node ) -> tcamprop1::prop_static_info_str
{
    tcamprop1::prop_static_info_str rval;
    rval.name = tcam_property_base_get_name( node );
    rval.display_name = tcam_property_base_get_display_name( node );
    rval.description = tcam_property_base_get_description( node );
    rval.iccategory = tcam_property_base_get_category( node );
    rval.visibility = transform_TcamPropertyEnum( tcam_property_base_get_visibility( node ) );
    return rval;
}

auto tcamprop1_consumer::impl::fetch_prop_state( TcamPropertyBase* prop_node ) ->outcome::result<tcamprop1::prop_state>
{
    tcamprop1::prop_state rval;
    GError* err = nullptr;
    rval.is_available = tcam_property_base_is_available( prop_node, &err ) != FALSE;
    if( err )
    {
        return convert_GError_to_error_code_consumer( err );
    }
    rval.is_locked = tcam_property_base_is_locked( prop_node, &err ) != FALSE;
    if( err )
    {
        return convert_GError_to_error_code_consumer( err );
    }
    return rval;
}

tcamprop1_consumer::impl::prop_consumer_boolean::prop_consumer_boolean( gobject_helper::gobject_ptr<TcamPropertyBoolean>&& ptr )
    : ptr_( std::move( ptr ) )
{
    init( get_derived_node() );
}

auto tcamprop1_consumer::impl::prop_consumer_boolean::get_property_default( uint32_t /*flags*/ ) -> outcome::result<bool>
{
    GError* err = nullptr;
    auto rval = tcam_property_boolean_get_default( ptr_.get(), &err );
    if( err )
    {
        return convert_GError_to_error_code_consumer( err );
    }
    return bool{ rval != FALSE };
}

auto tcamprop1_consumer::impl::prop_consumer_boolean::get_property_value( [[maybe_unused]] uint32_t flags ) ->outcome::result<bool>
{
    GError* err = nullptr;
    auto rval = tcam_property_boolean_get_value( ptr_.get(), &err );
    if( err )
    {
        return convert_GError_to_error_code_consumer( err );
    }
    return rval != FALSE;
}

auto tcamprop1_consumer::impl::prop_consumer_boolean::set_property_value( bool value, [[maybe_unused]] uint32_t flags ) -> std::error_code
{
    GError* err = nullptr;
    tcam_property_boolean_set_value( ptr_.get(), value, &err );
    if( err )
    {
        return convert_GError_to_error_code_consumer( err );
    }
    return {};
}

tcamprop1_consumer::impl::prop_consumer_integer::prop_consumer_integer( gobject_helper::gobject_ptr<TcamPropertyInteger>&& ptr ) : ptr_( std::move( ptr ) )
{
    init( get_derived_node() );
}

auto tcamprop1_consumer::impl::prop_consumer_integer::get_property_range( [[maybe_unused]] uint32_t flags ) -> outcome::result<tcamprop1::prop_range_integer>
{
    tcamprop1::prop_range_integer rval = {};
    GError* err = nullptr;
    tcam_property_integer_get_range( ptr_.get(), &rval.min, &rval.max, &rval.stp, &err );
    if( err )
    {
        return convert_GError_to_error_code_consumer( err );
    }
    return rval;
}

auto tcamprop1_consumer::impl::prop_consumer_integer::get_property_default( [[maybe_unused]] uint32_t flags ) ->outcome::result<int64_t>
{
    GError* err = nullptr;
    auto rval = tcam_property_integer_get_default( ptr_.get(), &err );
    if( err )
    {
        return convert_GError_to_error_code_consumer( err );
    }
    return rval;
}

auto tcamprop1_consumer::impl::prop_consumer_integer::get_property_value( [[maybe_unused]] uint32_t flags ) ->outcome::result<int64_t>
{
    GError* err = nullptr;
    auto rval = tcam_property_integer_get_value( ptr_.get(), &err );
    if( err )
    {
        return convert_GError_to_error_code_consumer( err );
    }
    return rval;
}

auto tcamprop1_consumer::impl::prop_consumer_integer::set_property_value( int64_t value, [[maybe_unused]] uint32_t flags ) -> std::error_code
{
    GError* err = nullptr;
    tcam_property_integer_set_value( ptr_.get(), value, &err );
    if( err )
    {
        return convert_GError_to_error_code_consumer( err );
    }
    return {};
}

auto tcamprop1_consumer::impl::prop_consumer_integer::get_representation() const noexcept -> tcamprop1::IntRepresentation_t
{
    return transform_TcamPropertyEnum( tcam_property_integer_get_representation( ptr_.get() ) );
}

auto tcamprop1_consumer::impl::prop_consumer_integer::get_unit() const noexcept -> std::string_view
{
    return tcam_property_integer_get_unit( ptr_.get() );
}

tcamprop1_consumer::impl::prop_consumer_float::prop_consumer_float( gobject_helper::gobject_ptr<TcamPropertyFloat>&& ptr ) : ptr_( std::move( ptr ) )
{
    init( get_derived_node() );
}

auto tcamprop1_consumer::impl::prop_consumer_float::get_property_range( [[maybe_unused]] uint32_t flags ) -> outcome::result<tcamprop1::prop_range_float>
{
    tcamprop1::prop_range_float rval = {};
    GError* err = nullptr;
    tcam_property_float_get_range( ptr_.get(), &rval.min, &rval.max, &rval.stp, &err );
    if( err )
    {
        return convert_GError_to_error_code_consumer( err );
    }
    return rval;
}

auto tcamprop1_consumer::impl::prop_consumer_float::get_property_default( [[maybe_unused]] uint32_t flags ) ->outcome::result<double>
{
    GError* err = nullptr;
    auto rval = tcam_property_float_get_default( ptr_.get(), &err );
    if( err )
    {
        return convert_GError_to_error_code_consumer( err );
    }
    return rval;
}

auto tcamprop1_consumer::impl::prop_consumer_float::get_property_value( [[maybe_unused]] uint32_t flags ) ->outcome::result<double>
{
    GError* err = nullptr;
    auto rval = tcam_property_float_get_value( ptr_.get(), &err );
    if( err )
    {
        return convert_GError_to_error_code_consumer( err );
    }
    return rval;
}

auto tcamprop1_consumer::impl::prop_consumer_float::set_property_value( double value, [[maybe_unused]] uint32_t flags ) -> std::error_code
{
    GError* err = nullptr;
    tcam_property_float_set_value( ptr_.get(), value, &err );
    if( err )
    {
        return convert_GError_to_error_code_consumer( err );
    }
    return {};
}

auto tcamprop1_consumer::impl::prop_consumer_float::get_representation() const noexcept -> tcamprop1::FloatRepresentation_t
{
    return transform_TcamPropertyEnum( tcam_property_float_get_representation( ptr_.get() ) );
}

auto tcamprop1_consumer::impl::prop_consumer_float::get_unit() const noexcept -> std::string_view
{
    return tcam_property_float_get_unit( ptr_.get() );
}

tcamprop1_consumer::impl::prop_consumer_enumeration::prop_consumer_enumeration( gobject_helper::gobject_ptr<TcamPropertyEnumeration>&& ptr ) : ptr_( std::move( ptr ) )
{
    init( get_derived_node() );
}

auto tcamprop1_consumer::impl::prop_consumer_enumeration::get_property_range( [[maybe_unused]] uint32_t flags ) -> outcome::result<tcamprop1::prop_range_enumeration>
{
    GError* err = nullptr;
    auto entry_gslist = tcam_property_enumeration_get_enum_entries( ptr_.get(), &err );
    if( err )
    {
        return convert_GError_to_error_code_consumer( err );
    }
    return tcamprop1::prop_range_enumeration{ gvalue::convert_GSList_to_string_vector_consume( entry_gslist ) };
}

auto tcamprop1_consumer::impl::prop_consumer_enumeration::get_property_default( [[maybe_unused]] uint32_t flags ) ->outcome::result<std::string_view>
{
    GError* err = nullptr;
    auto rval = tcam_property_enumeration_get_default( ptr_.get(), &err );
    if( err )
    {
        return convert_GError_to_error_code_consumer( err );
    }
    return rval;
}

auto tcamprop1_consumer::impl::prop_consumer_enumeration::get_property_value( [[maybe_unused]] uint32_t flags ) ->outcome::result<std::string_view>
{
    GError* err = nullptr;
    auto rval = tcam_property_enumeration_get_value( ptr_.get(), &err );
    if( err )
    {
        return convert_GError_to_error_code_consumer( err );
    }
    return rval;
}

auto tcamprop1_consumer::impl::prop_consumer_enumeration::set_property_value( std::string_view value, [[maybe_unused]] uint32_t flags ) -> std::error_code
{
    std::string tmp{ value };
    GError* err = nullptr;
    tcam_property_enumeration_set_value( ptr_.get(), tmp.c_str(), &err );
    if( err )
    {
        return convert_GError_to_error_code_consumer( err );
    }
    return {};
}

tcamprop1_consumer::impl::prop_consumer_command::prop_consumer_command( gobject_helper::gobject_ptr<TcamPropertyCommand>&& ptr ) : ptr_( std::move( ptr ) )
{
    init( get_derived_node() );
}

auto tcamprop1_consumer::impl::prop_consumer_command::execute_command( [[maybe_unused]] uint32_t flags ) -> std::error_code
{
    GError* err = nullptr;
    tcam_property_command_set_command( ptr_.get(), &err );
    if( err )
    {
        return convert_GError_to_error_code_consumer( err );
    }
    return {};
}

tcamprop1_consumer::impl::prop_consumer_string::prop_consumer_string(
    gobject_helper::gobject_ptr<TcamPropertyString>&& ptr)
    : ptr_(std::move(ptr))
{
    init(get_derived_node());
}

auto tcamprop1_consumer::impl::prop_consumer_string::get_property_value( [[maybe_unused]] uint32_t flags /*= 0*/)
    -> outcome::result<std::string>
{
    GError* err = nullptr;
    auto str = tcam_property_string_get_value(ptr_.get(), &err);
    if (err)
    {
        return convert_GError_to_error_code_consumer(err);
    }
    if (str)
    {
        auto ret = std::string{str};
        g_free(str);
        return ret;
    }
    return std::string{};
}

auto tcamprop1_consumer::impl::prop_consumer_string::set_property_value(std::string_view new_value, [[maybe_unused]] uint32_t flags /*= 0*/) -> std::error_code
{
    std::string tmp { new_value };
    GError* err = nullptr;
    tcam_property_string_set_value(ptr_.get(), tmp.c_str(), &err);
    if (err)
    {
        return convert_GError_to_error_code_consumer(err);
    }
    return {};
}
