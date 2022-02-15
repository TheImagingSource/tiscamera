
#pragma once

#include <cstdint>
#include <vector>
#include <string_view>
#include <array>
#include <string>

namespace tcamprop1
{
    enum class prop_type
    {
        Boolean,
        Integer,
        Float,
        Command,
        Enumeration,
        String,
    };
    enum class Visibility_t {
        Beginner,
        Expert,
        Guru,
        Invisible,
    };
    enum class Access_t {
        RO,
        WO,
        RW
    };
    enum class IntRepresentation_t {
        Linear,
        Logarithmic,
        Boolean,
        PureNumber,
        HexNumber,
        IPV4Address,
        MACAddress,
    };
    enum class FloatRepresentation_t {
        Linear,
        Logarithmic,
        PureNumber,
    };

    constexpr const char* to_string( prop_type t ) noexcept;
    constexpr const char* to_string( Visibility_t t ) noexcept;
    constexpr const char* to_string( Access_t t ) noexcept;
    constexpr const char* to_string( IntRepresentation_t t ) noexcept;
    constexpr const char* to_string( FloatRepresentation_t t ) noexcept;

    struct prop_static_info
    {
        std::string_view    name;   // the GenICam name of this property

        std::string_view    iccategory;     // IC category, or 'short' category name (GenICam categories are a tree, so we just mention a short category name here
        std::string_view    display_name;   // Display (or user-friendly) name of this property
        std::string_view    description;    // A longer format description of this property

        Visibility_t        visibility = Visibility_t::Beginner;    // Visibility of this property

        Access_t            access = Access_t::RW;     // Currently not used
    };

    /** A copy of the prop_static_info structure, but uses std::strings for its string members.
     * Keep this aligned with the prop_static_info struct.
     */
    struct prop_static_info_str
    {
        prop_static_info_str() noexcept = default;
        prop_static_info_str( const prop_static_info& str )
            : name{ str.name }, iccategory{ str.iccategory }, display_name{ str.display_name }, description{ str.description }, visibility{ str.visibility }, access{ str.access }
        {}

        auto to_prop_static_info() const noexcept -> prop_static_info {
            return { name, iccategory, display_name, description, visibility, access };
        }

        std::string     name;

        std::string    iccategory;     // IC category, or 'short' category name (GenICam categories are a tree, so we just mention a short category name here
        std::string    display_name;   // Display (or user-friendly) name of this property
        std::string    description;    // A longer format description of this property

        Visibility_t        visibility = Visibility_t::Beginner;
        Access_t            access = Access_t::RW;
    };

    struct prop_range_float
    {
        double min = 0;
        double max = 0;
        double stp = 1.;
    };

    struct prop_range_integer
    {
        int64_t min = 0;
        int64_t max = 0;
        int64_t stp = 1;
    };

    struct prop_range_enumeration
    {
        prop_range_enumeration() = default;
        prop_range_enumeration( std::vector<std::string>&& arr ) : enum_entries{ std::move( arr ) } {}
        prop_range_enumeration( const std::vector<std::string>& arr ) : enum_entries{ arr } {}

        template<size_t N>
        prop_range_enumeration( const std::array<std::string_view, N>&arr )
            : enum_entries{ arr.begin(), arr.end() }
        {
        }

        std::vector<std::string>   enum_entries;    // entries, beware that we should not change this to string_views

        /** Fetches the entry at index. Returns an empty string_view on out-of-range values */
        auto get_entry_at( int index ) const noexcept -> std::string_view {
            if( index < 0 || index >= static_cast<int>(enum_entries.size()) ) {
                return {};
            }
            return enum_entries[index];
        }
        /** Finds the index of the passed elem. Returns -1 when not found. */
        auto get_index_of( std::string_view elem ) const noexcept -> int {
            for( int idx = 0; idx < static_cast<int>(enum_entries.size()); ++idx ) {
                if( enum_entries[idx] == elem ) {
                    return idx;
                }
            }
            return -1;
        }
    };

    constexpr const char* to_string( prop_type t ) noexcept
    {
        switch( t )
        {
        case prop_type::Boolean:        return "Boolean";
        case prop_type::Integer:        return "Integer";
        case prop_type::Float:          return "Float";
        case prop_type::Command:        return "Command";
        case prop_type::Enumeration:    return "Enumeration";
        case prop_type::String:         return "String";
        };
        return nullptr;
    }

    constexpr const char* to_string( Visibility_t t ) noexcept
    {
        switch( t )
        {
        case tcamprop1::Visibility_t::Beginner: return "Beginner";
        case tcamprop1::Visibility_t::Expert:   return "Expert";
        case tcamprop1::Visibility_t::Guru:     return "Guru";
        case tcamprop1::Visibility_t::Invisible:return "Invisible";
        }
        return nullptr;
    }
    constexpr const char* to_string( Access_t t ) noexcept
    {
        switch( t )
        {
        case tcamprop1::Access_t::RO:       return "RO";
        case tcamprop1::Access_t::WO:       return "WO";
        case tcamprop1::Access_t::RW:       return "RW";
        }
        return nullptr;
    }

    constexpr const char* to_string( IntRepresentation_t t ) noexcept
    {
        switch( t )
        {
        case IntRepresentation_t::Linear:       return "Linear";
        case IntRepresentation_t::Logarithmic:  return "Logarithmic";
        case IntRepresentation_t::Boolean:      return "Boolean";
        case IntRepresentation_t::PureNumber:   return "PureNumber";
        case IntRepresentation_t::HexNumber:    return "HexNumber";
        case IntRepresentation_t::IPV4Address:  return "IPV4Address";
        case IntRepresentation_t::MACAddress:   return "MACAddress";
        }
        return "";
    }
    constexpr const char* to_string( FloatRepresentation_t t ) noexcept
    {
        switch( t )
        {
        case FloatRepresentation_t::Linear:         return "Linear";
        case FloatRepresentation_t::Logarithmic:    return "Logarithmic";
        case FloatRepresentation_t::PureNumber:     return "PureNumber";

        }
        return nullptr;
    }


} // namespace tcamprop1
