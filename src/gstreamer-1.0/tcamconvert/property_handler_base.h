
#pragma once

#include <vector>
#include <string>
#include <string_view>

#include <outcome/result.hpp>

namespace prop_system
{
    enum class prop_type
    {
        boolean,
        integer,
        real,
        button,
        menu
    };


    constexpr const char* to_string(prop_type t) noexcept
    {
        switch (t)
        {
            case prop_type::boolean:
                return "boolean";
            case prop_type::integer:
                return "integer";
            case prop_type::real:
                return "double";
            case prop_type::button:
                return "button";
            case prop_type::menu:
                return "enum";
            default:
                return nullptr;
        };
    }


    enum class prop_flags : uint32_t
    {
        noflags                         = 0x0,
        implemented                     = 0x1,
        available                       = 0x2,
        locked                          = 0x4,

        external                        = 0x8,  // additional flags like 'External' to indicate library and not camera internal properties?

        hide_from_get_property_names    = 0x100,
        debug_property                  = 0x200,

        def_flags = implemented | available,
    };

    constexpr prop_flags operator&( prop_flags lhs, prop_flags rhs ) noexcept {
        return static_cast<prop_flags>( static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs) );
    }
    constexpr prop_flags operator|( prop_flags lhs, prop_flags rhs ) noexcept {
        return static_cast<prop_flags>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
    }
    constexpr prop_flags& operator&=( prop_flags& lhs, prop_flags rhs ) noexcept { return lhs = lhs & rhs; }
    constexpr prop_flags& operator|=( prop_flags& lhs, prop_flags rhs ) noexcept { return lhs = lhs | rhs; }
    constexpr bool operator!( prop_flags lhs ) noexcept { return lhs == prop_flags::noflags; }

    union prop_values
    {
        constexpr prop_values() noexcept : integer( 0 ) {}
        constexpr prop_values( double v ) noexcept : real( v ) {}
        constexpr prop_values( float v ) noexcept : real( v ) {}
        constexpr prop_values( int v ) noexcept : integer( v ) {}
        constexpr prop_values( int64_t v ) noexcept : integer( v ) {}
        constexpr prop_values( bool v ) noexcept : integer( v ? 1 : 0 ) {}

        double      real;
        int64_t     integer;
    };

    struct property_desc
    {
        std::string_view prop_name;

        prop_type   type;

        std::string_view prop_category;
        std::string_view prop_group;

        std::vector<const char*> menu_entries = {};

        prop_flags type_flags = prop_flags::def_flags;
    };
    struct prop_range_real
    {
        double  min = 0;
        double  max = 0;
        double  def = 0;
        double  stp = 1.;
    };
    struct prop_range_integer
    {
        int64_t  min = 0;
        int64_t  max = 0;
        int64_t  def = 0;
        int64_t  stp = 1;
    };
    struct prop_range
    {
        constexpr prop_range()= default;
        constexpr prop_range( prop_values min, prop_values max, prop_values def, prop_values stp ) noexcept : val_min( min ), val_max( max ), val_def( def ), val_stp( stp ) {}
        constexpr prop_range( prop_range_integer r ) noexcept  : val_min( r.min ), val_max( r.max ), val_def( r.def ), val_stp( r.stp ) {}
        constexpr prop_range( prop_range_real r ) noexcept : val_min( r.min ), val_max( r.max ), val_def( r.def ), val_stp( r.stp ) {}

        prop_values val_min;
        prop_values val_max;
        prop_values val_def;
        prop_values val_stp;

        constexpr prop_range_real       to_real() const noexcept { return { val_min.real, val_max.real, val_def.real, val_stp.real }; }
        constexpr prop_range_integer    to_integer() const noexcept { return { val_min.integer, val_max.integer, val_def.integer, val_stp.integer }; }

        static prop_range  make_boolean( bool default_value ) noexcept { return prop_range_integer{ 0, 1, default_value ? 1 : 0, 1 }; }
        static prop_range  make_button() noexcept { return prop_range_integer{ 0, 1, 0, 1 }; }
    };

    class property_interface
    {
    public:
        virtual ~property_interface() = default;

        virtual std::vector<prop_system::property_desc>		get_property_list() = 0;
        virtual std::optional<prop_system::property_desc>   get_property_desc( std::string_view name ) = 0;

        virtual outcome::result<prop_system::prop_range>    get_property_range( std::string_view name ) = 0;
        virtual outcome::result<prop_system::prop_flags>	get_property_flags( std::string_view name ) = 0;
        virtual outcome::result<prop_system::prop_values>	get_property( std::string_view name ) = 0;
        virtual std::error_code					            set_property( std::string_view name, prop_system::prop_values new_value ) = 0;
    };


    inline auto prop_flags_add_locked( bool is_locked, prop_flags current_flags = prop_flags::def_flags ) -> prop_flags {
        if( is_locked ) {
            return current_flags | prop_flags::locked;
        }
        return current_flags;
    }

    inline auto prop_flags_add_locked( bool is_locked, outcome::result<prop_flags> res ) -> outcome::result<prop_flags>
    {
        if( res.has_value() ) {
            return prop_flags_add_locked( is_locked, res.value() );
        } 
        return res.error();
    }
}

