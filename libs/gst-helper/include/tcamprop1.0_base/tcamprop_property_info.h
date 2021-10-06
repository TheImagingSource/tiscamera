
#pragma once

#include "tcamprop_base.h"

namespace tcamprop1
{
    struct prop_static_info_boolean : prop_static_info
    {
        static const constexpr prop_type property_type = prop_type::Boolean;
    };

    struct prop_static_info_integer : prop_static_info
    {
        static const constexpr prop_type property_type = prop_type::Integer;

        std::string_view        unit;
        IntRepresentation_t     representation = IntRepresentation_t::Linear;
    };

    struct prop_static_info_float : prop_static_info
    {
        static const constexpr prop_type property_type = prop_type::Float;

        std::string_view        unit;
        FloatRepresentation_t   representation = FloatRepresentation_t::Linear;
    };

    struct prop_static_info_enumeration : prop_static_info
    {
        static const constexpr prop_type property_type = prop_type::Enumeration;
    };

    struct prop_static_info_command : prop_static_info
    {
        static const constexpr prop_type property_type = prop_type::Command;
    };

    namespace prop_list
    {
        constexpr prop_static_info_float                make_Float( std::string_view name, std::string_view category, std::string_view display_name, std::string_view description,
                                                                    std::string_view unit = {}, FloatRepresentation_t rep = FloatRepresentation_t::Linear, 
                                                                    Visibility_t vis = Visibility_t::Beginner, Access_t access = Access_t::RW ) noexcept {
            return { { name, category, display_name, description, vis, access }, unit, rep };
        }
        constexpr prop_static_info_float                make_Float( std::string_view name, std::string_view category, std::string_view display_name, std::string_view description,
                                                                    Visibility_t vis, Access_t access = Access_t::RW ) noexcept {
            return { { name, category, display_name, description, vis, access }, {}, FloatRepresentation_t::Linear };
        }
        static constexpr prop_static_info_integer       make_Integer( std::string_view name, std::string_view category, std::string_view display_name, std::string_view description,
                                                                      std::string_view unit = {}, IntRepresentation_t rep = IntRepresentation_t::Linear, 
                                                                      Visibility_t vis = Visibility_t::Beginner, Access_t access = Access_t::RW ) noexcept {
            return { { name, category, display_name, description, vis, access }, unit, rep };
        }
        static constexpr prop_static_info_enumeration   make_Enumeration( std::string_view name, std::string_view category, std::string_view display_name, std::string_view description,
                                                                          Visibility_t vis = Visibility_t::Beginner, Access_t access = Access_t::RW ) noexcept {
            return { name, category, display_name, description, vis, access };
        }
        static constexpr prop_static_info_boolean       make_Boolean( std::string_view name, std::string_view category, std::string_view display_name, std::string_view description,
                                                                      Visibility_t vis = Visibility_t::Beginner, Access_t access = Access_t::RW ) noexcept {
            return { name, category, display_name, description, vis, access };
        }
        static constexpr prop_static_info_command       make_Command( std::string_view name, std::string_view category, std::string_view display_name, std::string_view description,
                                                                      Visibility_t vis = Visibility_t::Beginner, Access_t access = Access_t::RW ) noexcept {
            return { name, category, display_name, description, vis, access };
        }
    }

    struct prop_static_info_find_result
    {
        tcamprop1::prop_type    type = prop_type::Boolean;
        const prop_static_info* info_ptr = nullptr;

        explicit operator bool () const noexcept { return info_ptr != nullptr; }
    };

    auto find_prop_static_info( std::string_view name ) noexcept -> prop_static_info_find_result;

} // namespace tcamprop1
