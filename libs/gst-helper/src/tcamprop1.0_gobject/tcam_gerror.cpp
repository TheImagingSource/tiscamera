
#include "../../include/tcamprop1.0_gobject/tcam_gerror.h"

#include <tcam-property-1.0.h>

#include <shared_mutex>
#include <cassert>

static auto map_error( tcamprop1::status err ) -> TcamError
{
    switch( err )
    {
    case tcamprop1::status::success:                            return TCAM_ERROR_SUCCESS;
    case tcamprop1::status::unknown:                            return TCAM_ERROR_UNKNOWN;
    case tcamprop1::status::parameter_null:                     return TCAM_ERROR_PARAMETER_INVALID;

    case tcamprop1::status::property_is_not_implemented:        return TCAM_ERROR_PROPERTY_NOT_IMPLEMENTED;
    case tcamprop1::status::property_is_not_available:          return TCAM_ERROR_PROPERTY_NOT_AVAILABLE;
    case tcamprop1::status::property_is_locked:                 return TCAM_ERROR_PROPERTY_NOT_WRITEABLE;
    case tcamprop1::status::parameter_out_ot_range:             return TCAM_ERROR_PROPERTY_VALUE_OUT_OF_RANGE;
    case tcamprop1::status::parameter_type_incompatible:        return TCAM_ERROR_PROPERTY_TYPE_INCOMPATIBLE;

    case tcamprop1::status::property_is_readonly:               return TCAM_ERROR_PROPERTY_NOT_WRITEABLE;
    case tcamprop1::status::property_default_not_available:     return TCAM_ERROR_PROPERTY_DEFAULT_NOT_AVAILABLE;
    
    case tcamprop1::status::enumeration_property_list_error:    return TCAM_ERROR_PROPERTY_VALUE_OUT_OF_RANGE;
    case tcamprop1::status::device_not_opened:                  return TCAM_ERROR_DEVICE_NOT_OPENED;
    case tcamprop1::status::device_closed:                      return TCAM_ERROR_DEVICE_LOST;
    }
    return TCAM_ERROR_UNKNOWN;
}

static bool translate_tcamprop1_status( GError** gerr_val, const std::error_code& errc )
{
    if( errc.category() != tcamprop1::error_category() ) {
        return false;
    }
    tcamprop1_gobj::set_gerror( gerr_val, static_cast<tcamprop1::status>(errc.value()) );
    return true;
}


static std::shared_mutex translator_mtx;
static tcamprop1_gobj::translate_to_gerror* g_func_list[16] = {
    nullptr
};

bool tcamprop1_gobj::register_translator( translate_to_gerror* func )
{
    std::lock_guard lck{ translator_mtx };
    for( auto& entry : g_func_list ) {
        if( entry == nullptr ) {
            entry = func;
            return true;
        }
    }
    assert( false && "Failed to register std::error_code translation function" );
    return false;
}

static bool exec_translator( GError** gerr_val, const std::error_code& errc )
{
    assert( gerr_val != nullptr );

    std::shared_lock lck{ translator_mtx };
    for( auto& entry : g_func_list ) {
        if( entry == nullptr ) { // nullptr signals the end of the list
            return false;
        }
        if( entry( gerr_val, errc ) ) {
            return true;
        }
    }
    return false;
}

void tcamprop1_gobj::set_gerror( GError** gerr_val, tcamprop1::status errc )
{
    if( gerr_val == nullptr ) {
        return;
    }
    if( errc == tcamprop1::status::success ) {
        return;
    }
    tcamprop1_gobj::set_gerror( gerr_val, map_error( errc ), tcamprop1::to_string( errc ) );
}

void tcamprop1_gobj::set_gerror( GError** gerr_val, const std::error_code& errc )
{
    if( gerr_val == nullptr || !errc ) {
        return;
    }
    if( translate_tcamprop1_status( gerr_val, errc ) )
    {
        return;
    }
    if( !exec_translator( gerr_val, errc ) ) {
        tcamprop1_gobj::set_gerror( gerr_val, TCAM_ERROR_UNKNOWN, errc.message() );
    }
}

void tcamprop1_gobj::set_gerror( GError** gerr_val, TcamError errc, std::string_view txt )
{
    if( !gerr_val || errc == TCAM_ERROR_SUCCESS ) {
        return;
    }
    if( txt.empty() ) {
        auto tmp = g_enum_to_string( tcam_error_get_type(), errc );
        if( tmp != nullptr ) {
            set_gerror( gerr_val, errc, tmp );
            g_free( tmp );
        } else {
            g_set_error( gerr_val, tcam_error_quark(), errc, "Error: Unknown" );
        }
    } else {
        g_set_error( gerr_val, tcam_error_quark(), errc, "Error: %.*s", static_cast<int>(txt.size()), txt.data() );
    }
}
