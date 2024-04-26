#pragma once

#include <cstdint>
#include <optional>
#include <string_view>
#include <type_traits>
#include <string>

#include "gvalue_helper.h"

namespace gvalue
{
    /** Type mapping from GType value -> type
     * E.g. from_gtype<G_TYPE_CHAR>::type == char
     * Only some fundamental types are supported
     */
    template<GType T>
    struct from_gtype {
        //using type = ???;
    };

    template<> struct from_gtype<G_TYPE_CHAR    > { using type = char; };
    template<> struct from_gtype<G_TYPE_UCHAR   > { using type = guchar; };
    template<> struct from_gtype<G_TYPE_BOOLEAN > { using type = bool; };
    template<> struct from_gtype<G_TYPE_INT     > { using type = int; };
    template<> struct from_gtype<G_TYPE_UINT    > { using type = unsigned int; };
    template<> struct from_gtype<G_TYPE_LONG    > { using type = long; };
    template<> struct from_gtype<G_TYPE_ULONG   > { using type = unsigned long; };
    template<> struct from_gtype<G_TYPE_INT64   > { using type = int64_t; };
    template<> struct from_gtype<G_TYPE_UINT64  > { using type = uint64_t; };
    template<> struct from_gtype<G_TYPE_FLOAT   > { using type = float; };
    template<> struct from_gtype<G_TYPE_DOUBLE  > { using type = double; };
    template<> struct from_gtype<G_TYPE_STRING  > { using type = char*; };
    template<> struct from_gtype<G_TYPE_POINTER > { using type = void*; };

    /** Helper for the type mapping of from_gtype.
     * 
     */
    template<GType T>
    using from_gtype_t = typename from_gtype<T>::type;

    template<typename T>
    constexpr GType   to_gtype() noexcept
    {
        using Tval = typename std::remove_cv<T>::type;
        if constexpr( std::is_same_v<Tval, char> ) { return G_TYPE_CHAR; }
        else if constexpr( std::is_same_v<Tval, guchar> ) { return G_TYPE_UCHAR; }
        else if constexpr( std::is_same_v<Tval, bool> ) { return G_TYPE_BOOLEAN; }
        else if constexpr( std::is_same_v<Tval, int> ) { return G_TYPE_INT; }
        else if constexpr( std::is_same_v<Tval, unsigned int> ) { return G_TYPE_UINT; }
        else if constexpr( std::is_same_v<Tval, long> ) { return G_TYPE_LONG; }
        else if constexpr( std::is_same_v<Tval, unsigned long> ) { return G_TYPE_ULONG; }
        else if constexpr( std::is_same_v<Tval, int64_t> ) { return G_TYPE_INT64; }
        else if constexpr( std::is_same_v<Tval, uint64_t> ) { return G_TYPE_UINT64; }
        else if constexpr( std::is_same_v<Tval, float> ) { return G_TYPE_FLOAT; }
        else if constexpr( std::is_same_v<Tval, double> ) { return G_TYPE_DOUBLE; }
        else if constexpr( std::is_same_v<Tval, char*> ) { return G_TYPE_STRING; }
        else if constexpr( std::is_same_v<Tval, void*> ) { return G_TYPE_POINTER; }
        else if constexpr( std::is_same_v<Tval, const char*> ) { return G_TYPE_STRING; }
        else if constexpr( std::is_same_v<Tval, const void*> ) { return G_TYPE_POINTER; }
        else {
            static_assert(std::is_same_v<Tval, guchar>, "Type mapping not implemented for this T");
        }
        return G_TYPE_INVALID;
    }

    template<typename T>
    T               get_typed( const GValue& gval ) noexcept
    {
        if constexpr( std::is_same_v<T,std::string_view> )
        {
            if( auto res = g_value_get_string( &gval ); res ) {
                return std::string_view( res );
            }
            return std::string_view{};
        }
        else
        {
            constexpr GType type = to_gtype<T>();
            if constexpr( type == G_TYPE_CHAR ) { return g_value_get_schar( &gval ); }
            else if constexpr( type == G_TYPE_UCHAR ) { return g_value_get_uchar( &gval ); }
            else if constexpr( type == G_TYPE_BOOLEAN ) { return g_value_get_boolean( &gval ) != FALSE; }
            else if constexpr( type == G_TYPE_INT ) { return g_value_get_int( &gval ); }
            else if constexpr( type == G_TYPE_UINT ) { return g_value_get_uint( &gval ); }
            else if constexpr( type == G_TYPE_LONG ) { return g_value_get_long( &gval ); }
            else if constexpr( type == G_TYPE_ULONG ) { return g_value_get_ulong( &gval ); }
            else if constexpr( type == G_TYPE_INT64 ) { return g_value_get_int64( &gval ); }
            else if constexpr( type == G_TYPE_UINT64 ) { return g_value_get_uint64( &gval ); }
            else if constexpr( type == G_TYPE_FLOAT ) { return g_value_get_float( &gval ); }
            else if constexpr( type == G_TYPE_DOUBLE ) { return g_value_get_double( &gval ); }
            else if constexpr( type == G_TYPE_STRING ) { return g_value_get_string( &gval ); }
            else if constexpr( type == G_TYPE_POINTER ) { return g_value_get_pointer( &gval ); }
            else
            {
                static_assert(std::is_same_v<T, char>, "Invalid type specified to GType mapping");
            }
            return T{};
        }
    }

    template<typename T>
    std::optional<T>               get_typed_opt( const GValue& gval ) noexcept
    {
        if constexpr( std::is_same_v<T, std::string_view> )
        {
            if( G_VALUE_TYPE( &gval ) != G_TYPE_STRING ) {
                return std::nullopt;
            }
            if( auto res = g_value_get_string( &gval ); res ) {
                return std::string_view( res );
            }
            return std::string_view{};
        }
        else if constexpr( std::is_same_v<T, std::string> )
        {
            if( G_VALUE_TYPE( &gval ) != G_TYPE_STRING ) {
                return std::nullopt;
            }
            if( auto res = g_value_get_string( &gval ); res ) {
                return std::string( res );
            }
            return std::string{};
        }
        else
        {
            if( G_VALUE_TYPE( &gval ) != to_gtype<T>() ) {
                return std::nullopt;
            }
            return get_typed<T>( gval );
        }
    }

    /** Fetches the contained value and tries to convert it to the passed in type.
     * If the GValue contains the exact type, just returns its value, otherwise tries to convert it to the requested type.
     */
    template<typename T>
    std::optional<T>        fetch_typed( const GValue& gval ) noexcept
    {
        static_assert(!std::is_same_v<T, std::string_view>, "Returning a string_view is a bad idea from a transformed temporary GValue");
        if constexpr( std::is_same_v<T, std::string> )
        {
            auto tmp = fetch_typed<const char*>( gval );
            if( tmp.has_value() ) {
                return std::string( tmp.value() );
            }
            return std::string{};
        }
        else
        {
            if( G_VALUE_TYPE( &gval ) == to_gtype<T>() ) {
                return get_typed<T>( gval );
            }

            GValue trans = {};
            g_value_init( &trans, to_gtype<T>() );
            if( g_value_transform( &gval, &trans ) ) {
                return get_typed<T>( trans );
            }
            return std::nullopt;
        }
    }

}

