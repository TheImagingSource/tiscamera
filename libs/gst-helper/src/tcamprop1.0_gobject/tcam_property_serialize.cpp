
#include "../../include/tcamprop1.0_gobject/tcam_property_serialize.h"


#include <gst-helper/gvalue_wrapper.h>
#include <gst-helper/gvalue_helper.h>
#include <gst-helper/gobject_ptr.h>
#include <tcamprop1.0_gobject/tcam_gerror.h>
#include <tcam-property-1.0.h>
#include <cassert>
#include <fmt/format.h>

namespace
{
    struct gst_apply_entry
    {
        std::string				name;
        gvalue::gvalue_wrapper	val;
    };

    enum class apply_entry_result {
        success,
        failure,
        failure_because_locked,
    };

    auto    apply_entry( TcamPropertyProvider* prop_provider, const gst_apply_entry& entry, const tcamprop1_gobj::report_error_function& report_func ) -> apply_entry_result
    {
        GError* err = nullptr;
        auto ptr = gobject_helper::make_ptr( tcam_property_provider_get_tcam_property( prop_provider, entry.name.c_str(), &err ) );
        if( err )
        {
            report_func( *err, entry.name, entry.val.get() );
            g_error_free( err );
            return apply_entry_result::failure;
        }
        assert( ptr != nullptr );

        if (tcam_property_base_get_access(ptr.get()) == TCAM_PROPERTY_ACCESS_RO)
            return apply_entry_result::failure;
        if (!tcam_property_base_is_available(ptr.get(), nullptr))
            return apply_entry_result::failure_because_locked;

        switch( tcam_property_base_get_property_type( ptr.get() ) )
        {
        case TCAM_PROPERTY_TYPE_BOOLEAN:
        {
            if( auto actual_val = entry.val.fetch_typed<bool>(); actual_val )
            {
                tcam_property_boolean_set_value( TCAM_PROPERTY_BOOLEAN( ptr.get() ), actual_val.value(), &err );
            }
            else
            {
                tcamprop1_gobj::set_gerror( &err, tcamprop1::status::parameter_type_incompatible );
            }
            break;
        }
        case TCAM_PROPERTY_TYPE_INTEGER:
        {
            if( auto actual_val = entry.val.fetch_typed<int64_t>(); actual_val )
            {
                tcam_property_integer_set_value( TCAM_PROPERTY_INTEGER( ptr.get() ), actual_val.value(), &err );
            }
            else
            {
                tcamprop1_gobj::set_gerror( &err, tcamprop1::status::parameter_type_incompatible );
            }
            break;
        }
        case TCAM_PROPERTY_TYPE_FLOAT:
        {
            if( auto actual_val = entry.val.fetch_typed<double>(); actual_val )
            {
                tcam_property_float_set_value( TCAM_PROPERTY_FLOAT( ptr.get() ), actual_val.value(), &err );
            }
            else
            {
                tcamprop1_gobj::set_gerror( &err, tcamprop1::status::parameter_type_incompatible );
            }
            break;
        }
        case TCAM_PROPERTY_TYPE_ENUMERATION:
        {
            if( entry.val.type() != G_TYPE_STRING )	// try to cast the value passed in to a index
            {
                if( auto actual_val = entry.val.fetch_typed<int64_t>(); actual_val )
                {
                    auto lst = tcam_property_enumeration_get_enum_entries( TCAM_PROPERTY_ENUMERATION( ptr.get() ), &err );
                    if( err || lst == nullptr ) {
                        break;
                    }

                    auto index = actual_val.value();

                    auto vec = gvalue::convert_GSList_to_string_vector_consume( lst );
                    if( index < 0 || index >= static_cast<int64_t>(vec.size()) ) {
                        tcamprop1_gobj::set_gerror( &err, TCAM_ERROR_PROPERTY_VALUE_OUT_OF_RANGE, fmt::format( "Failed to find value for index: {}", index ) );
                        break;
                    }

                    tcam_property_enumeration_set_value( TCAM_PROPERTY_ENUMERATION( ptr.get() ), vec[actual_val.value()].c_str(), &err );
                }
            }
            else if( auto actual_val = entry.val.fetch_typed<std::string>(); actual_val )
            {
                tcam_property_enumeration_set_value( TCAM_PROPERTY_ENUMERATION( ptr.get() ), std::string{ actual_val.value() }.c_str(), &err );
            }
            else
            {
                tcamprop1_gobj::set_gerror( &err, tcamprop1::status::parameter_type_incompatible );
            }
            break;
        }
        case TCAM_PROPERTY_TYPE_COMMAND:
        {
            tcam_property_command_set_command( TCAM_PROPERTY_COMMAND( ptr.get() ), &err );
            break;
        }
        case TCAM_PROPERTY_TYPE_STRING:
        {
            if (auto actual_val = entry.val.fetch_typed<std::string>(); actual_val)
            {
                tcam_property_string_set_value(
                    TCAM_PROPERTY_STRING(ptr.get()), actual_val.value().c_str(), &err);
            }
            else
            {
                tcamprop1_gobj::set_gerror(&err, tcamprop1::status::parameter_type_incompatible);
            }
            break;
        }
        }

        if( err )
        {
            if( err->domain == tcam_error_quark() && err->code == TCAM_ERROR_PROPERTY_NOT_WRITEABLE )
            {
                g_error_free( err );

                return apply_entry_result::failure_because_locked;
            }
            report_func( *err, entry.name, entry.val.get() );
            g_error_free( err );

            return apply_entry_result::failure;
        }
        else
        {
            return apply_entry_result::success;
        }
    }

}

void tcamprop1_gobj::apply_properties( TcamPropertyProvider* prop_provider, const GstStructure& data_struct, const report_error_function& report_func )
{
    std::vector<gst_apply_entry>	struct_list;

    auto func = []( GQuark field_id, const GValue* value, gpointer user_data ) -> gboolean
    {
        auto& lst = *reinterpret_cast<std::vector<gst_apply_entry>*>(user_data);
        auto prop_name = g_quark_to_string( field_id );
        if( prop_name == nullptr ) {
            return TRUE;
        }
        lst.push_back( gst_apply_entry{ prop_name, gvalue::gvalue_wrapper{ *value } } );
        return TRUE;
    };

    gst_structure_foreach( &data_struct, func, &struct_list );


    if( struct_list.empty() ) {
        return;
    }

    bool at_least_one_success = false;
    do
    {
        at_least_one_success = false;

        std::vector<gst_apply_entry> retry_list;

        for( auto& e : struct_list )
        {
            auto res = apply_entry( prop_provider, e, report_func );
            if( res == apply_entry_result::failure_because_locked ) {
                retry_list.push_back( e );
            }
            else if( res == apply_entry_result::success ) {
                at_least_one_success = true;
            }
        }

        struct_list = std::move( retry_list );
    } while( at_least_one_success && !struct_list.empty() );

    if( !struct_list.empty() )
    {
        GError* err = nullptr;
        tcamprop1_gobj::set_gerror( &err, TCAM_ERROR_PROPERTY_NOT_WRITEABLE );

        for( auto&& entry : struct_list ) {
            report_func( *err, entry.name, entry.val.get() );
        }
        g_error_free( err );
    }
}

void tcamprop1_gobj::serialize_properties( TcamPropertyProvider* prop_provider, GstStructure& data_struct )
{
    auto prop_list = tcam_property_provider_get_tcam_property_names( prop_provider, nullptr );
    if( !prop_list )
    {
        return;
    }

    auto prop_names = gvalue::convert_GSList_to_string_vector_consume( prop_list );
    for( auto&& entry_name : prop_names )
    {
        auto ptr = gobject_helper::make_ptr( tcam_property_provider_get_tcam_property( prop_provider, entry_name.c_str(), nullptr ) );
        if( !ptr )
        {
            continue;
        }

        if (tcam_property_base_get_access(ptr.get()) == TCAM_PROPERTY_ACCESS_WO)
            continue;
        if (!tcam_property_base_is_available(ptr.get(), nullptr))
            continue;

        switch( tcam_property_base_get_property_type( ptr.get() ) )
        {
        case TCAM_PROPERTY_TYPE_BOOLEAN:
        {
            GError* err = nullptr;
            bool tmp = tcam_property_boolean_get_value( TCAM_PROPERTY_BOOLEAN( ptr.get() ), &err );
            if( !err ) {
                auto val = gvalue::gvalue_wrapper::make_value( tmp );
                gst_structure_set_value( &data_struct, entry_name.c_str(), val.get() );
            }
            else {
                g_error_free( err );
            }
            break;
        }
        case TCAM_PROPERTY_TYPE_INTEGER:
        {
            GError* err = nullptr;
            auto tmp = tcam_property_integer_get_value( TCAM_PROPERTY_INTEGER( ptr.get() ), &err );
            if( !err ) {
                auto val = gvalue::gvalue_wrapper::make_value( tmp );
                gst_structure_set_value( &data_struct, entry_name.c_str(), val.get() );
            }
            else {
                g_error_free( err );
            }
            break;
        }
        case TCAM_PROPERTY_TYPE_FLOAT:
        {
            GError* err = nullptr;
            auto tmp = tcam_property_float_get_value( TCAM_PROPERTY_FLOAT( ptr.get() ), &err );
            if( !err ) {
                auto val = gvalue::gvalue_wrapper::make_value( tmp );
                gst_structure_set_value( &data_struct, entry_name.c_str(), val.get() );
            }
            else {
                g_error_free( err );
            }
            break;
        }
        case TCAM_PROPERTY_TYPE_ENUMERATION:
        {
            GError* err = nullptr;
            const char* tmp = tcam_property_enumeration_get_value( TCAM_PROPERTY_ENUMERATION( ptr.get() ), &err );
            if( !err ) {
                auto val = gvalue::gvalue_wrapper::make_value( tmp );
                gst_structure_set_value( &data_struct, entry_name.c_str(), val.get() );
            }
            else {
                g_error_free( err );
            }
            break;
        }
        case TCAM_PROPERTY_TYPE_COMMAND:        // we can skip this, nothing to serialize here
            break;
        case TCAM_PROPERTY_TYPE_STRING:
        {
            GError* err = nullptr;
            char* tmp = tcam_property_string_get_value(TCAM_PROPERTY_STRING(ptr.get()), &err);
            if (!err)
            {
                auto val = gvalue::gvalue_wrapper::make_value(tmp);
                gst_structure_set_value(&data_struct, entry_name.c_str(), val.get());
                g_free(tmp);
            }
            else
            {
                g_error_free(err);
            }
            break;
        }
        };
    }
}
