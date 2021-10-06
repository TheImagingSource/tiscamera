
#include "tcamprop1.0_helper/tcamprop_consumer.h"

#include <gst-helper/gvalue_wrapper.h>

#include <tcamprop.h>

namespace
{
    using gvalue_wrapper = gvalue::gvalue_wrapper;


bool  internal_get_value( TcamProp* elem, const char* name, gvalue_wrapper& val )
{
    return tcam_prop_get_tcam_property( elem, name, val.get(), nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr );
}


constexpr auto prop_type_from_string(std::string_view v) noexcept -> std::optional<tcamprop1::prop_type>
{
    if (v == "boolean")
    {
        return tcamprop1::prop_type::Boolean;
    }
    if (v == "integer")
    {
        return tcamprop1::prop_type::Integer;
    }
    if (v == "double")
    {
        return tcamprop1::prop_type::Float;
    }
    if (v == "button")
    {
        return tcamprop1::prop_type::Command;
    }
    if (v == "enum")
    {
        return tcamprop1::prop_type::Enumeration;
    }
    return std::nullopt;
}

}

bool  tcamprop1::get_value( TcamProp* elem, const char* name, _GValue& val )
{
    if( !tcam_prop_get_tcam_property( elem, name, &val, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr ) )
    {
        return false;
    }
    return true;
}

std::optional<std::string> tcamprop1::get_category( TcamProp* elem, const char* name )
{
    gvalue_wrapper cat = {};
    if( !tcam_prop_get_tcam_property( elem, name, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, cat.get(), nullptr ) )
    {
        return {};
    }
    if( cat.type() != G_TYPE_STRING ) {
        return {};
    }
    return cat.get_string();
}

auto tcamprop1::get_prop_type(TcamProp* elem, const char* name) -> std::optional<tcamprop1::prop_type>
{
    gvalue_wrapper type = {};
    if (!tcam_prop_get_tcam_property(elem,
                                     name,
                                     nullptr,
                                     nullptr,
                                     nullptr,
                                     nullptr,
                                     nullptr,
                                     type.get(),
                                     nullptr,
                                     nullptr,
                                     nullptr))
    {
        return {};
    }
    if( type.type() != G_TYPE_STRING ) {
        return {};
    }
    return prop_type_from_string( type.get_string_view() );
}

static std::vector<std::string>    conv_GSList_string_list( GSList* lst )
{
    std::vector<std::string> rval;

    for( auto ptr = lst; ptr != nullptr; ptr = ptr->next )
    {
        rval.push_back( static_cast<const char*>(ptr->data) );
    }

    return rval;
}

TcamProp* tcamprop1::to_TcamProp(_GstElement* elem) noexcept {
    return TCAM_PROP( elem );
}

TcamProp* tcamprop1::to_TcamProp( _GstElement& elem ) noexcept {
    return TCAM_PROP( &elem );
}

std::vector<std::string> tcamprop1::get_property_names( TcamProp* elem )
{
    GSList* prop_name_gslist = tcam_prop_get_tcam_property_names( elem );
    auto prop_name_list = conv_GSList_string_list( prop_name_gslist );
    g_slist_free_full( prop_name_gslist, g_free );

    return prop_name_list;
}

bool  tcamprop1::has_property( TcamProp* elem, const char* name )
{
    gvalue_wrapper val = {};
    bool res = internal_get_value( elem, name, val );
    return res;
}

bool tcamprop1::has_property(TcamProp* elem, const char* name, prop_type type)
{
    auto res_type = get_prop_type( elem, name );
    return !res_type.has_value() ? false : res_type.value() == type;
}

template<>
auto  tcamprop1::get_range<tcamprop1::prop_range_integer>( TcamProp* elem, const char* name ) -> std::optional<prop_range_integer>
{
    gvalue_wrapper min, max, stp, def;
    if( !tcam_prop_get_tcam_property( elem, name, nullptr, min.get(), max.get(), def.get(), stp.get(), nullptr, nullptr, nullptr, nullptr ) )
    {
        return {};
    }

    if( min.type() != G_TYPE_INT ) {
        return {};
    }
    return prop_range_integer{
        min.get_int(),
        max.get_int(),
        def.get_int(),
        stp.get_int()
    };
}

template<>
auto  tcamprop1::get_range<tcamprop1::prop_range_float>( TcamProp* elem, const char* name ) -> std::optional<prop_range_float>
{
    gvalue_wrapper min, max, stp, def;
    if( !tcam_prop_get_tcam_property( elem, name, nullptr, min.get(), max.get(), def.get(), stp.get(), nullptr, nullptr, nullptr, nullptr ) )
    {
        return {};
    }
    if( min.type() != G_TYPE_DOUBLE ) {
        return {};
    }
    return prop_range_float{
        min.get_double(),
        max.get_double(),
        def.get_double(),
        stp.get_double()
    };
}

template<>
auto  tcamprop1::get_value<bool>( TcamProp* elem, const char* name ) -> std::optional<bool>
{
    gvalue_wrapper val = {};
    if( !internal_get_value( elem, name, val ) )
    {
        return {};
    }
    return val.get_typed_opt<bool>();
}

template<>
auto  tcamprop1::get_value<double>( TcamProp* elem, const char* name ) -> std::optional<double>
{
    gvalue_wrapper val = {};
    if( !internal_get_value( elem, name, val ) )
    {
        return {};
    }
    return val.fetch_typed<double>();
}

template<>
auto  tcamprop1::get_value<int>( TcamProp* elem, const char* name ) -> std::optional<int>
{
    gvalue_wrapper val = {};
    if( !internal_get_value( elem, name, val ) ) {
        return {};
    }
    return val.fetch_typed<int>();
}

template<>
auto  tcamprop1::get_value<int64_t>( TcamProp* elem, const char* name ) -> std::optional<int64_t>
{
    gvalue_wrapper val = {};
    if( !internal_get_value( elem, name, val ) ) {
        return {};
    }
    return val.fetch_typed<int64_t>();
}

auto tcamprop1::get_menu_items( TcamProp* prop, const char* menu_name ) -> std::vector<std::string>
{
    GSList* ptr_root = tcam_prop_get_tcam_menu_entries( prop, menu_name );

    auto rval = conv_GSList_string_list( ptr_root );

    g_slist_free_full( ptr_root, g_free );

    return rval;
}

bool tcamprop1::set_value( _TcamProp* elem, const char* name, const _GValue* value_to_set )
{
    return tcam_prop_set_tcam_property( elem, name, value_to_set );
}

bool tcamprop1::set_value( _TcamProp* elem, const char* name, int value_to_set )
{
    gvalue_wrapper val = gvalue_wrapper::make_value( value_to_set );
    return tcam_prop_set_tcam_property( elem, name, val.get() );
}

bool tcamprop1::set_value( _TcamProp* elem, const char* name, bool value_to_set )
{
    gvalue_wrapper val = gvalue_wrapper::make_value( value_to_set );
    return tcam_prop_set_tcam_property( elem, name, val.get() );
}

bool    tcamprop1::set_value( _TcamProp* elem, const char* name, int64_t value_to_set )
{
    gvalue_wrapper val = gvalue_wrapper::make_value( (int)value_to_set );
    return tcam_prop_set_tcam_property( elem, name, val.get() );
}

bool    tcamprop1::set_value( _TcamProp* elem, const char* name, double value_to_set )
{
    gvalue_wrapper val = gvalue_wrapper::make_value( value_to_set );
    return tcam_prop_set_tcam_property( elem, name, val.get() );
}

