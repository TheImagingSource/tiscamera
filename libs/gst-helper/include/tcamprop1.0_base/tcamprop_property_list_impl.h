
#pragma once

#include "tcamprop_property_interface.h"
#include "tcamprop_property_info.h"

#include <functional>
#include <memory>

namespace tcamprop1
{
    template<typename T>    using set_value_func = std::function<std::error_code( T )>;
    template<typename T>    using get_value_func = std::function<outcome::result<T>()>;

    using get_state_func = std::function<outcome::result<prop_state>()>;
    using set_command_func = std::function<std::error_code()>;

    enum class propgen_flags : uint32_t {
        none = 0x0,
        check_out_of_range = 0x1,
        check_state = 0x2,

        check_all = check_out_of_range | check_state,
    };

    constexpr propgen_flags   operator|( propgen_flags lhs, propgen_flags rhs ) noexcept { return static_cast<propgen_flags>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs)); }
    constexpr propgen_flags   operator&( propgen_flags lhs, propgen_flags rhs ) noexcept { return static_cast<propgen_flags>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs)); }

    auto make_range_integer_max() -> tcamprop1::prop_range_integer;

    template<typename T>
    struct value_or_func
    {
        value_or_func() = default;
        value_or_func( const value_or_func& ) = default;
        value_or_func( value_or_func&& ) = default;
        value_or_func& operator=( const value_or_func& ) = default;
        value_or_func& operator=( value_or_func&& ) = default;

        value_or_func( T val ) noexcept : value_{ val } {}
        value_or_func( get_value_func<T>&& func ) noexcept : func_{ std::move( func ) } {}
        value_or_func( const get_value_func<T>& func ) noexcept : func_{ func } {}

        auto    operator()() const -> outcome::result<T> { return func_ ? func_() : value_; }

        bool    is_value() const noexcept { return !func_; }
        T       get_value() const noexcept { return value_; }
    private:
        T                   value_ = {};
        get_value_func<T>   func_;
    };

    struct propgen_params_base
    {
        propgen_params_base() = default;
        propgen_params_base( const prop_static_info& nfo, const get_state_func& func, propgen_flags flag = propgen_flags::check_all ) noexcept
            : info( nfo ), get_state{ func }, flags{ flag }
        {}
        propgen_params_base( const prop_static_info& nfo, prop_state state, propgen_flags flag = propgen_flags::check_all ) noexcept
            : info( nfo ), get_state{ state }, flags{ flag }
        {}
        propgen_params_base( const prop_static_info& nfo, propgen_flags flag = propgen_flags::check_all ) noexcept
            : info( nfo ), flags{ flag }
        {}

        prop_static_info            info;
        value_or_func<prop_state>   get_state;
        propgen_flags               flags = propgen_flags::check_all;
    };

    struct propgen_params_enumeration : propgen_params_base
    {
        prop_range_enumeration   get_range;     // we currently only support a static range and default for this ...
        int64_t                  get_default;

        get_value_func<int64_t> get_func;
        set_value_func<int64_t> set_func;
    };

    struct propgen_params_float : propgen_params_base
    {
        value_or_func<prop_range_float>     get_range;
        value_or_func<double>               get_default;

        get_value_func<double> get_func;
        set_value_func<double> set_func;

        std::string_view        unit;
        FloatRepresentation_t   rep;
    };

    struct propgen_params_integer : propgen_params_base
    {
        value_or_func<prop_range_integer>   get_range;
        value_or_func<int64_t>              get_default;

        get_value_func<int64_t> get_func;
        set_value_func<int64_t> set_func;

        std::string_view        unit;
        IntRepresentation_t     rep;
    };

    struct propgen_params_boolean : propgen_params_base
    {
        propgen_params_boolean( propgen_params_base&& base, bool r, get_value_func<bool> get, set_value_func<bool> set )
            : propgen_params_base{ base }, get_default{ r }, get_func{ get }, set_func{ set } {}

        value_or_func<bool>     get_default;

        get_value_func<bool>    get_func;
        set_value_func<bool>    set_func;
    };

    struct propgen_params_command : propgen_params_base
    {
        set_command_func set_func;
    };

    auto    create_propgen( const propgen_params_float& prop_impl )->std::unique_ptr<property_interface_float>;
    auto    create_propgen( const propgen_params_integer& prop_impl )->std::unique_ptr<property_interface_integer>;
    auto    create_propgen( const propgen_params_boolean& prop_impl )->std::unique_ptr<property_interface_boolean>;
    auto    create_propgen( const propgen_params_command& prop_impl )->std::unique_ptr<property_interface_command>;
    auto    create_propgen( const propgen_params_enumeration& prop_impl )->std::unique_ptr<property_interface_enumeration>;


    class property_list_funcbased : public property_list_interface
    {
    public:
        property_list_funcbased();
        ~property_list_funcbased();

        virtual auto get_property_list()->std::vector<std::string_view> final;
        virtual auto find_property( std::string_view name ) -> tcamprop1::property_interface* final;

        void register_Enumeration( const prop_static_info_enumeration& nfo, const prop_range_enumeration& range, int default_index, set_value_func<int64_t> set, get_value_func<int64_t> get, get_state_func get_state = {} );
        void register_Float( const prop_static_info_float& nfo, const prop_range_float& range, double default_value, set_value_func<double> set, get_value_func<double> get, get_state_func get_state = {} );
        void register_Integer( const prop_static_info_integer& nfo, const prop_range_integer& range, int64_t default_value, set_value_func<int64_t> set, get_value_func<int64_t> get, get_state_func get_state = {} );
        void register_Boolean( const prop_static_info_boolean& nfo, bool default_value, set_value_func<bool> set, get_value_func<bool> get, get_state_func get_state = {} );
        void register_Command( const prop_static_info_command& dsc, set_command_func set, get_state_func get_state = {} );


        void register_RO_Integer( const prop_static_info_integer& nfo, get_value_func<int64_t> get );

        template<size_t N>
        void register_Enumeration( const prop_static_info_enumeration& nfo, const std::array<std::string_view,N>& arr, int default_index, set_value_func<int64_t> set, get_value_func<int64_t> get, get_state_func get_state = {} )
        {
            register_Enumeration( nfo, { arr }, default_index, set, get, get_state );
        }

        void    add( const propgen_params_float& prop_impl )        { add_interface( create_propgen( prop_impl ) ); }
        void    add( const propgen_params_integer& prop_impl )      { add_interface( create_propgen( prop_impl ) ); }
        void    add( const propgen_params_boolean& prop_impl )      { add_interface( create_propgen( prop_impl ) ); }
        void    add( const propgen_params_command& prop_impl )      { add_interface( create_propgen( prop_impl ) ); }
        void    add( const propgen_params_enumeration& prop_impl )  { add_interface( create_propgen( prop_impl ) ); }

        void    add_interface( std::unique_ptr<property_interface>&& entry );
    private:
        std::vector<std::unique_ptr<property_interface>> props_;
    };
}

