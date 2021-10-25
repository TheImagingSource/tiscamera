
#pragma once

/** gvalue_wrapper is a simple GValue wrapper that implements value-semantics and offers some convenience methods
 */

#include <gst/gst.h>
#include "gvalue_functions.h"

namespace gvalue
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
        explicit gvalue_wrapper( const GValue& op2 ) noexcept
        {
            g_value_init( &value_, G_VALUE_TYPE( &op2) );
            g_value_copy( &op2, &value_ );
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
                unset();
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
        static gvalue_wrapper   make_value( double value_to_set ) noexcept {
            gvalue_wrapper rval = make_typed( G_TYPE_DOUBLE );
            g_value_set_double( &rval.value_, value_to_set );
            return rval;
        }
        static gvalue_wrapper   make_value( int value_to_set ) noexcept {
            gvalue_wrapper rval = make_typed( G_TYPE_INT );
            g_value_set_int( &rval.value_, value_to_set );
            return rval;
        }
        static gvalue_wrapper   make_value( int64_t value_to_set ) noexcept {
            gvalue_wrapper rval = make_typed( G_TYPE_INT64 );
            g_value_set_int64( &rval.value_, value_to_set );
            return rval;
        }
        static gvalue_wrapper   make_value( bool value_to_set ) noexcept {
            gvalue_wrapper rval = make_typed( G_TYPE_BOOLEAN );
            g_value_set_boolean( &rval.value_, value_to_set ? TRUE : FALSE );
            return rval;
        }
        static gvalue_wrapper   make_value( const char* value_to_set ) noexcept {
            gvalue_wrapper rval = make_typed( G_TYPE_STRING );
            g_value_set_string( &rval.value_, value_to_set );
            return rval;
        }
        ~gvalue_wrapper()
        {
            g_value_unset( &value_ );
        }

        constexpr const GValue*   get() const noexcept { return &value_; }
        constexpr GValue*         get() noexcept { return &value_; }

        constexpr const GValue& reference() const noexcept { return value_; }
        constexpr GValue&       reference() noexcept { return value_; }

        /** Clears the internal GValue,
         * Calls g_value_reset to reset the internal value, but does not reset the type.
         */
        void            reset() noexcept { g_value_reset( &value_ ); }
        /** Clears the full GValue.
         * Calls g_value_unset to reset the internal value and then clear the type.
         */
        void            unset() noexcept { g_value_unset( &value_ ); }

        constexpr GType           type() const noexcept { return G_VALUE_TYPE(&value_); }

        /**Fetches a string from the GValue. 
         * @returns An empty string when the returned pointer is nullptr, otherwise a copy of the const char* returned by g_value_get_string
         */
        std::string     get_string() const {
            if( auto res = g_value_get_string( &value_ ); res ) {
                return res;
            }
            return {};
        }
        /** Fetches a string from the GValue.
         * @returns An empty string_view when the returned pointer is nullptr, otherwise a string_view to the const char* returned by g_value_get_string
         */
        std::string_view    get_string_view() const noexcept { return gvalue::get_typed<std::string_view>( value_ ); }
        const char*         get_char_ptr() const noexcept { return g_value_get_string( &value_ ); }
        bool                get_bool() const noexcept { return g_value_get_boolean( &value_ ) != FALSE; }
        int                 get_int() const noexcept { return g_value_get_int( &value_ ); }
        double              get_double() const noexcept { return g_value_get_double( &value_ ); }
        int                 get_int64() const noexcept { return g_value_get_int64( &value_ ); }

        template<typename T>
        auto                get_typed() const noexcept -> T { return gvalue::get_typed<T>( value_ ); }
        template<typename T>
        auto                get_typed_opt() const noexcept -> std::optional<T> { return gvalue::get_typed_opt<T>( value_ ); }

        constexpr bool      contains( GType type ) const noexcept { return G_VALUE_TYPE( &value_ ) == type; }

        constexpr bool      is_untyped() const noexcept { return G_VALUE_TYPE( &value_ ) == 0; }
        constexpr bool      empty() const noexcept{ return is_untyped(); }

        /** Fetches the contained value and tries to convert it to the passed in type.
         * If the GValue contains the exact type, just returns its value, otherwise tries to convert it to the requested type.
         */
        template<typename T>    auto fetch_typed() const noexcept -> std::optional<T>
        {
            return gvalue::fetch_typed<T>( value_ );
        }
    private:
        GValue value_ = {};
    };

}

