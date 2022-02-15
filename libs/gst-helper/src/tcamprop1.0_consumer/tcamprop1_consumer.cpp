
#include "tcamprop1.0_consumer/tcamprop1_consumer.h"

#include <gst-helper/gvalue_helper.h>   // convert_GSList_to_string_vector_consume
#include <tcamprop1.0_base/tcamprop_errors.h>

#include <tcam-property-1.0.h>

#include "consumer_prop_impl.h"

auto tcamprop1_consumer::convert_prop_type( tcamprop1::prop_type t ) -> TcamPropertyType
{
    switch( t )
    {
    case tcamprop1::prop_type::Boolean: return TCAM_PROPERTY_TYPE_BOOLEAN;
    case tcamprop1::prop_type::Integer: return TCAM_PROPERTY_TYPE_INTEGER;
    case tcamprop1::prop_type::Float: return TCAM_PROPERTY_TYPE_FLOAT;
    case tcamprop1::prop_type::Enumeration: return TCAM_PROPERTY_TYPE_ENUMERATION;
    case tcamprop1::prop_type::Command: return TCAM_PROPERTY_TYPE_COMMAND;
    case tcamprop1::prop_type::String: return TCAM_PROPERTY_TYPE_STRING;
    }
    return TCAM_PROPERTY_TYPE_INTEGER;
}

TcamPropertyProvider* tcamprop1_consumer::get_TcamPropertyProvider( _GstElement* elem ) noexcept {
    if( !TCAM_IS_PROPERTY_PROVIDER( elem ) ) {
        return nullptr;
    }
    return TCAM_PROPERTY_PROVIDER( elem );
}

bool tcamprop1_consumer::has_TcamPropertyProvider( _GstElement& elem ) noexcept
{
    return TCAM_IS_PROPERTY_PROVIDER( &elem ) != FALSE;
}

bool    tcamprop1_consumer::has_property_interface( TcamPropertyProvider* elem, const char* name )
{
    if( elem == nullptr ) {
        return false;
    }
    GError* err = nullptr;
    auto ptr = tcam_property_provider_get_tcam_property( elem, name, &err );
    if( err ) {
        g_error_free( err );
        return false;
    }
    g_object_unref( ptr );
    return true;
}

bool    tcamprop1_consumer::has_property_interface( TcamPropertyProvider* elem, const char* name, TcamPropertyType type )
{
    if( elem == nullptr ) {
        return false;
    }
    GError* err = nullptr;
    auto ptr = tcam_property_provider_get_tcam_property( elem, name, &err );
    if( err ) {
        g_error_free( err );
        return false;
    }

    bool rval = tcam_property_base_get_property_type( ptr ) == type;

    g_object_unref( ptr );
    return rval;
}

outcome::result<std::vector<std::string>> tcamprop1_consumer::get_property_names( TcamPropertyProvider* elem )
{
    if( elem == nullptr ) {
        return tcamprop1::status::parameter_null;
    }
    GError* err = nullptr;
    auto list = tcam_property_provider_get_tcam_property_names( elem, &err );
    if( err ) {
        return tcamprop1_consumer::impl::convert_GError_to_error_code_consumer( err );
    }
    return gvalue::convert_GSList_to_string_vector_consume( list );
}

auto tcamprop1_consumer::get_property_names_noerror( TcamPropertyProvider* elem ) -> std::vector<std::string>
{
    if( elem == nullptr ) {
        return {};
    }
    auto list = tcam_property_provider_get_tcam_property_names( elem, nullptr );
    return gvalue::convert_GSList_to_string_vector_consume( list );
}

outcome::result<gobject_helper::gobject_ptr<TcamPropertyBase>> tcamprop1_consumer::get_property_node( TcamPropertyProvider* elem, const char* name )
{
    if( elem == nullptr ) {
        return tcamprop1::status::parameter_null;
    }
    GError* err = nullptr;
    auto ptr = tcam_property_provider_get_tcam_property( elem, name, &err );
    if( err ) {
        return tcamprop1_consumer::impl::convert_GError_to_error_code_consumer( err );
    }
    return gobject_helper::make_wrap_ptr( ptr );
}

template<class TItf>
static auto to_derived_ptr( const gobject_helper::gobject_ptr<TcamPropertyBase>& base_ptr ) -> gobject_helper::gobject_ptr<TItf>
{
    if constexpr( std::is_same_v<TItf,TcamPropertyInteger> ) {
        return gobject_helper::make_addref_ptr( TCAM_PROPERTY_INTEGER( base_ptr.get() ) );
    }
    else if constexpr( std::is_same_v<TItf, TcamPropertyFloat> ) {
        return gobject_helper::make_addref_ptr( TCAM_PROPERTY_FLOAT( base_ptr.get() ) );
    }
    else if constexpr( std::is_same_v<TItf, TcamPropertyBoolean> ) {
        return gobject_helper::make_addref_ptr( TCAM_PROPERTY_BOOLEAN( base_ptr.get() ) );
    }
    else if constexpr( std::is_same_v<TItf, TcamPropertyEnumeration> ) {
        return gobject_helper::make_addref_ptr( TCAM_PROPERTY_ENUMERATION( base_ptr.get() ) );
    }
    else if constexpr( std::is_same_v<TItf, TcamPropertyCommand> ) {
        return gobject_helper::make_addref_ptr( TCAM_PROPERTY_COMMAND( base_ptr.get() ) );
    }
    else if constexpr (std::is_same_v<TItf, TcamPropertyString>) {
        return gobject_helper::make_addref_ptr(TCAM_PROPERTY_STRING(base_ptr.get()));
    }
    else
    {
        static_assert(!std::is_same_v<TItf, TcamPropertyInteger>, "Invalid type argument" );
        return nullptr;
    }
}

auto tcamprop1_consumer::get_property_interface( TcamPropertyProvider* elem, const char* name ) ->outcome::result<std::unique_ptr<tcamprop1::property_interface>>
{
    auto node_opt = get_property_node( elem, name );
    if( node_opt.has_error() ) {
        return node_opt.error();
    }
    gobject_helper::gobject_ptr<TcamPropertyBase> node_ptr = node_opt.value();

    switch( tcam_property_base_get_property_type( node_ptr.get() ) )
    {
    case TCAM_PROPERTY_TYPE_BOOLEAN:        return std::make_unique<impl::prop_consumer_boolean>( to_derived_ptr<TcamPropertyBoolean>( node_ptr ) );
    case TCAM_PROPERTY_TYPE_INTEGER:        return std::make_unique<impl::prop_consumer_integer>( to_derived_ptr<TcamPropertyInteger>( node_ptr ) );
    case TCAM_PROPERTY_TYPE_FLOAT:          return std::make_unique<impl::prop_consumer_float>( to_derived_ptr<TcamPropertyFloat>( node_ptr ) );
    case TCAM_PROPERTY_TYPE_ENUMERATION:    return std::make_unique<impl::prop_consumer_enumeration>( to_derived_ptr<TcamPropertyEnumeration>( node_ptr ) );
    case TCAM_PROPERTY_TYPE_COMMAND:        return std::make_unique<impl::prop_consumer_command>( to_derived_ptr<TcamPropertyCommand>( node_ptr ) );
    case TCAM_PROPERTY_TYPE_STRING:         return std::make_unique<impl::prop_consumer_string>( to_derived_ptr<TcamPropertyString>( node_ptr ) );
    }
    return tcamprop1::status::parameter_type_incompatible;
}
