
#pragma once

#include <optional>
#include <outcome/result.hpp>
#include <string_view>
#include <vector>

#include "tcamprop_system_base.h"

namespace tcamprop_system
{
struct property_desc
{
    std::string_view prop_name;

    prop_type type;

    std::string_view prop_category;
    std::string_view prop_group;

    std::vector<const char*> menu_entries = {};

    prop_flags type_flags = prop_flags::def_flags;
};

class property_interface
{
public:
    virtual ~property_interface() = default;

    virtual auto get_property_desc() -> tcamprop_system::property_desc = 0;

    virtual auto get_property_range() -> outcome::result<tcamprop_system::prop_range> = 0;
    virtual auto get_property_flags() -> outcome::result<tcamprop_system::prop_flags> = 0;
    virtual auto get_property_value() -> outcome::result<tcamprop_system::prop_value> = 0;
    virtual auto set_property_value(tcamprop_system::prop_value new_value) -> std::error_code = 0;
};


class property_list_interface
{
public:
    virtual ~property_list_interface() = default;

    virtual auto get_property_list() -> std::vector<tcamprop_system::property_desc> = 0;
    virtual auto find_property(std::string_view name) -> property_interface* = 0;
};
} // namespace prop_system
