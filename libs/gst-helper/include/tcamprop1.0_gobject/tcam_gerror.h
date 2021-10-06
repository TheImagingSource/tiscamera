
#pragma once

#include <tcamprop1.0_base/tcamprop_errors.h>

#include <glib.h>

#include <Tcam-1.0.h>

namespace tcamprop1_gobj
{
    using   translate_to_gerror = bool( GError** gerr_val, const std::error_code& errc );
    /**
     * Registers a translation function for a specific std::error_code category to the GError.
     * A function could look like this:
     * 
        static bool translate_tcamprop1_status( GError** gerr_val, const std::error_code& errc )
        {
            if( errc.category() != tcamprop1::error_category() ) {
                return false;
            }
            tcamprop1_gobj::set_gerror( gerr_val, static_cast<tcamprop1::status>(errc.value()) );
            return true;
        }
     * Note: There are 16 slots for functions. If there is no more room, the function will not be added
     */
    bool    register_translator( translate_to_gerror* func );

    void    set_gerror( GError** gerr_val, tcamprop1::status errc );
    void    set_gerror( GError** gerr_val, const std::error_code& errc );
    void    set_gerror( GError** gerr_val, TcamError errc, std::string_view txt = {} );
}