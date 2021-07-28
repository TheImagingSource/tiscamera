
#pragma once

#include <gst/gst.h>

namespace gst_helper
{
    struct gvalue_wrapper
    {
        gvalue_wrapper() = default;
        gvalue_wrapper( gvalue_wrapper&& ) = delete;
        gvalue_wrapper( const gvalue_wrapper& op2 )
        {
            g_value_init( &value_, G_VALUE_TYPE( op2.value_ ) );
            g_value_copy( &op2.value_, &value_ );
        }
        gvalue_wrapper& operator=( gvalue_wrapper&& ) = delete;
        gvalue_wrapper& operator=( const gvalue_wrapper& op2 )
        {
            if( this != &op2 ) {
                reset();
                g_value_init( &value_, G_VALUE_TYPE( op2.value_ ) );
                g_value_copy( &op2.value_, &value_ );
            }
            return *this;
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

        operator GValue* () noexcept { return &value_; }
        GValue* operator&() noexcept { return &value_; }

        const GValue*   get() const noexcept { return &value_; }
        GValue*         get() noexcept { return &value_; }

        void            reset() noexcept { g_value_unset( &value_ ); }
    private:
        GValue value_ = {};
    };
}

