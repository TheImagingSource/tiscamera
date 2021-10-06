
#include "../../include/tcamprop1.0_base/tcamprop_property_list_impl.h"
#include "../../include/tcamprop1.0_base/tcamprop_provider_interface_impl.h"

#include <algorithm>
#include <limits>


auto tcamprop1::make_range_integer_max() -> tcamprop1::prop_range_integer
{
    return tcamprop1::prop_range_integer{ std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max(), 1 };
}

tcamprop1::property_list_funcbased::property_list_funcbased() = default;
tcamprop1::property_list_funcbased::~property_list_funcbased() = default;

tcamprop1::property_interface* tcamprop1::property_list_funcbased::find_property(
    std::string_view name)
{
    auto it = std::find_if(props_.begin(), props_.end(), [name](auto& p) { return p->get_property_name() == name; });
    if (it == props_.end())
    {
        return nullptr;
    }
    return it->get();
}

std::vector<std::string_view> tcamprop1::property_list_funcbased::get_property_list()
{
    std::vector<std::string_view> rval;
    for (auto&& p : props_) { rval.push_back(p->get_property_name()); }
    return rval;
}

void tcamprop1::property_list_funcbased::add_interface( std::unique_ptr<tcamprop1::property_interface>&& entry )
{
    props_.push_back( std::move( entry ) );
}

void tcamprop1::property_list_funcbased::register_Float( const prop_static_info_float& nfo, const prop_range_float& range, double default_value, set_value_func<double> set, get_value_func<double> get, get_state_func get_state )
{
    add( propgen_params_float{ { nfo, get_state }, range, default_value, get, set, nfo.unit, nfo.representation } );
}

void tcamprop1::property_list_funcbased::register_Integer( const prop_static_info_integer& nfo, const prop_range_integer& range, int64_t default_value, set_value_func<int64_t> set, get_value_func<int64_t> get, get_state_func get_state )
{
    add( propgen_params_integer{ { nfo, get_state }, range, default_value, get, set, nfo.unit, nfo.representation } );
}

void tcamprop1::property_list_funcbased::register_Boolean( const prop_static_info_boolean& nfo, bool default_value, set_value_func<bool> set, get_value_func<bool> get, get_state_func get_state )
{
    add( propgen_params_boolean{ { nfo, get_state }, default_value, get, set } );
}

void tcamprop1::property_list_funcbased::register_Enumeration( const prop_static_info_enumeration& nfo, const prop_range_enumeration& range, int default_index, set_value_func<int64_t> set, get_value_func<int64_t> get, get_state_func get_state )
{
    add( propgen_params_enumeration{ { nfo, get_state }, range, default_index, get, set } );
}

void tcamprop1::property_list_funcbased::register_Command( const prop_static_info_command& nfo, set_command_func set, get_state_func get_state )
{
    add( propgen_params_command{ { nfo, get_state }, set } );
}

void tcamprop1::property_list_funcbased::register_RO_Integer( const prop_static_info_integer& nfo, get_value_func<int64_t> get )
{
    add( propgen_params_integer{ { nfo, prop_state{ true, true, true } }, make_range_integer_max(), 0, std::move( get ), {}, nfo.unit, nfo.representation } );
}

auto tcamprop1::create_propgen( const tcamprop1::propgen_params_enumeration& prop_impl ) ->std::unique_ptr<tcamprop1::property_interface_enumeration>
{
    return std::make_unique<tcamprop1::impl::property_interface_impl_enumeration>( prop_impl );
}

auto tcamprop1::create_propgen( const tcamprop1::propgen_params_float& prop_impl ) ->std::unique_ptr<tcamprop1::property_interface_float>
{
    return std::make_unique<tcamprop1::impl::property_interface_impl_float>( prop_impl );
}

auto tcamprop1::create_propgen( const tcamprop1::propgen_params_integer& prop_impl ) ->std::unique_ptr<tcamprop1::property_interface_integer>
{
    return std::make_unique<tcamprop1::impl::property_interface_impl_integer>( prop_impl );
}

auto tcamprop1::create_propgen( const tcamprop1::propgen_params_boolean& prop_impl ) ->std::unique_ptr<tcamprop1::property_interface_boolean>
{
    return std::make_unique<tcamprop1::impl::property_interface_impl_boolean>( prop_impl );
}

auto tcamprop1::create_propgen( const tcamprop1::propgen_params_command& prop_impl ) ->std::unique_ptr<tcamprop1::property_interface_command>
{
    return std::make_unique<tcamprop1::impl::property_interface_impl_command>( prop_impl );
}
