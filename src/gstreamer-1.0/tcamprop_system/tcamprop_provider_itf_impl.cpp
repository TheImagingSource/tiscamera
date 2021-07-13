
#include "tcamprop_provider_itf_impl.h"

#include <cassert>
#include <spdlog/fmt/fmt.h>

namespace
{
    using namespace tcamprop_system;

    static char* g_strdup_string_view( std::string_view str ) noexcept
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

    static void write_gvalue( GValue* dst, bool val )
    {
        if( !dst )
            return;

        g_value_init( dst, G_TYPE_BOOLEAN );
        g_value_set_boolean( dst, val ? TRUE : FALSE );
    }

    static void write_gvalue( GValue* dst, int val )
    {
        if( !dst )
            return;

        g_value_init( dst, G_TYPE_INT );
        g_value_set_int( dst, val );
    }

    static void write_gvalue( GValue* dst, int64_t val )
    {
        if( !dst )
            return;

        g_value_init( dst, G_TYPE_INT );
        g_value_set_int( dst, val );
    }

    static void write_gvalue( GValue* dst, double val )
    {
        if( !dst )
            return;

        g_value_init( dst, G_TYPE_DOUBLE );
        g_value_set_double( dst, val );
    }

    static void write_gvalue( GValue* dst, const char* val )
    {
        if( !dst )
            return;

        g_value_init( dst, G_TYPE_STRING );
        g_value_set_string( dst, val );
    }

    static void write_gvalue( GValue* dst, const std::string_view& val )
    {
        if( !dst )
            return;

        g_value_init( dst, G_TYPE_STRING );
        g_value_take_string( dst, g_strdup_string_view( val ) );
    }

    void    fwd_error_message( const report_error& report_err_func, tcamprop_system::error_id id, std::string_view dsc = {} )
    {
        if( report_err_func ) {
            report_err_func( id, dsc );
        }
    }
    void    fwd_error_message( const report_error& report_err_func, tcamprop_system::error_id id, const tcamprop_system::property_info& info, std::string_view dsc )
    {
        if( report_err_func ) {
            report_err_func( id, fmt::format( "Property '{}' ({}), {}.", info.prop_name, tcamprop_system::to_string( info.type ), dsc ) );
        }
    }
    void    fwd_error_message( const report_error& report_err_func, tcamprop_system::error_id id, const tcamprop_system::property_info& info, std::string_view dsc, std::error_code errc )
    {
        if( report_err_func ) {
            report_err_func( id, fmt::format( "Property '{}' ({}), {}. Error-message: '{}'", info.prop_name, tcamprop_system::to_string( info.type ), dsc, errc.message() ) );
        }
    }

} // namespace


GSList* tcamprop_system::tcamprop_impl_get_tcam_property_names( property_list_interface* prop_list_itf,
                                                       const report_error& report_err_func )
{
    if( prop_list_itf == nullptr ) {
        fwd_error_message( report_err_func, error_id::parameter_nullptr );
        return FALSE;
    }

    GSList* names = nullptr;
    for( const auto& prop_name : prop_list_itf->get_property_list() )
    {
        auto prop_itf = prop_list_itf->find_property( prop_name );
        if( prop_itf == nullptr )
        {
            assert( prop_itf != nullptr );
            continue;
        }

        auto prop = prop_itf->get_property_info();

        auto prop_flags_opt = prop_itf->get_property_flags();
        if( !prop_flags_opt.has_value() )
        {
            fwd_error_message( report_err_func, error_id::property_failed_get_flags, prop, "failed to get flags", prop_flags_opt.error() );
            continue;
        }
        auto flags = prop_flags_opt.value();
        if( !!(flags & tcamprop_system::prop_flags::hide_from_get_property_names) )
        {
            continue;
        }
        if( !(flags & tcamprop_system::prop_flags::implemented) )
        {
            continue;
        }

        names = g_slist_append( names, g_strdup_string_view( prop_name ) );
    }
    return names;
}

gboolean tcamprop_system::tcamprop_impl_get_tcam_property( property_list_interface* prop_list_itf, const gchar* name,
                                                           GValue* value,
                                                           GValue* min,
                                                           GValue* max,
                                                           GValue* def,
                                                           GValue* stp,
                                                           GValue* type,
                                                           GValue* flags,
                                                           GValue* category,
                                                           GValue* group,
                                                           const report_error& report_err_func )
{
    if( prop_list_itf == nullptr || name == nullptr ) {
        fwd_error_message( report_err_func, error_id::parameter_nullptr );
        return FALSE;
    }

    auto prop_itf = prop_list_itf->find_property( name );
    if( prop_itf == nullptr ) {
        fwd_error_message( report_err_func, error_id::property_not_found, fmt::format( "Failed to find property '{}'.", name ) );
        return FALSE;
    }

    auto prop = prop_itf->get_property_info();

    auto prop_flags_opt = prop_itf->get_property_flags();
    if( !prop_flags_opt )
    {
        fwd_error_message( report_err_func, error_id::property_failed_get_flags, prop, "failed to get flags", prop_flags_opt.error() );
        return FALSE;
    }

    auto prop_flags = prop_flags_opt.value();
    if( !(prop_flags & tcamprop_system::prop_flags::implemented) )
    {
        return FALSE;   // this is not an error
    }
    write_gvalue( flags, (int)(prop_flags) );
    write_gvalue( category, prop.prop_category );
    write_gvalue( group, prop.prop_group );
    write_gvalue( type, tcamprop_system::to_string( prop.type ) );

    if( prop.type == tcamprop_system::prop_type::button )
    {
        write_gvalue( value, false );
        write_gvalue( min, false );
        write_gvalue( max, true );
        write_gvalue( def, false );
        write_gvalue( stp, true );

        return TRUE;
    }
    else if( prop.type == tcamprop_system::prop_type::boolean )
    {
        if( value )
        {
            auto tmp = prop_itf->get_property_value();
            if( !tmp.has_value() ) {
                fwd_error_message( report_err_func, error_id::property_failed_get_value, prop, "failed to get value", tmp.error() );
                return FALSE;
            }
            write_gvalue( value, tmp.value().as_bool() );
        }
        write_gvalue( min, false );
        write_gvalue( max, true );
        if( def )
        {
            if( auto range_opt = prop_itf->get_property_range(); range_opt.has_error() )
            {
                fwd_error_message( report_err_func, error_id::property_failed_get_range, prop, "failed to get range", range_opt.error() );
                return FALSE;
            }
            else
            {
                write_gvalue( def, range_opt.value().val_def.as_bool() );
            }
        }
        write_gvalue( stp, true );
        return TRUE;
    }
    else if( prop.type == tcamprop_system::prop_type::integer )
    {
        if( value )
        {
            auto tmp = prop_itf->get_property_value();
            if( !tmp.has_value() ) {
                fwd_error_message( report_err_func, error_id::property_failed_get_value, prop, "failed to get value", tmp.error() );
                return FALSE;
            }
            write_gvalue( value, tmp.value().integer );
        }
        if( min || max || def || stp )
        {
            auto range_opt = prop_itf->get_property_range();
            if( range_opt.has_error() )
            {
                fwd_error_message( report_err_func, error_id::property_failed_get_range, prop, "failed to get range", range_opt.error() );
                return FALSE;
            }
            auto range = range_opt.value().to_integer();

            write_gvalue( min, range.min );
            write_gvalue( max, range.max );
            write_gvalue( def, range.def );
            write_gvalue( stp, range.stp );
        }
        return TRUE;
    }
    else if( prop.type == tcamprop_system::prop_type::real )
    {
        if( value )
        {
            auto tmp = prop_itf->get_property_value();
            if( !tmp.has_value() ) {
                fwd_error_message( report_err_func, error_id::property_failed_get_value, prop, "failed to get value", tmp.error() );
                return FALSE;
            }
            write_gvalue( value, tmp.value().real );
        }

        if( min || max || def || stp )
        {
            auto range_opt = prop_itf->get_property_range();
            if( range_opt.has_error() )
            {
                fwd_error_message( report_err_func, error_id::property_failed_get_range, prop, "failed to get range", range_opt.error() );
                return FALSE;
            }
            auto range = range_opt.value().to_real();

            write_gvalue( min, range.min );
            write_gvalue( max, range.max );
            write_gvalue( def, range.def );
            write_gvalue( stp, range.stp );
        }
        return TRUE;
    }
    else if( prop.type == tcamprop_system::prop_type::menu )
    {
        auto range_opt = prop_itf->get_property_range();
        if( range_opt.has_error() )
        {
            fwd_error_message( report_err_func, error_id::property_failed_get_range, prop, "failed to get range", range_opt.error() );
            return FALSE;
        }

        auto range = range_opt.value();

        auto find_string = [&range]( int idx ) -> std::string_view {
            if( idx < 0 && idx >= (int)range.menu_entries.size() )
            {
                return {};
            }
            return range.menu_entries[idx];
        };
        if( value )
        {
            auto tmp = prop_itf->get_property_value();
            if( !tmp.has_value() ) {
                fwd_error_message( report_err_func, error_id::property_failed_get_value, prop, "failed to get value", tmp.error() );
                return FALSE;
            }
            write_gvalue( value, find_string( tmp.value().integer ) );
        }

        write_gvalue( min, "" );
        write_gvalue( max, "" );
        write_gvalue( def, find_string( range.val_def.integer ) );
        write_gvalue( stp, "" );
        return TRUE;
    }
    else
    {
        fwd_error_message( report_err_func, error_id::property_type_incompatible, prop, "Property has invalid type" );
        return FALSE;
    }
    return TRUE;
}

GSList* tcamprop_system::tcamprop_impl_get_menu_entries( property_list_interface* prop_list_itf, const gchar* name, const report_error& report_err_func )
{
    if( prop_list_itf == nullptr || name == nullptr ) {
        fwd_error_message( report_err_func, error_id::parameter_nullptr );
        return nullptr;
    }

    auto prop_itf = prop_list_itf->find_property( name );
    if( prop_itf == nullptr ) {
        fwd_error_message( report_err_func, error_id::property_not_found, fmt::format( "Failed to find property '{}'.", name ) );
        return nullptr;
    }

    auto desc = prop_itf->get_property_info();
    if( desc.type != tcamprop_system::prop_type::menu )
    {
        fwd_error_message( report_err_func, error_id::property_type_incompatible, desc, fmt::format( "expected type ({})", tcamprop_system::prop_type::menu ) );
        return nullptr;
    }
    auto range_opt = prop_itf->get_property_range();
    if( !range_opt )
    {
        fwd_error_message( report_err_func, error_id::property_type_incompatible, desc, "failed to fetch menu-items due to", range_opt.error() );
        return nullptr;
    }

    GSList* ret = nullptr;
    for( const auto& m : range_opt.value().menu_entries ) { ret = g_slist_append( ret, g_strdup_string_view( m ) ); }
    return ret;
}


gboolean tcamprop_system::tcamprop_impl_set_tcam_property( property_list_interface* prop_list_itf,
                                                  const gchar* name,
                                                  const GValue* value, const report_error& report_err_func )
{
    if( prop_list_itf == nullptr || name == nullptr || value == nullptr ) {
        fwd_error_message( report_err_func, error_id::parameter_nullptr );
        return FALSE;
    }
    auto prop_itf = prop_list_itf->find_property( name );
    if( prop_itf == nullptr ) {
        fwd_error_message( report_err_func, error_id::property_not_found, fmt::format( "Failed to find property '{}'.", name ) );
        return FALSE;
    }

    auto prop = prop_itf->get_property_info();

    switch( prop.type )
    {
    case tcamprop_system::prop_type::button:
    {
        auto err = prop_itf->set_property_value( tcamprop_system::prop_value{ true } );
        if( err )
        {
            fwd_error_message( report_err_func, error_id::set_function_failed, prop, fmt::format( "err='{}'", err.message() ) );
            return FALSE;
        }
        return TRUE;
    }
    case tcamprop_system::prop_type::boolean:
    {
        if( !G_VALUE_HOLDS_BOOLEAN( value ) )
        {
            fwd_error_message( report_err_func, error_id::property_type_incompatible, prop, fmt::format( "expected boolean, got {}", g_type_name( g_value_get_gtype( value ) ) ) );
            return FALSE;
        }

        auto val = g_value_get_boolean( value ) == TRUE;
        auto err = prop_itf->set_property_value( tcamprop_system::prop_value{ val } );
        if( err )
        {
            fwd_error_message( report_err_func, error_id::set_function_failed, prop, "set function failed", err );
            return FALSE;
        }
        return TRUE;
    }
    case tcamprop_system::prop_type::integer:
    {
        if( !G_VALUE_HOLDS_INT( value ) )
        {
            fwd_error_message( report_err_func, error_id::property_type_incompatible, prop, fmt::format( "expected integer, got {}", g_type_name( g_value_get_gtype( value ) ) ) );
            return FALSE;
        }
        auto val = (int64_t)g_value_get_int( value );
        auto err = prop_itf->set_property_value( tcamprop_system::prop_value{ val } );
        if( err )
        {
            fwd_error_message( report_err_func, error_id::set_function_failed, prop, "set function failed", err );
            return FALSE;
        }
        return TRUE;
    }
    case tcamprop_system::prop_type::real:
    {
        if( !G_VALUE_HOLDS_DOUBLE( value ) )
        {
            fwd_error_message( report_err_func, error_id::property_type_incompatible, prop, fmt::format( "expected real, got {}", g_type_name( g_value_get_gtype( value ) ) ) );
            return FALSE;
        }
        auto val = g_value_get_double( value );
        auto err = prop_itf->set_property_value( tcamprop_system::prop_value{ val } );
        if( err )
        {
            fwd_error_message( report_err_func, error_id::set_function_failed, prop, "set function failed", err );
            return FALSE;
        }
        return TRUE;
    }
    case tcamprop_system::prop_type::menu:
    {
        int index = 0;
        if( G_VALUE_HOLDS_STRING( value ) )
        {
            auto range_res = prop_itf->get_property_range();
            if( range_res.has_error() ) {
                return FALSE;
            }

            std::string_view str = g_value_get_string( value );

            auto& menu_entries = range_res.value().menu_entries;
            auto it = std::find( menu_entries.begin(), menu_entries.end(), str );
            if( it == menu_entries.end() )
            {
                fwd_error_message( report_err_func, error_id::property_type_incompatible, prop, fmt::format( "failed to find menu entry for '{}'", str ) );
                return FALSE;
            }
            index = std::distance( menu_entries.begin(), it );
        }
        else if( G_VALUE_HOLDS_INT( value ) )
        {
            index = g_value_get_int( value );
            if( index < 0 )
            {
                fwd_error_message( report_err_func, error_id::parameter_invalid_value, prop, fmt::format( "failed to find menu entry for '{}'", index ) );
                return FALSE;
            }
        }
        else
        {
            fwd_error_message( report_err_func, error_id::property_type_incompatible, prop, fmt::format( "expected int or string, got {}", g_type_name( g_value_get_gtype( value ) ) ) );
            return FALSE;
        }

        auto err = prop_itf->set_property_value( tcamprop_system::prop_value{ index } );
        if( err )
        {
            fwd_error_message( report_err_func, error_id::set_function_failed, prop, "set function failed", err );
            return FALSE;
        }
        return TRUE;
    }
    default:
        return FALSE;
    }
}

gchar* tcamprop_system::tcamprop_impl_get_property_type( property_list_interface* prop_list_itf,
                                                           const gchar* name, const report_error& report_err_func )
{
    if( prop_list_itf == nullptr || name == nullptr ) {
        fwd_error_message( report_err_func, error_id::parameter_nullptr );
        return FALSE;
    }
    auto prop_itf = prop_list_itf->find_property( name );
    if( !prop_itf ) {
        fwd_error_message( report_err_func, error_id::property_not_found, fmt::format( "Failed to find property '{}'.", name ) );
        return nullptr;
    }

    auto prop = prop_itf->get_property_info();

    return g_strdup( to_string( prop.type ) );
}
