
#include "tcamprop_consumer.h"
//#include "../../lib/tcamprop/src/tcamprop.h"
#include <tcamprop.h>

namespace
{
struct gvalue_wrapper
{
    gvalue_wrapper() = default;

    explicit gvalue_wrapper(double value_to_set) noexcept
    {
        g_value_init(&value, G_TYPE_DOUBLE);
        g_value_set_double(&value, value_to_set);
    }
    explicit gvalue_wrapper(int value_to_set) noexcept
    {
        g_value_init(&value, G_TYPE_INT);
        g_value_set_int(&value, value_to_set);
    }
    explicit gvalue_wrapper(bool value_to_set) noexcept
    {
        g_value_init(&value, G_TYPE_BOOLEAN);
        g_value_set_boolean(&value, value_to_set ? TRUE : FALSE);
    }
    ~gvalue_wrapper()
    {
        g_value_unset(&value);
    }

    operator GValue*() noexcept
    {
        return &value;
    }
    GValue* operator&() noexcept
    {
        return &value;
    }

private:
    GValue value = {};
};

bool internal_get_value(TcamProp* elem, const char* name, gvalue_wrapper& val)
{
    return tcam_prop_get_tcam_property(
        elem, name, &val, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
}


constexpr auto prop_type_from_string(std::string_view v) noexcept
    -> std::optional<tcamprop_system::prop_type>
{
    using namespace tcamprop_system;

    if (v == "boolean")
    {
        return prop_type::boolean;
    }
    if (v == "integer")
    {
        return prop_type::integer;
    }
    if (v == "double")
    {
        return prop_type::real;
    }
    if (v == "button")
    {
        return prop_type::button;
    }
    if (v == "enum")
    {
        return prop_type::menu;
    }
    return {};
}

} // namespace

bool tcamprop_system::get_value(TcamProp* elem, const char* name, _GValue& val)
{
    if (!tcam_prop_get_tcam_property(elem,
                                     name,
                                     &val,
                                     nullptr,
                                     nullptr,
                                     nullptr,
                                     nullptr,
                                     nullptr,
                                     nullptr,
                                     nullptr,
                                     nullptr))
    {
        return false;
    }
    return true;
}

auto tcamprop_system::get_flags(TcamProp* elem, const char* name) -> std::optional<prop_flags>
{
    gvalue_wrapper flags;
    if (!tcam_prop_get_tcam_property(elem,
                                     name,
                                     nullptr,
                                     nullptr,
                                     nullptr,
                                     nullptr,
                                     nullptr,
                                     nullptr,
                                     &flags,
                                     nullptr,
                                     nullptr))
    {
        return {};
    }
    if (G_VALUE_TYPE(&flags) != G_TYPE_INT)
    {
        return {};
    }
    return static_cast<prop_flags>(g_value_get_int(&flags));
}

std::optional<std::string> tcamprop_system::get_category(TcamProp* elem, const char* name)
{
    gvalue_wrapper cat = {};
    if (!tcam_prop_get_tcam_property(elem,
                                     name,
                                     nullptr,
                                     nullptr,
                                     nullptr,
                                     nullptr,
                                     nullptr,
                                     nullptr,
                                     nullptr,
                                     &cat,
                                     nullptr))
    {
        return {};
    }
    if (G_VALUE_TYPE(&cat) != G_TYPE_STRING)
    {
        return {};
    }
    auto str = g_value_get_string(&cat);
    if (str == nullptr)
    {
        return {};
    }
    return std::string(str);
}

auto tcamprop_system::get_prop_type(TcamProp* elem, const char* name)
    -> std::optional<tcamprop_system::prop_type>
{
    gvalue_wrapper type = {};
    if (!tcam_prop_get_tcam_property(elem,
                                     name,
                                     nullptr,
                                     nullptr,
                                     nullptr,
                                     nullptr,
                                     nullptr,
                                     &type,
                                     nullptr,
                                     nullptr,
                                     nullptr))
    {
        return {};
    }
    if (G_VALUE_TYPE(&type) != G_TYPE_STRING)
    {
        return {};
    }
    auto str = g_value_get_string(&type);
    if (str == nullptr)
    {
        return {};
    }
    return prop_type_from_string(str);
}

static std::vector<std::string> conv_GSList_string_list(GSList* lst)
{
    std::vector<std::string> rval;

    for (auto ptr = lst; ptr != nullptr; ptr = ptr->next)
    {
        rval.push_back(static_cast<const char*>(ptr->data));
    }

    return rval;
}

TcamProp* tcamprop_system::to_TcamProp(_GstElement* elem) noexcept
{
    return TCAM_PROP(elem);
}

std::vector<std::string> tcamprop_system::get_property_names(TcamProp* elem)
{
    GSList* prop_name_gslist = tcam_prop_get_tcam_property_names(elem);
    auto prop_name_list = conv_GSList_string_list(prop_name_gslist);
    g_slist_free_full(prop_name_gslist, g_free);

    return prop_name_list;
}

bool tcamprop_system::has_property(TcamProp* elem, const char* name)
{
    gvalue_wrapper val = {};
    bool res = internal_get_value(elem, name, val);

    return res;
}

bool tcamprop_system::has_property(TcamProp* elem, const char* name, prop_type type)
{
    auto res_type = get_prop_type(elem, name);
    return !res_type.has_value() ? false : res_type.value() == type;
}

template<>
auto tcamprop_system::get_range<tcamprop_system::prop_range_integer>(TcamProp* elem,
                                                                     const char* name)
    -> std::optional<prop_range_integer>
{
    gvalue_wrapper val = {};
    gvalue_wrapper min = {};
    gvalue_wrapper max = {};
    gvalue_wrapper stp = {};
    gvalue_wrapper def = {};
    if (!tcam_prop_get_tcam_property(
            elem, name, &val, &min, &max, &def, &stp, nullptr, nullptr, nullptr, nullptr))
    {
        return {};
    }

    if (G_VALUE_TYPE(&val) != G_TYPE_INT)
    {
        return {};
    }

    prop_range_integer rval = {};
    rval.min = g_value_get_int(&min);
    rval.max = g_value_get_int(&max);
    rval.def = g_value_get_int(&def);
    rval.stp = g_value_get_int(&stp);
    return rval;
}

template<>
auto tcamprop_system::get_range<tcamprop_system::prop_range_real>(TcamProp* elem, const char* name)
    -> std::optional<prop_range_real>
{
    gvalue_wrapper val = {};
    gvalue_wrapper min = {};
    gvalue_wrapper max = {};
    gvalue_wrapper stp = {};
    gvalue_wrapper def = {};
    if (!tcam_prop_get_tcam_property(
            elem, name, &val, &min, &max, &def, &stp, nullptr, nullptr, nullptr, nullptr))
    {
        return {};
    }

    if (G_VALUE_TYPE(&val) != G_TYPE_DOUBLE)
    {
        return {};
    }

    prop_range_real rval = {};
    rval.min = g_value_get_double(&min);
    rval.max = g_value_get_double(&max);
    rval.def = g_value_get_double(&def);
    rval.stp = g_value_get_double(&stp);
    return rval;
}

template<>
auto tcamprop_system::get_value<bool>(TcamProp* elem, const char* name) -> std::optional<bool>
{
    gvalue_wrapper val = {};
    if (!internal_get_value(elem, name, val))
    {
        return {};
    }

    if (G_VALUE_TYPE(&val) != G_TYPE_BOOLEAN)
    {
        return {};
    }
    return g_value_get_boolean(&val);
}

template<>
auto tcamprop_system::get_value<double>(TcamProp* elem, const char* name) -> std::optional<double>
{
    gvalue_wrapper val = {};
    if (!internal_get_value(elem, name, val))
    {
        return {};
    }
    if (G_VALUE_TYPE(&val) != G_TYPE_DOUBLE)
    {
        return {};
    }
    return g_value_get_double(&val);
}

template<>
auto tcamprop_system::get_value<int>(TcamProp* elem, const char* name) -> std::optional<int>
{
    gvalue_wrapper val = {};
    if (!internal_get_value(elem, name, val))
    {
        return {};
    }
    if (G_VALUE_TYPE(&val) != G_TYPE_INT)
    {
        return {};
    }
    return g_value_get_int(&val);
}

auto tcamprop_system::get_menu_items(TcamProp* prop, const char* menu_name)
    -> std::vector<std::string>
{
    GSList* ptr_root = tcam_prop_get_tcam_menu_entries(prop, menu_name);

    auto rval = conv_GSList_string_list(ptr_root);

    g_slist_free_full(ptr_root, g_free);

    return rval;
}

void tcamprop_system::set_value(TcamProp* elem, const char* name, bool value_to_set)
{
    gvalue_wrapper val { value_to_set };
    tcam_prop_set_tcam_property(elem, name, &val);
}

void tcamprop_system::set_value(TcamProp* elem, const char* name, int value_to_set)
{
    gvalue_wrapper val { value_to_set };
    tcam_prop_set_tcam_property(elem, name, &val);
}

void tcamprop_system::set_value(TcamProp* elem, const char* name, double value_to_set)
{
    gvalue_wrapper val { value_to_set };
    tcam_prop_set_tcam_property(elem, name, &val);
}
