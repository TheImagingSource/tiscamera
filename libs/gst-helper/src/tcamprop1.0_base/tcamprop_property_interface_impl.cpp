

#include "../../include/tcamprop1.0_base/tcamprop_provider_interface_impl.h"
#include "../../include/tcamprop1.0_base/tcamprop_errors.h"

static auto check_against_state_( bool is_write, const tcamprop1::prop_state& state ) noexcept -> std::error_code
{
    if( !state.is_implemented ) {
        return tcamprop1::status::property_is_not_implemented;
    }
    if( !state.is_available ) {
        return tcamprop1::status::property_is_not_available;
    }
    if( is_write && state.is_locked ) {
        return tcamprop1::status::property_is_locked;
    }
    return tcamprop1::status::success;
}

tcamprop1::impl::property_interface_impl_base_data::property_interface_impl_base_data( const propgen_params_base& params )
    : 
    propgen_flags_{ params.flags },
    static_info_{ params.info }, get_state_( params.get_state )
{
}

tcamprop1::impl::property_interface_impl_base_data::property_interface_impl_base_data( propgen_params_base&& params )
    :
    propgen_flags_{ params.flags },
    static_info_{ params.info }, get_state_( std::move( params.get_state ) )
{
}

auto tcamprop1::impl::property_interface_impl_base_data::check_against_state( bool is_write, uint32_t flags ) const noexcept -> std::error_code
{
    if( !should_check_state( flags ) ) {
        return tcamprop1::status::success;
    }
    if( !get_state_.is_value() ) {
        auto cur_state_opt = get_state_();
        if( cur_state_opt.has_error() ) {
            return cur_state_opt.error();
        }
        return check_against_state_( is_write, cur_state_opt.value() );
    }
    return check_against_state_( is_write, get_state_.get_value() );
}

auto tcamprop1::impl::property_interface_impl_base_data::get_property_info() const noexcept -> tcamprop1::prop_static_info
{
    return static_info_.to_prop_static_info();
}

auto tcamprop1::impl::property_interface_impl_base_data::get_property_state( uint32_t /*flags*/ ) const -> outcome::result<tcamprop1::prop_state>
{
    return get_state_();
}

tcamprop1::impl::property_interface_impl_enumeration::property_interface_impl_enumeration( propgen_params_enumeration&& params )
    : property_interface_impl_base{ params }, set_value_{ std::move( params.set_func ) }, get_value_{ std::move( params.get_func ) }, 
    static_range_{ std::move( params.get_range ) }, static_default_{ std::move( params.get_default ) }
{

}

tcamprop1::impl::property_interface_impl_enumeration::property_interface_impl_enumeration( const propgen_params_enumeration& params )
    : property_interface_impl_base{ params }, set_value_{ params.set_func }, get_value_{ params.get_func }, 
    static_range_{ params.get_range }, static_default_{ params.get_default }
{

}

auto tcamprop1::impl::property_interface_impl_enumeration::get_property_range( uint32_t /*flags*/ ) -> outcome::result<prop_range_enumeration>
{
    return static_range_;
}

auto tcamprop1::impl::property_interface_impl_enumeration::get_property_default( uint32_t /*flags*/ ) ->outcome::result<std::string_view>
{
    return static_range_.get_entry_at( static_default_ );
}

auto tcamprop1::impl::property_interface_impl_enumeration::get_property_value( uint32_t flags ) -> outcome::result<std::string_view>
{
    if( auto err = check_against_state( false, flags ); err ) {
        return err;
    }
    if( !get_value_ ) {
        return tcamprop1::status::property_is_not_available;
    }
    if( auto res = get_value_(); res.has_value() ) {
        return static_range_.get_entry_at( res.value() );
    }
    return tcamprop1::status::parameter_out_ot_range;
}

auto tcamprop1::impl::property_interface_impl_enumeration::set_property_value( std::string_view val, uint32_t flags ) -> std::error_code
{
    if( auto err = check_against_state( true, flags ); err ) {
        return err;
    }
    if( !set_value_ ) {
        return tcamprop1::status::property_is_locked;
    }
    if( int idx = static_range_.get_index_of( val ); idx != -1 ) {
        return set_value_( idx );
    }
    return tcamprop1::status::parameter_out_ot_range;
}

tcamprop1::impl::property_interface_impl_float::property_interface_impl_float( const propgen_params_float& params )
    : property_interface_impl_base{ params }, set_value_{ std::move( params.set_func ) }, get_value_{ std::move( params.get_func ) },
    get_range_{ params.get_range }, get_default_{ params.get_default }, unit_{ params.unit }, rep_{ params.rep }
{
}

auto tcamprop1::impl::property_interface_impl_float::get_property_range( uint32_t /*flags*/ ) -> outcome::result<prop_range_float>
{
    return get_range_();
}

auto tcamprop1::impl::property_interface_impl_float::get_property_default( uint32_t /*flags*/ ) -> outcome::result<double>
{
    return get_default_();
}

auto tcamprop1::impl::property_interface_impl_float::get_property_value( uint32_t flags ) -> outcome::result<double>
{
    if( auto err = check_against_state( false, flags ); err ) {
        return err;
    }
    if( !get_value_ ) {
        return tcamprop1::status::property_is_locked;
    }
    return get_value_();
}

auto tcamprop1::impl::property_interface_impl_float::set_property_value( double value, uint32_t flags ) -> std::error_code
{
    if( auto err = check_against_state( true, flags ); err ) {
        return err;
    }
    if( !set_value_ ) {
        return tcamprop1::status::property_is_readonly;
    }
    if( should_check_out_of_range( flags ) ) {
        auto range_res = get_range_();
        if( range_res.has_error() ) {
            return range_res.error();
        }
        auto range = range_res.value();
        if( value < range.min || value > range.max ) {
            return tcamprop1::status::parameter_out_ot_range;
        }
    }
    return set_value_( value );
}

tcamprop1::impl::property_interface_impl_integer::property_interface_impl_integer( const propgen_params_integer& params )
    : property_interface_impl_base{ params }, set_value_{ std::move( params.set_func ) }, get_value_{ std::move( params.get_func ) }, 
    get_range_{ params.get_range }, get_default_{ params.get_default }, unit_{ params.unit }, rep_{ params.rep }
{
}

auto tcamprop1::impl::property_interface_impl_integer::get_property_range( uint32_t /*flags*/ ) -> outcome::result<prop_range_integer>
{
    return get_range_();
}

auto tcamprop1::impl::property_interface_impl_integer::get_property_default( uint32_t /*flags*/ ) ->outcome::result<int64_t>
{
    return get_default_();
}

auto tcamprop1::impl::property_interface_impl_integer::get_property_value( uint32_t flags ) ->outcome::result<int64_t>
{
    if( auto err = check_against_state( false, flags ); err ) {
        return err;
    }
    if( !get_value_ ) {
        return tcamprop1::status::property_is_not_available;
    }
    return get_value_();
}

auto tcamprop1::impl::property_interface_impl_integer::set_property_value( int64_t value, uint32_t flags ) ->std::error_code
{
    if( auto err = check_against_state( true, flags ); err ) {
        return err;
    }
    if( !set_value_ ) {
        return tcamprop1::status::property_is_locked;
    }
    if( should_check_out_of_range( flags ) ) {
        auto range_res = get_range_();
        if( range_res.has_error() ) {
            return range_res.error();
        }
        auto range = range_res.value();
        if( value < range.min || value > range.max ) {
            return tcamprop1::status::parameter_out_ot_range;
        }
    }
    return set_value_( value );
}

tcamprop1::impl::property_interface_impl_boolean::property_interface_impl_boolean( const propgen_params_boolean& params ) 
    : property_interface_impl_base{ params }, set_value_{ params.set_func }, get_value_{ params.get_func }, get_default_{ params.get_default }
{
}

auto tcamprop1::impl::property_interface_impl_boolean::get_property_default( uint32_t /*flags*/ ) -> outcome::result<bool>
{
    return get_default_();
}

auto tcamprop1::impl::property_interface_impl_boolean::get_property_value( uint32_t flags ) ->outcome::result<bool>
{
    if( auto err = check_against_state( false, flags ); err ) {
        return err;
    }
    if( !get_value_ ) {
        return tcamprop1::status::property_is_not_available;
    }
    return get_value_();
}

auto tcamprop1::impl::property_interface_impl_boolean::set_property_value( bool value, uint32_t flags ) ->std::error_code
{
    if( auto err = check_against_state( true, flags ); err ) {
        return err;
    }
    if( !set_value_ ) {
        return tcamprop1::status::property_is_locked;
    }
    return set_value_( value );
}

tcamprop1::impl::property_interface_impl_command::property_interface_impl_command( const propgen_params_command& params )
    : property_interface_impl_base( params ), set_value_{ params.set_func }
{
    assert( set_value_ != nullptr );
}

auto tcamprop1::impl::property_interface_impl_command::execute_command( uint32_t flags ) ->std::error_code
{
    if( auto err = check_against_state( true, flags ); err ) {
        return err;
    }
    return set_value_();
}
