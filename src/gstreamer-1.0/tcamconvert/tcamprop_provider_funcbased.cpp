
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
    property_desc desc_;

    prop_range range_;
    get_flags_func get_flags_func_;

    virtual outcome::result<prop_value> get() = 0;
    virtual std::error_code set(prop_value val) = 0;

public:
    property_list_funcbased_property_base(const property_desc& d,
                                          const prop_range& range,
                                          get_flags_func flags_func) noexcept
        : desc_ { d }, range_(range), get_flags_func_(flags_func)
    {
    }

    virtual ~property_list_funcbased_property_base() = default;

    property_desc get_property_desc() final
    {
        return desc_;
    }
    outcome::result<prop_range> get_property_range() final
    {
        return range_;
    }

    outcome::result<prop_flags> get_property_flags() final
    {
        if (get_flags_func_)
        {
            return get_flags_func_();
        }
        return desc_.type_flags;
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
        return desc_.prop_name == name;
    }
    bool operator!=(std::string_view name) const noexcept
    {
        return desc_.prop_name != name;
    }
};

namespace
{
struct property_real : property_list_funcbased_property_base
{
    set_value_func<double> set_value;
    get_value_func<double> get_value;

    property_real(const property_desc& d,
                  set_value_func<double> s,
                  get_value_func<double> g,
                  prop_range range,
                  get_flags_func flags_func)
        : property_list_funcbased_property_base { d, range, flags_func }, set_value { s },
          get_value { g }
    {
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
    set_value_func<int> set_value;
    get_value_func<int> get_value;

    property_integer(const property_desc& d,
                     set_value_func<int> s,
                     get_value_func<int> g,
                     prop_range range,
                     get_flags_func flags_func)
        : property_list_funcbased_property_base { d, range, flags_func }, set_value { s },
          get_value { g }
    {
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

    property_boolean(const property_desc& d,
                     set_value_func<bool> s,
                     get_value_func<bool> g,
                     bool def,
                     get_flags_func flags_func)
        : property_list_funcbased_property_base { d, prop_range::make_boolean(def), flags_func },
          set_value { s }, get_value { g }
    {
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

    property_menu(const property_desc& d,
                  set_value_func<int> s,
                  get_value_func<int> g,
                  int def,
                  get_flags_func flags_func)
        : property_list_funcbased_property_base { d,
                                                  prop_range_integer { 0,
                                                                       (int)d.menu_entries.size(),
                                                                       def,
                                                                       1 },
                                                  flags_func },
          set_value { s }, get_value { g }
    {
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
        : property_list_funcbased_property_base { d, prop_range::make_button(), flags_func },
          set_value { s }
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
} // namespace prop_system

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

std::vector<property_desc> tcamprop_system::property_list_funcbased::get_property_list()
{
    std::vector<property_desc> rval;
    for (auto&& p : props_) { rval.push_back(p->get_property_desc()); }
    return rval;
}

void tcamprop_system::property_list_funcbased::register_double(const property_desc& dsc,
                                                           set_value_func<double> set,
                                                           get_value_func<double> get,
                                                           const prop_range_real& range)
{
    register_double(dsc, set, get, get_flags_func {}, range);
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

void tcamprop_system::property_list_funcbased::register_menu(const property_desc& dsc,
                                                         set_value_func<int> set,
                                                         get_value_func<int> get,
                                                         int def)
{
    register_menu(dsc, set, get, get_flags_func {}, def);
}

void tcamprop_system::property_list_funcbased::register_menu(const property_desc& dsc,
                                                         set_value_func<int> set,
                                                         get_value_func<int> get,
                                                         get_flags_func get_flags,
                                                         int def)
{
    props_.push_back(std::make_unique<property_menu>(dsc, set, get, def, get_flags));
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
