
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

        void register_double( const property_info& nfo, const prop_range_real& range, set_value_func<double> set, get_value_func<double> get, get_flags_func get_fla );
        void register_double( const property_info& nfo, const prop_range_real& range, set_value_func<double> set, get_value_func<double> get, prop_flags def_flags = prop_flags::def_flags );
        void register_menu( const property_info& nfo, const std::vector<std::string_view>& menu_entries, int default_menu_entry, set_value_func<int> set, get_value_func<int> get, get_flags_func get_fla );
        void register_menu( const property_info& nfo, const std::vector<std::string_view>& menu_entries, int default_menu_entry, set_value_func<int> set, get_value_func<int> get, prop_flags def_flags = prop_flags::def_flags );
        void register_integer( const property_info& nfo, const prop_range_integer& range, set_value_func<int> set, get_value_func<int> get, get_flags_func get_fla );
        void register_integer( const property_info& nfo, const prop_range_integer& range, set_value_func<int> set, get_value_func<int> get, prop_flags def_flags = prop_flags::def_flags );

        void register_double( const property_desc& dsc, set_value_func<double> set, get_value_func<double> get, const prop_range_real& range );
        void register_double( const property_desc& dsc, set_value_func<double> set, get_value_func<double> get, get_flags_func get_fla, const prop_range_real& range );

        void register_integer( const property_desc& dsc, set_value_func<int> set, get_value_func<int> get, const prop_range_integer& range );
        void register_integer( const property_desc& dsc, set_value_func<int> set, get_value_func<int> get, get_flags_func get_fla, const prop_range_integer& range );

        void register_boolean( const property_desc& dsc, set_value_func<bool> set, get_value_func<bool> get, bool def );
        void register_boolean( const property_desc& dsc, set_value_func<bool> set, get_value_func<bool> get, get_flags_func get_flags, bool def );

        void register_menu( const property_desc& dsc, set_value_func<int> set, get_value_func<int> get, int def );
        void register_menu( const property_desc& dsc, set_value_func<int> set, get_value_func<int> get, int def, const std::vector<std::string>& menu_entries );
        void register_menu( const property_desc& dsc, set_value_func<int> set, get_value_func<int> get, get_flags_func get_flags, int def, const std::vector<std::string>& menu_entries );

        void register_button( const property_desc& dsc, set_func_button set );
        void register_button( const property_desc& dsc, set_func_button set, get_flags_func get_flags );
    private:
        std::vector<std::unique_ptr<property_list_funcbased_property_base>> props_;
    };
}

