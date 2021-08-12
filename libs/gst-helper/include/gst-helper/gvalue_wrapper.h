
#pragma once

/** gvalue_wrapper is a simple GValue wrapper that implements value-semantics and offers some convenience methods
 */

#include <gst/gst.h>

namespace gst_helper
{
    struct gvalue_wrapper
    {
        gvalue_wrapper() = default;
        gvalue_wrapper( gvalue_wrapper&& op2 ) noexcept
        {
            value_ = op2.value_;
            op2.value_ = {};
        }
        gvalue_wrapper( const gvalue_wrapper& op2 ) noexcept
        {
            g_value_init( &value_, G_VALUE_TYPE( &op2.value_ ) );
            g_value_copy( &op2.value_, &value_ );
        }
        gvalue_wrapper& operator=( gvalue_wrapper&& op2 ) noexcept
        {
            if( this != &op2 ) {
                value_ = op2.value_;
                op2.value_ = {};
            }
            return *this;
        }
        gvalue_wrapper& operator=( const gvalue_wrapper& op2 ) noexcept
        {
            if( this != &op2 ) {
                reset();
                g_value_init( &value_, G_VALUE_TYPE( &op2.value_ ) );
                g_value_copy( &op2.value_, &value_ );
            }
            return *this;
        }
        static gvalue_wrapper   make_typed( GType type ) noexcept {
            gvalue_wrapper rval;
            g_value_init( &rval.value_, type );
            return rval;
        }

        explicit gvalue_wrapper( double value_to_set ) noexcept {
            g_value_init( &value_, G_TYPE_DOUBLE );
            g_value_set_double( &value_, value_to_set );
        }
        explicit gvalue_wrapper( int value_to_set ) noexcept {
            g_value_init( &value_, G_TYPE_INT );
            g_value_set_int( &value_, value_to_set );
        }
        explicit gvalue_wrapper( bool value_to_set ) noexcept {
            g_value_init( &value_, G_TYPE_BOOLEAN );
            g_value_set_boolean( &value_, value_to_set ? TRUE : FALSE );
        }
        ~gvalue_wrapper()
        {
            g_value_unset( &value_ );
        }

        const GValue*   get() const noexcept { return &value_; }
        GValue*         get() noexcept { return &value_; }

        void            reset() noexcept { g_value_unset( &value_ ); }

        GType           get_type() const noexcept { return G_VALUE_TYPE(&value_); }

        std::string     get_string() const {
            if( auto res = g_value_get_string( &value_ ); res ) {
                return res;
            }
            return {};
        }
        std::string_view     get_string_view() const noexcept {
            if( auto res = g_value_get_string( &value_ ); res ) {
                return res;
            }
            return {};
        }
        const char*     get_char_ptr() const noexcept { return g_value_get_string( &value_ ); }
        bool            get_bool() const noexcept { return g_value_get_boolean( &value_ ) != FALSE; }
        int             get_int() const noexcept { return g_value_get_int( &value_ ); }
        double          get_double() const noexcept { return g_value_get_double( &value_ ); }

        bool            contains( GType type ) const noexcept { return G_VALUE_TYPE( &value_ ) == type; }

        bool            is_untyped() const noexcept { return G_VALUE_TYPE( &value_ ) == 0; }
    private:
        GValue value_ = {};
    };
}

