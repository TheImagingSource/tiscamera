
#pragma once

//#include <gst/gst.h>

#include "tcamprop_system_base.h"

#include <optional>
#include <string>
#include <vector>

struct TcamProp;
struct _GstElement;

namespace tcamprop_system
{
TcamProp* to_TcamProp(_GstElement* elem) noexcept;

std::vector<std::string> get_property_names(TcamProp* elem);

bool has_property(TcamProp* elem, const char* name);
bool has_property( TcamProp* elem, const char* name, prop_type type );
bool get_value(TcamProp* elem, const char* name, GValue& val);
auto get_flags(TcamProp* elem, const char* name) -> std::optional<prop_flags>;
auto get_category(TcamProp* elem, const char* name) -> std::optional<std::string>;
auto get_prop_type( TcamProp* elem, const char* name )->std::optional<prop_type>;

template<typename T> auto get_range(TcamProp* elem, const char* name) -> std::optional<T> = delete;

template<>
auto get_range<prop_range_integer>(TcamProp* elem, const char* name)
    -> std::optional<prop_range_integer>;
template<>
auto get_range<prop_range_real>(TcamProp* elem, const char* name) -> std::optional<prop_range_real>;

template<typename T> auto get_value(TcamProp* elem, const char* name) -> std::optional<T> = delete;
template<> auto get_value<bool>(TcamProp* elem, const char* name) -> std::optional<bool>;
template<> auto get_value<double>(TcamProp* elem, const char* name) -> std::optional<double>;
template<> auto get_value<int>(TcamProp* elem, const char* name) -> std::optional<int>;

void set_value(TcamProp* elem, const char* name, bool value_to_set);
void set_value(TcamProp* elem, const char* name, int value_to_set);
void set_value(TcamProp* elem, const char* name, double value_to_set);

auto get_menu_items(TcamProp* prop, const char* menu_name) -> std::vector<std::string>;
} // namespace tcamprop_system
