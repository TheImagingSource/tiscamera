#pragma once

#include "tcamprop_property_list_impl.h"

namespace tcamprop1::impl
{
    struct property_interface_impl_base_data
    {
    private:
        propgen_flags               propgen_flags_ = propgen_flags::none;
    public:
        prop_static_info_str        static_info_;
    private:
        value_or_func<prop_state>   get_state_;
    public:
        property_interface_impl_base_data( const propgen_params_base& params );
        property_interface_impl_base_data( propgen_params_base&& params );

        bool    should_check_out_of_range( uint32_t /*flags*/ ) const noexcept { return (propgen_flags_ & propgen_flags::check_out_of_range) == propgen_flags::check_out_of_range; }
        bool    should_check_state( uint32_t /*flags*/ ) const noexcept { return (propgen_flags_ & propgen_flags::check_state) == propgen_flags::check_state; }

        auto    check_against_state( bool is_write, uint32_t flags ) const noexcept -> std::error_code;

        auto    get_property_info() const noexcept -> tcamprop1::prop_static_info;
        auto    get_property_state( uint32_t flags ) const -> outcome::result<tcamprop1::prop_state>;
    };

    template<class TBase>
    class property_interface_impl_base : public TBase
    {
    private:
        property_interface_impl_base_data   base_data_;
    protected:
        bool    should_check_out_of_range( uint32_t flags ) const noexcept { return base_data_.should_check_out_of_range( flags ); }

        auto    check_against_state( bool is_write, uint32_t flags ) const noexcept -> std::error_code { return base_data_.check_against_state( is_write, flags ); }
    public:
        using base_type = TBase;

        property_interface_impl_base( const propgen_params_base& params ) : base_data_{ params } {}
        property_interface_impl_base( propgen_params_base&& params ) : base_data_{ std::move( params ) } {}

        auto get_property_name() const noexcept -> std::string_view final { return base_data_.static_info_.name; }
        auto get_property_info() const noexcept -> tcamprop1::prop_static_info final { return base_data_.get_property_info(); }
        auto get_property_state( uint32_t flags ) -> outcome::result<tcamprop1::prop_state> final { return base_data_.get_property_state( flags ); }
    };

    class property_interface_impl_enumeration : public property_interface_impl_base<tcamprop1::property_interface_enumeration>
    {
    private:
        set_value_func<int64_t>         set_value_;
        get_value_func<int64_t>         get_value_;

        prop_range_enumeration          static_range_;
        int64_t                         static_default_ = 0;
    public:
        property_interface_impl_enumeration( const propgen_params_enumeration& params );
        property_interface_impl_enumeration( propgen_params_enumeration&& params );

        auto get_property_range( uint32_t flags ) -> outcome::result<prop_range_enumeration> final;
        auto get_property_default( uint32_t flags )->outcome::result<std::string_view> final;
        auto get_property_value( uint32_t flags ) -> outcome::result<std::string_view> final;
        auto set_property_value( std::string_view val, uint32_t flags ) -> std::error_code  final;
    };

    class property_interface_impl_float : public property_interface_impl_base<tcamprop1::property_interface_float>
    {
    private:
        set_value_func<double>              set_value_;
        get_value_func<double>              get_value_;
        
        value_or_func<prop_range_float>     get_range_;
        value_or_func<double>               get_default_;

        std::string             unit_;
        FloatRepresentation_t   rep_ = FloatRepresentation_t::Linear;
    public:
        property_interface_impl_float( const propgen_params_float& params );
            
        auto get_property_range( uint32_t flags ) -> outcome::result<prop_range_float> final;
        auto get_property_default( uint32_t flags )->outcome::result<double> final;
        auto get_property_value( uint32_t flags )->outcome::result<double> final;
        auto set_property_value( double value, uint32_t flags )->std::error_code final;

        auto get_representation() const noexcept ->FloatRepresentation_t final { return rep_; }
        auto get_unit() const noexcept -> std::string_view final { return unit_; }
    };

    class property_interface_impl_integer : public property_interface_impl_base<tcamprop1::property_interface_integer>
    {
    private:
        set_value_func<int64_t>             set_value_;
        get_value_func<int64_t>             get_value_;

        value_or_func<prop_range_integer>   get_range_;
        value_or_func<int64_t>              get_default_;

        std::string             unit_;
        IntRepresentation_t     rep_ = IntRepresentation_t::Linear;
    public:
        property_interface_impl_integer( const propgen_params_integer& params );

        auto get_property_range( uint32_t flags ) -> outcome::result<prop_range_integer> final;
        auto get_property_default( uint32_t flags ) -> outcome::result<int64_t> final;

        auto get_property_value( uint32_t flags )->outcome::result<int64_t> final;
        auto set_property_value( int64_t value, uint32_t flags )->std::error_code final;

        auto get_representation()  const noexcept -> IntRepresentation_t final { return rep_; }
        auto get_unit()  const noexcept -> std::string_view final { return unit_; }
    };

    class property_interface_impl_boolean : public property_interface_impl_base<tcamprop1::property_interface_boolean>
    {
    private:
        set_value_func<bool>    set_value_;
        get_value_func<bool>    get_value_;

        value_or_func<bool>     get_default_;
    public:
        property_interface_impl_boolean( const propgen_params_boolean& params );

        auto get_property_default( uint32_t flags ) -> outcome::result<bool> final;
        auto get_property_value( uint32_t flags )->outcome::result<bool> final;
        auto set_property_value( bool value, uint32_t flags )->std::error_code final;
    };

    class property_interface_impl_command : public property_interface_impl_base<tcamprop1::property_interface_command>
    {
    private:
        set_command_func    set_value_;
    public:
        property_interface_impl_command( const propgen_params_command& params );

        auto execute_command( uint32_t flags )->std::error_code final;
    };
}

