
#pragma once

#include "tcamprop_property_info.h"

#include <outcome/result.hpp>
#include <string_view>
#include <vector>
#include <optional>

namespace tcamprop1
{

struct prop_state
{
    bool        is_implemented = true;      // not implemented properties get not enumerated, and are not accessible
    bool        is_available = true;        // available means, that we can read the property value
    bool        is_locked = false;          // locked means, we currently cannot write the property value

    bool        is_name_hidden = false;     // property name doesn't get exposed in property_list_interface::get_property_list()

    static constexpr auto default_locked( bool locked_value_to_set ) noexcept {
        return prop_state{ true, true, locked_value_to_set };
    }
    static constexpr auto default_hidden() noexcept {
        return prop_state{ true, true, false, true };
    }
};

/**
 * Missing interfaces: 
    IString
    ICategory
 */

/**
 * Reference: http://gitlab.theimagingsource.com/bv/genicam_lib/-/blob/master/include/genicam/genicam_interfaces.h#L58
 * Currently not implemented stuff: 
 * 
 * get_tooltip -> std::string_view
 * get_docuurl -> std::string_view
 * is_deprecated -> bool
 * is_streamable -> bool
 */
class property_interface
{
public:
    virtual ~property_interface() = default;

    virtual auto get_property_name() const noexcept -> std::string_view = 0;
    virtual auto get_property_type() const noexcept -> tcamprop1::prop_type = 0;

    virtual auto get_property_info() const noexcept -> tcamprop1::prop_static_info = 0;
    virtual auto get_property_state( uint32_t flags = 0 ) -> outcome::result<tcamprop1::prop_state> = 0;

    //virtual auto get_extension_entry( std::string_view /*name*/ ) const noexcept -> std::optional<std::string_view> {
    //    return {};
    //}
};
/**
 * Reference: http://gitlab.theimagingsource.com/bv/genicam_lib/-/blob/master/include/genicam/genicam_interfaces.h#L134
 * Currently not implemented stuff:
 * get_pSelected
 */
class property_interface_integer : public property_interface
{
public:
    static constexpr auto itf_type = prop_type::Integer;

    auto get_property_type() const noexcept -> tcamprop1::prop_type final { return prop_type::Integer; }

    virtual auto get_property_range( uint32_t flags = 0 )->outcome::result<tcamprop1::prop_range_integer> = 0;
    virtual auto get_property_default( uint32_t flags = 0 )->outcome::result<int64_t> = 0;

    virtual auto get_property_value( uint32_t flags = 0 )->outcome::result<int64_t> = 0;
    virtual auto set_property_value( int64_t value, uint32_t flags = 0 ) -> std::error_code = 0;

    virtual auto get_representation() const noexcept -> tcamprop1::IntRepresentation_t = 0;
    virtual auto get_unit() const noexcept -> std::string_view = 0;
};
/**
 * Reference: http://gitlab.theimagingsource.com/bv/genicam_lib/-/blob/master/include/genicam/genicam_interfaces.h#L154
 * Currently not implemented:
 * get_display_notation -> DisplayNotation_t
 * get_display_precision -> int64_t
 */
class property_interface_float : public property_interface
{
public:
    static constexpr auto itf_type = prop_type::Float;

    auto get_property_type() const noexcept -> tcamprop1::prop_type final { return prop_type::Float; }

    virtual auto get_property_range( uint32_t flags = 0 )->outcome::result<tcamprop1::prop_range_float> = 0;
    virtual auto get_property_default( uint32_t flags = 0 )->outcome::result<double> = 0;

    virtual auto get_property_value( uint32_t flags = 0 )->outcome::result<double> = 0;
    virtual auto set_property_value( double value, uint32_t flags = 0 ) -> std::error_code = 0;

    virtual auto get_representation() const noexcept -> tcamprop1::FloatRepresentation_t = 0;
    virtual auto get_unit() const noexcept -> std::string_view = 0;
};

/* Missing IEnzmEntry with:
 * get_value -> int64_t
 * get_numeric_value -> optional<double>
 * is_self_cleaing -> bool
 */
/**
 * Reference: http://gitlab.theimagingsource.com/bv/genicam_lib/-/blob/master/include/genicam/genicam_interfaces.h#L190
 * Currently not implemented:
 * get_entries **Real IEnumEntry**
 * get_polling_time
 * get_pSelected
 */
class property_interface_enumeration : public property_interface
{
public:
    static constexpr auto itf_type = prop_type::Enumeration;

    auto get_property_type() const noexcept -> tcamprop1::prop_type final { return prop_type::Enumeration; }

    virtual auto get_property_range( uint32_t flags = 0 )->outcome::result<tcamprop1::prop_range_enumeration> = 0;
    virtual auto get_property_default( uint32_t flags = 0 ) -> outcome::result<std::string_view> = 0;

    virtual auto get_property_value( uint32_t flags = 0 )->outcome::result<std::string_view> = 0;
    virtual auto set_property_value( std::string_view value, uint32_t flags = 0 )->std::error_code = 0;
};
/**
 * Reference: http://gitlab.theimagingsource.com/bv/genicam_lib/-/blob/master/include/genicam/genicam_interfaces.h#L247
 * Currently not implemented:
 * get_pSelected
 */
class property_interface_boolean : public property_interface
{
public:
    static constexpr auto itf_type = prop_type::Boolean;

    auto get_property_type() const noexcept -> tcamprop1::prop_type final { return prop_type::Boolean; }

    virtual auto get_property_default( uint32_t flags = 0 )->outcome::result<bool> = 0;

    virtual auto get_property_value( uint32_t flags = 0 )->outcome::result<bool> = 0;
    virtual auto set_property_value( bool value, uint32_t flags = 0 )->std::error_code = 0;
};
/**
 * Reference: http://gitlab.theimagingsource.com/bv/genicam_lib/-/blob/master/include/genicam/genicam_interfaces.h#L234
 * Currently not implemented:
 * get_polling_time
 * is_done
 */
class property_interface_command : public property_interface
{
public:
    static constexpr auto itf_type = prop_type::Command;

    auto get_property_type() const noexcept -> tcamprop1::prop_type final { return prop_type::Command; }

    virtual auto execute_command( uint32_t flags = 0 )->std::error_code = 0;
};

/**
 * Reference: http://gitlab.theimagingsource.com/bv/genicam_lib/-/blob/master/include/genicam/genicam_interfaces.h#L123
 */
class property_interface_string : public property_interface
{
public:
    static constexpr auto itf_type = prop_type::String;

    auto get_property_type() const noexcept -> tcamprop1::prop_type final
    {
        return prop_type::String;
    }

    virtual auto get_property_value(uint32_t flags = 0) -> outcome::result<std::string> = 0;
    virtual auto set_property_value(std::string_view new_value, uint32_t flags = 0) -> std::error_code = 0;
};

class property_list_interface
{
public:
    virtual ~property_list_interface() = default;

    virtual auto get_property_list() -> std::vector<std::string_view> = 0;
    virtual auto find_property(std::string_view name) ->tcamprop1::property_interface* = 0;

    template<class TItf>
    auto find_property_typed( std::string_view name ) -> TItf*
    {
        auto ptr = find_property( name );
        if( ptr == nullptr ) {
            return nullptr;
        }
        if( ptr->get_property_type() != TItf::itf_type ) {
            return nullptr;
        }
        return static_cast<TItf*>( ptr );
    }
};

} // namespace tcamprop1
