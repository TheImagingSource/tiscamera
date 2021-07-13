
#pragma once

#include "tcamprop_system_base.h"

#include <array>
#include <outcome/result.hpp>
#include <string_view>
#include <vector>

namespace tcamprop_system
{
struct property_desc
{
    std::string_view prop_name;

    prop_type type;

    std::string_view prop_category;
    std::string_view prop_group;

    std::array<std::string_view, 4> static_menu_entries = {};

    prop_flags type_flags = prop_flags::def_flags;
};

struct property_info
{
    std::string_view prop_name;

    prop_type type;

    std::string_view prop_category;
    std::string_view prop_group;
};

class property_interface
{
public:
    virtual ~property_interface() = default;

    virtual auto get_property_info() -> tcamprop_system::property_info = 0;

    virtual auto get_property_range() -> outcome::result<tcamprop_system::prop_range> = 0;
    virtual auto get_property_flags() -> outcome::result<tcamprop_system::prop_flags> = 0;
    virtual auto get_property_value() -> outcome::result<tcamprop_system::prop_value> = 0;
    virtual auto set_property_value(tcamprop_system::prop_value new_value) -> std::error_code = 0;
};


class property_list_interface
{
public:
    virtual ~property_list_interface() = default;

    virtual auto get_property_list() -> std::vector<std::string_view> = 0;
    virtual auto find_property(std::string_view name) -> tcamprop_system::property_interface* = 0;
};
} // namespace tcamprop_system
