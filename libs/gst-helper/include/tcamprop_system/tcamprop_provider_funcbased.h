
#pragma once

#include "tcamprop_provider_base.h"

#include <functional>
#include <memory>

namespace tcamprop_system
{
    struct property_list_funcbased_property_base;

    class property_list_funcbased : public property_list_interface
    {
    public:
        property_list_funcbased();
        ~property_list_funcbased();

        virtual auto get_property_list()->std::vector<std::string_view> final;
        virtual auto find_property( std::string_view name )->tcamprop_system::property_interface* final;

        template<typename T>
        using set_value_func = std::function<std::error_code( T )>;
        template<typename T>
        using get_value_func = std::function<outcome::result<T>()>;
        using get_flags_func = std::function<outcome::result<prop_flags>()>;

        using set_func_button = std::function<std::error_code()>;

        void register_menu( const property_info& nfo, const std::vector<std::string_view>& menu_entries, int64_t default_menu_entry, set_value_func<int64_t> set, get_value_func<int64_t> get, get_flags_func get_flags );
        void register_menu( const property_info& nfo, const std::vector<std::string_view>& menu_entries, int64_t default_menu_entry, set_value_func<int64_t> set, get_value_func<int64_t> get, prop_flags def_flags = prop_flags::def_flags );
        void register_double( const property_info& nfo, const prop_range_real& range, set_value_func<double> set, get_value_func<double> get, get_flags_func get_flags );
        void register_double( const property_info& nfo, const prop_range_real& range, set_value_func<double> set, get_value_func<double> get, prop_flags def_flags = prop_flags::def_flags );
        void register_integer( const property_info& nfo, const prop_range_integer& range, set_value_func<int64_t> set, get_value_func<int64_t> get, get_flags_func get_flags );
        void register_integer( const property_info& nfo, const prop_range_integer& range, set_value_func<int64_t> set, get_value_func<int64_t> get, prop_flags def_flags = prop_flags::def_flags );
        void register_boolean( const property_info& nfo, bool def, set_value_func<bool> set, get_value_func<bool> get, get_flags_func get_flags );
        void register_boolean( const property_info& nfo, bool def, set_value_func<bool> set, get_value_func<bool> get, prop_flags def_flags = prop_flags::def_flags );
        void register_button( const property_info& dsc, set_func_button set, get_flags_func get_flags );
        void register_button( const property_info& dsc, set_func_button set, prop_flags def_flags = prop_flags::def_flags );
    private:
        std::vector<std::unique_ptr<property_list_funcbased_property_base>> props_;
    };
}

