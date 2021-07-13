
#include "tcamprop_provider_funcbased.h"

#include <algorithm>


using namespace tcamprop_system;

namespace
{
template<typename T> using set_value_func = std::function<std::error_code(T)>;
template<typename T> using get_value_func = std::function<outcome::result<T>()>;
using get_flags_func = property_list_funcbased::get_flags_func;

using set_func_button = std::function<std::error_code()>;
} // namespace

namespace tcamprop_system
{
struct property_list_funcbased_property_base : public property_interface
{
private:
    std::string prop_name_;
    std::string prop_category_;
    std::string prop_group_;

    prop_type prop_type_ = {};

    get_flags_func get_flags_func_;
    prop_flags static_type_flags_ = prop_flags::noflags;

    virtual outcome::result<prop_value> get() = 0;
    virtual std::error_code set(prop_value val) = 0;

public:
    property_list_funcbased_property_base(const property_desc& d,
                                          get_flags_func flags_func) noexcept
        : prop_name_ { d.prop_name }, prop_category_ { d.prop_category },
          prop_group_ { d.prop_group }, prop_type_ { d.type },
          get_flags_func_(flags_func), static_type_flags_ { d.type_flags }
    {
    }
    property_list_funcbased_property_base(const property_info& nfo,
                                          get_flags_func&& flags_func) noexcept
        : prop_name_ { nfo.prop_name }, prop_category_ { nfo.prop_category },
          prop_group_ { nfo.prop_group }, prop_type_ { nfo.type },
          get_flags_func_(std::move(flags_func))
    {
    }

    virtual ~property_list_funcbased_property_base() = default;

    property_info get_property_info() final
    {
        return { prop_name_, prop_type_, prop_category_, prop_group_ };
    }
    outcome::result<prop_range> get_property_range() override
    {
        return std::make_error_code(std::errc::invalid_argument);
    }

    outcome::result<prop_flags> get_property_flags() final
    {
        if (get_flags_func_)
        {
            return get_flags_func_();
        }
        return static_type_flags_;
    }
    outcome::result<prop_value> get_property_value() final
    {
        return get();
    }
    std::error_code set_property_value(prop_value new_value) final
    {
        return set(new_value);
    }

    bool operator==(std::string_view name) const noexcept
    {
        return prop_name_ == name;
    }
    bool operator!=(std::string_view name) const noexcept
    {
        return prop_name_ != name;
    }
};

namespace
{
struct property_real : property_list_funcbased_property_base
{
private:
    prop_range_real range_;

    set_value_func<double> set_value;
    get_value_func<double> get_value;

public:
    property_real(const property_desc& d,
                  set_value_func<double> s,
                  get_value_func<double> g,
                  prop_range_real range,
                  get_flags_func flags_func)
        : property_list_funcbased_property_base { d, flags_func }, range_ { range },
          set_value { s }, get_value { g }
    {
        assert(d.type == prop_type::real);
    }
    property_real(const property_info& nfo,
                  prop_range_real range,
                  set_value_func<double>&& s,
                  get_value_func<double>&& g,
                  get_flags_func&& flags_func)
        : property_list_funcbased_property_base { nfo, std::move(flags_func) }, range_ { range },
          set_value { std::move(s) }, get_value { std::move(g) }
    {
        assert(nfo.type == prop_type::real);
    }

    outcome::result<prop_range> get_property_range() final
    {
        return range_;
    }

    outcome::result<prop_value> get() final
    {
        auto res = get_value();
        if (res.has_value())
        {
            return prop_value(res.value());
        }
        return res.error();
    }
    std::error_code set(prop_value val) final
    {
        return set_value(val.real);
    }
};

struct property_integer : property_list_funcbased_property_base
{
    prop_range_integer range_;

    set_value_func<int> set_value;
    get_value_func<int> get_value;

    property_integer(const property_desc& d,
                     set_value_func<int> s,
                     get_value_func<int> g,
                     prop_range_integer range,
                     get_flags_func flags_func)
        : property_list_funcbased_property_base { d, flags_func }, range_ { range },
          set_value { s }, get_value { g }
    {
        assert(d.type == prop_type::integer);
    }
    property_integer(const property_info& d,
                     prop_range_integer range,
                     set_value_func<int>&& s,
                     get_value_func<int>&& g,
                     get_flags_func&& flags_func)
        : property_list_funcbased_property_base { d, std::move(flags_func) }, range_ { range },
          set_value { std::move(s) }, get_value { std::move(g) }
    {
        assert(d.type == prop_type::integer);
    }

    outcome::result<prop_range> get_property_range() final
    {
        return range_;
    }
    outcome::result<prop_value> get() final
    {
        auto res = get_value();
        if (res.has_value())
        {
            return prop_value(res.value());
        }
        return res.error();
    }
    std::error_code set(prop_value val) final
    {
        return set_value(val.integer);
    }
};

struct property_boolean : property_list_funcbased_property_base
{
    set_value_func<bool> set_value;
    get_value_func<bool> get_value;

    bool default_value_ = false;

    property_boolean(const property_desc& d,
                     set_value_func<bool> s,
                     get_value_func<bool> g,
                     bool def,
                     get_flags_func flags_func)
        : property_list_funcbased_property_base { d, flags_func }, set_value { s }, get_value { g },
          default_value_(def)
    {
        assert(d.type == prop_type::boolean);
    }
    outcome::result<prop_range> get_property_range() final
    {
        return prop_range::make_boolean(default_value_);
    }
    outcome::result<prop_value> get() final
    {
        auto res = get_value();
        if (res.has_value())
        {
            return prop_value(res.value() ? 1 : 0);
        }
        return res.error();
    }
    std::error_code set(prop_value val) final
    {
        return set_value(val.integer != 0);
    }
};

struct property_menu : property_list_funcbased_property_base
{
    set_value_func<int> set_value;
    get_value_func<int> get_value;

    std::vector<std::string> menu_entries_;

    int default_entry_index_ = 0;

    property_menu(const property_desc& d,
                  set_value_func<int> s,
                  get_value_func<int> g,
                  int def,
                  get_flags_func flags_func,
                  const std::vector<std::string>& menu_entries)
        : property_list_funcbased_property_base { d, flags_func }, set_value { s }, get_value { g },
          menu_entries_ { menu_entries }, default_entry_index_(def)
    {
        if (menu_entries_.empty())
        {
            for (auto&& e : d.static_menu_entries)
            {
                if (e.empty())
                {
                    continue;
                }
                menu_entries_.push_back(std::string(e));
            }
        }
    }

    static auto to_string_vec(const std::vector<std::string_view>& menu_entries)
    {
        return std::vector<std::string> { menu_entries.begin(), menu_entries.end() };
    }

    property_menu(const property_info& d,
                  const std::vector<std::string_view>& menu_entries,
                  int def,
                  set_value_func<int>&& s,
                  get_value_func<int>&& g,
                  get_flags_func&& flags_func)
        : property_list_funcbased_property_base { d, std::move(flags_func) },
          set_value { std::move(s) }, get_value { std::move(g) }, menu_entries_ { to_string_vec(
                                                                      menu_entries) },
          default_entry_index_(def)
    {
    }


    outcome::result<prop_range> get_property_range() final
    {
        prop_range tmp;
        for (auto&& e : menu_entries_) { tmp.menu_entries.push_back(e); }
        tmp.val_def.integer = default_entry_index_;
        return tmp;
    }

    outcome::result<prop_value> get() final
    {
        auto res = get_value();
        if (res.has_value())
        {
            return prop_value(res.value());
        }
        return res.error();
    }
    std::error_code set(prop_value val) final
    {
        return set_value(val.integer);
    }
};


struct property_button : property_list_funcbased_property_base
{
    set_func_button set_value;

    property_button(const property_desc& d, set_func_button s, get_flags_func flags_func)
        : property_list_funcbased_property_base { d, flags_func }, set_value { s }
    {
    }

    outcome::result<prop_value> get() final
    {
        return prop_value {};
    }
    std::error_code set(prop_value) final
    {
        return set_value();
    }
};

} // namespace
} // namespace tcamprop_system

tcamprop_system::property_list_funcbased::property_list_funcbased() = default;
tcamprop_system::property_list_funcbased::~property_list_funcbased() = default;

tcamprop_system::property_interface* tcamprop_system::property_list_funcbased::find_property(
    std::string_view name)
{
    auto it = std::find_if(props_.begin(), props_.end(), [name](auto& p) { return *p == name; });
    if (it == props_.end())
    {
        return nullptr;
    }
    return it->get();
}

std::vector<std::string_view> tcamprop_system::property_list_funcbased::get_property_list()
{
    std::vector<std::string_view> rval;
    for (auto&& p : props_) { rval.push_back(p->get_property_info().prop_name); }
    return rval;
}

void tcamprop_system::property_list_funcbased::register_double(const property_desc& dsc,
                                                               set_value_func<double> set,
                                                               get_value_func<double> get,
                                                               const prop_range_real& range)
{
    register_double(dsc, set, get, get_flags_func {}, range);
}

void property_list_funcbased::register_double(const property_info& nfo,
                                              const prop_range_real& range,
                                              set_value_func<double> set,
                                              get_value_func<double> get,
                                              prop_flags def_flags)
{
    props_.push_back(std::make_unique<property_real>(
        nfo, range, std::move(set), std::move(get), [def_flags] { return def_flags; }));
}

void property_list_funcbased::register_double(const property_info& nfo,
                                              const prop_range_real& range,
                                              set_value_func<double> set,
                                              get_value_func<double> get,
                                              get_flags_func get_fla)
{
    props_.push_back(std::make_unique<property_real>(
        nfo, range, std::move(set), std::move(get), std::move(get_fla)));
}

void tcamprop_system::property_list_funcbased::register_double(const property_desc& dsc,
                                                               set_value_func<double> set,
                                                               get_value_func<double> get,
                                                               get_flags_func get_fla,
                                                               const prop_range_real& range)
{
    props_.push_back(std::make_unique<property_real>(dsc, set, get, range, get_fla));
}

void tcamprop_system::property_list_funcbased::register_integer(const property_desc& dsc,
                                                                set_value_func<int> set,
                                                                get_value_func<int> get,
                                                                const prop_range_integer& range)
{
    register_integer(dsc, set, get, get_flags_func {}, range);
}

void property_list_funcbased::register_integer(const property_info& nfo,
                                               const prop_range_integer& range,
                                               set_value_func<int> set,
                                               get_value_func<int> get,
                                               get_flags_func get_flags)
{
    props_.push_back(std::make_unique<property_integer>(
        nfo, range, std::move(set), std::move(get), std::move(get_flags)));
}

void property_list_funcbased::register_integer(const property_info& nfo,
                                               const prop_range_integer& range,
                                               set_value_func<int> set,
                                               get_value_func<int> get,
                                               prop_flags def_flags /*= prop_flags::def_flags */)
{
    register_integer(
        nfo, range, std::move(set), std::move(get), std::move([def_flags] { return def_flags; }));
}

void tcamprop_system::property_list_funcbased::register_integer(const property_desc& dsc,
                                                                set_value_func<int> set,
                                                                get_value_func<int> get,
                                                                get_flags_func get_flags,
                                                                const prop_range_integer& range)
{
    props_.push_back(std::make_unique<property_integer>(dsc, set, get, range, get_flags));
}

void tcamprop_system::property_list_funcbased::register_boolean(const property_desc& dsc,
                                                                set_value_func<bool> set,
                                                                get_value_func<bool> get,
                                                                bool def)
{
    register_boolean(dsc, set, get, get_flags_func {}, def);
}

void tcamprop_system::property_list_funcbased::register_boolean(const property_desc& dsc,
                                                                set_value_func<bool> set,
                                                                get_value_func<bool> get,
                                                                get_flags_func get_flags,
                                                                bool def)
{
    props_.push_back(std::make_unique<property_boolean>(dsc, set, get, def, get_flags));
}

void tcamprop_system::property_list_funcbased::register_menu(
    const property_desc& dsc,
    set_value_func<int> set,
    get_value_func<int> get,
    int def,
    const std::vector<std::string>& menu_entries)
{
    register_menu(dsc, set, get, get_flags_func {}, def, menu_entries);
}

void property_list_funcbased::register_menu(const property_info& nfo,
                                            const std::vector<std::string_view>& menu_entries,
                                            int default_menu_entry,
                                            set_value_func<int> set,
                                            get_value_func<int> get,
                                            prop_flags def_flags)
{
    props_.push_back(std::make_unique<property_menu>(
        nfo, menu_entries, default_menu_entry, std::move(set), std::move(get), [def_flags] {
            return def_flags;
        }));
}

void property_list_funcbased::register_menu(const property_info& nfo,
                                            const std::vector<std::string_view>& menu_entries,
                                            int default_menu_entry,
                                            set_value_func<int> set,
                                            get_value_func<int> get,
                                            get_flags_func get_flags)
{
    props_.push_back(std::make_unique<property_menu>(nfo,
                                                     menu_entries,
                                                     default_menu_entry,
                                                     std::move(set),
                                                     std::move(get),
                                                     std::move(get_flags)));
}

void property_list_funcbased::register_menu(const property_desc& dsc,
                                            set_value_func<int> set,
                                            get_value_func<int> get,
                                            int def)
{
    register_menu(dsc, set, get, def, std::vector<std::string> {});
}

void tcamprop_system::property_list_funcbased::register_menu(
    const property_desc& dsc,
    set_value_func<int> set,
    get_value_func<int> get,
    get_flags_func get_flags,
    int def,
    const std::vector<std::string>& menu_entries)
{
    props_.push_back(std::make_unique<property_menu>(dsc, set, get, def, get_flags, menu_entries));
}

void tcamprop_system::property_list_funcbased::register_button(const property_desc& dsc,
                                                               set_func_button set)
{
    register_button(dsc, set, get_flags_func {});
}

void tcamprop_system::property_list_funcbased::register_button(const property_desc& dsc,
                                                               set_func_button set,
                                                               get_flags_func get_flags)
{
    props_.push_back(std::make_unique<property_button>(dsc, set, get_flags));
}
