
#pragma once

#include <cstdint>
#include <vector>
#include <string_view>

namespace tcamprop_system
{
enum class prop_type
{
    boolean,
    integer,
    real,
    button,
    menu
};

constexpr const char* to_string(prop_type t) noexcept
{
    switch (t)
    {
        case prop_type::boolean:
            return "boolean";
        case prop_type::integer:
            return "integer";
        case prop_type::real:
            return "double";
        case prop_type::button:
            return "button";
        case prop_type::menu:
            return "enum";
        default:
            return nullptr;
    };
}


enum class prop_flags : uint32_t
{
    noflags = 0x0,
    implemented = 0x1,
    available = 0x2,
    locked = 0x4,

    external =
        0x8, // additional flags like 'External' to indicate library and not camera internal properties?

    hide_from_get_property_names = 0x100,
    debug_property = 0x200,

    public_flags = 0xFF,
    def_flags = implemented | available,
};

constexpr prop_flags operator&(prop_flags lhs, prop_flags rhs) noexcept
{
    return static_cast<prop_flags>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}
constexpr prop_flags operator|(prop_flags lhs, prop_flags rhs) noexcept
{
    return static_cast<prop_flags>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}
constexpr prop_flags& operator&=(prop_flags& lhs, prop_flags rhs) noexcept
{
    return lhs = lhs & rhs;
}
constexpr prop_flags& operator|=(prop_flags& lhs, prop_flags rhs) noexcept
{
    return lhs = lhs | rhs;
}
constexpr bool operator!(prop_flags lhs) noexcept
{
    return lhs == prop_flags::noflags;
}

union prop_value
{
    constexpr prop_value() noexcept : integer(0) {}
    explicit constexpr prop_value(double v) noexcept : real(v) {}
    explicit constexpr prop_value(float v) noexcept : real(v) {}
    explicit constexpr prop_value(int v) noexcept : integer(v) {}
    explicit constexpr prop_value(int64_t v) noexcept : integer(v) {}
    explicit constexpr prop_value(bool v) noexcept : integer(v ? 1 : 0) {}

    double real;
    int64_t integer;

    constexpr bool as_bool() const noexcept
    {
        return integer != 0;
    }
};

struct prop_range_real
{
    double min = 0;
    double max = 0;
    double def = 0;
    double stp = 1.;
};
struct prop_range_integer
{
    int64_t min = 0;
    int64_t max = 0;
    int64_t def = 0;
    int64_t stp = 1;

    constexpr prop_range_real to_real() const noexcept {
        return prop_range_real{ static_cast<double>(min), static_cast<double>(max), static_cast<double>(def), static_cast<double>(stp) };
    }
};
struct prop_range
{
    prop_range() = default;
    prop_range(prop_value min, prop_value max, prop_value def, prop_value stp) noexcept
        : val_min(min), val_max(max), val_def(def), val_stp(stp)
    {
    }
    prop_range(prop_range_integer r) noexcept
        : val_min(r.min), val_max(r.max), val_def(r.def), val_stp(r.stp)
    {
    }
    prop_range(prop_range_real r) noexcept
        : val_min(r.min), val_max(r.max), val_def(r.def), val_stp(r.stp)
    {
    }
    prop_range( std::vector<std::string_view>&& op1, int64_t def_index ) noexcept
        : val_def( def_index ), menu_entries( std::move( op1 ) )
    {
    }

    prop_value val_min;
    prop_value val_max;
    prop_value val_def;
    prop_value val_stp;

    std::vector<std::string_view>   menu_entries;

    constexpr prop_range_real to_real() const noexcept
    {
        return { val_min.real, val_max.real, val_def.real, val_stp.real };
    }
    constexpr prop_range_integer to_integer() const noexcept
    {
        return { val_min.integer, val_max.integer, val_def.integer, val_stp.integer };
    }

    static prop_range make_boolean(bool default_value) noexcept
    {
        return prop_range_integer { 0, 1, default_value ? 1 : 0, 1 };
    }
    static prop_range make_button() noexcept
    {
        return prop_range_integer { 0, 1, 0, 1 };
    }
};

} // namespace tcamprop_system
