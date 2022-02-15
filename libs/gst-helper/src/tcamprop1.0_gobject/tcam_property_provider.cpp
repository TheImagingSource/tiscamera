
#include "../../include/tcamprop1.0_gobject/tcam_property_provider.h"
#include "../../include/tcamprop1.0_gobject/tcam_gerror.h"

#include "tcam_propnode_impl.h"
#include <gst-helper/gvalue_helper.h>

#include <unordered_map>
#include <tcamprop1.0_base/tcamprop_property_interface.h>

static bool is_err( GError** err ) {
    return err && *err;
}

static TcamPropertyBase* tcamprop_impl_create_node( tcamprop1::property_list_interface& prop_list_itf, const tcamprop1_gobj::impl::guard_state_handle& guard_handle, std::string_view name, GError** err )
{
    auto prop_itf_ptr = prop_list_itf.find_property( name );
    if( prop_itf_ptr == nullptr ) {
        tcamprop1_gobj::set_gerror( err, tcamprop1::status::property_is_not_implemented );
        return nullptr;
    }

    auto prop_state_opt = prop_itf_ptr->get_property_state();
    if( prop_state_opt.has_error() )
    {
        tcamprop1_gobj::set_gerror( err, prop_state_opt.error() );
        return nullptr;
    }
    if( !prop_state_opt.value().is_implemented )
    {
        tcamprop1_gobj::set_gerror( err, tcamprop1::status::property_is_not_implemented );
        return nullptr;
    }

    switch( prop_itf_ptr->get_property_type() )
    {
    case tcamprop1::prop_type::Boolean:
        return tcamprop1_gobj::impl::create_boolean( static_cast<tcamprop1::property_interface_boolean*>(prop_itf_ptr), guard_handle );
    case tcamprop1::prop_type::Integer:
        return tcamprop1_gobj::impl::create_integer( static_cast<tcamprop1::property_interface_integer*>(prop_itf_ptr), guard_handle );
    case tcamprop1::prop_type::Float:
        return tcamprop1_gobj::impl::create_float( static_cast<tcamprop1::property_interface_float*>(prop_itf_ptr), guard_handle );
    case tcamprop1::prop_type::Enumeration:
        return tcamprop1_gobj::impl::create_enumeration( static_cast<tcamprop1::property_interface_enumeration*>(prop_itf_ptr), guard_handle );
    case tcamprop1::prop_type::Command:
        return tcamprop1_gobj::impl::create_command( static_cast<tcamprop1::property_interface_command*>(prop_itf_ptr), guard_handle );
    case tcamprop1::prop_type::String:
        return tcamprop1_gobj::impl::create_string( static_cast<tcamprop1::property_interface_string*>(prop_itf_ptr), guard_handle );
    }

    tcamprop1_gobj::set_gerror( err, tcamprop1::status::property_is_not_implemented );
    g_warn_if_reached();

    return nullptr;
}

static GSList* tcamprop_impl_fetch_names( tcamprop1::property_list_interface& prop_list_itf )
{
    GSList* names = nullptr;
    for( const auto& prop_name : prop_list_itf.get_property_list() )
    {
        auto prop_itf = prop_list_itf.find_property( prop_name );
        if( prop_itf == nullptr )
        {
            assert( prop_itf != nullptr );
            continue;
        }

        auto prop_state_opt = prop_itf->get_property_state();
        if( !prop_state_opt.has_value() )
        {
            continue;
        }
        auto state = prop_state_opt.value();
        if( state.is_name_hidden )
        {
            continue;
        }
        if( !state.is_implemented )
        {
            continue;
        }
        names = g_slist_append( names, gvalue::g_strdup_string( prop_name ) );
    }
    return names;
}

namespace tcamprop1_gobj::impl
{
    class tcam_property_provider_impl_data
    {
    public:
        tcam_property_provider_impl_data( tcamprop1::property_list_interface* itf )
            : prop_list_itf_{ itf } 
        {}
        ~tcam_property_provider_impl_data()
        {
            tcamprop1_gobj::impl::guard_state_raii_exclusive lck{ guard_ };

            lck.mark_closed();

            for( auto&& e : flyweight_container_ ) {
                g_object_unref( e.second );
            }
            flyweight_container_.clear();
        }

        TcamPropertyBase* find_or_create_entry( const std::string& name, GError** err )
        {
            tcamprop1_gobj::impl::guard_state_raii lck{ guard_ };
            if( !lck ) {
                tcamprop1_gobj::set_gerror( err, tcamprop1::status::device_closed );
                return nullptr;
            }
            auto f = flyweight_container_.find( name );
            if( f != flyweight_container_.end() )
            {
                auto rval = f->second;
                g_object_ref( rval );
                return rval;
            }

            auto new_node = tcamprop_impl_create_node( *prop_list_itf_, guard_, name, err );
            if( new_node == nullptr ) {
                return nullptr;
            }

            /*auto [iter,added] =*/ flyweight_container_.emplace( name, new_node );

            g_object_ref( new_node );
            return new_node;
        }

        GSList* fetch_names( GError** err )
        {
            tcamprop1_gobj::impl::guard_state_raii lck{ guard_ };
            if( !lck ) {
                tcamprop1_gobj::set_gerror( err, tcamprop1::status::device_closed );
                return nullptr;
            }
            return tcamprop_impl_fetch_names( *prop_list_itf_ );
        }


    private:
        tcamprop1_gobj::impl::guard_state_handle            guard_ = tcamprop1_gobj::impl::create_guard_state_handle();
        tcamprop1::property_list_interface*                 prop_list_itf_ = nullptr;
        std::unordered_map<std::string, TcamPropertyBase*>  flyweight_container_;
    };
}


tcamprop1_gobj::tcam_property_provider::~tcam_property_provider()
{
    clear_list();
}


void tcamprop1_gobj::tcam_property_provider::create_list( tcamprop1::property_list_interface* itf )
{
    std::lock_guard lck0{ data_mtx_ };

    data_ = std::make_shared<tcamprop1_gobj::impl::tcam_property_provider_impl_data>( itf );
}

void tcamprop1_gobj::tcam_property_provider::clear_list()
{
    std::lock_guard lck0{ data_mtx_ };

    data_ = nullptr;
}

TcamPropertyBase* tcamprop1_gobj::tcam_property_provider::get_tcam_property( tcam_property_provider* cont, const char* name, GError** err )
{
    if( !cont ) {
        set_gerror( err, tcamprop1::status::device_closed );
        return nullptr;
    }
    return cont->fetch_item( name, err );
}

TcamPropertyBase* tcamprop1_gobj::tcam_property_provider::fetch_item( const char* name, GError** err )
{
    if( name == nullptr ) {
        set_gerror( err, tcamprop1::status::parameter_null );
        return nullptr;
    }

    std::shared_lock lck0{ data_mtx_ };
    if( data_ == nullptr ) {
        set_gerror( err, tcamprop1::status::device_not_opened );
        return nullptr;
    }

    auto ptr = data_->find_or_create_entry( std::string{ name }, err );
    if( is_err( err ) || ptr == nullptr ) {
        return nullptr;
    }
    return ptr;
}


auto    tcamprop1_gobj::tcam_property_provider::get_tcam_property_names( tcam_property_provider* cont, GError** err ) -> GSList*
{
    if( !cont ) {
        set_gerror( err, tcamprop1::status::device_closed );
        return nullptr;
    }

    std::shared_lock lck0{ cont->data_mtx_ };
    if( cont->data_ == nullptr ) {
        set_gerror( err, tcamprop1::status::device_not_opened );
        return nullptr;
    }
    return cont->data_->fetch_names( err );
}

auto tcamprop1_gobj::tcam_property_provider::get_boolean( tcam_property_provider* cont, const char* name, GError** err ) -> gboolean
{
    auto ptr_base = get_tcam_property( cont, name, err );
    if( is_err( err ) || ptr_base == nullptr ) {
        return FALSE;
    }
    if( !TCAM_IS_PROPERTY_BOOLEAN( ptr_base ) ) {
        set_gerror( err, TCAM_ERROR_PROPERTY_TYPE_INCOMPATIBLE );
        return FALSE;
    }

    auto rval = tcam_property_boolean_get_value( TCAM_PROPERTY_BOOLEAN( ptr_base ), err );

    g_object_unref( ptr_base );

    return rval;
}

auto tcamprop1_gobj::tcam_property_provider::get_integer( tcam_property_provider* cont, const char* name, GError** err ) -> int64_t
{
    auto ptr_base = get_tcam_property( cont, name, err );
    if( is_err( err ) || ptr_base == nullptr ) {
        return 0;
    }
    if( !TCAM_IS_PROPERTY_INTEGER( ptr_base ) ) {
        set_gerror( err, TCAM_ERROR_PROPERTY_TYPE_INCOMPATIBLE );
        return 0;
    }

    auto rval = tcam_property_integer_get_value( TCAM_PROPERTY_INTEGER( ptr_base ), err );

    g_object_unref( ptr_base );

    return rval;
}

auto tcamprop1_gobj::tcam_property_provider::get_float( tcam_property_provider* cont, const char* name, GError** err ) -> gdouble
{
    auto ptr_base = get_tcam_property( cont, name, err );
    if( is_err( err ) || ptr_base == nullptr ) {
        return 0;
    }
    if( !TCAM_IS_PROPERTY_FLOAT( ptr_base ) ) {
        set_gerror( err, TCAM_ERROR_PROPERTY_TYPE_INCOMPATIBLE );
        return 0;
    }

    auto rval = tcam_property_float_get_value( TCAM_PROPERTY_FLOAT( ptr_base ), err );

    g_object_unref( ptr_base );
    return rval;
}

auto tcamprop1_gobj::tcam_property_provider::get_enumeration( tcam_property_provider* cont, const char* name, GError** err ) -> const gchar*
{
    auto ptr_base = get_tcam_property( cont, name, err );
    if( is_err( err ) || ptr_base == nullptr ) {
        return nullptr;
    }
    if( !TCAM_IS_PROPERTY_ENUMERATION( ptr_base ) ) {
        set_gerror( err, TCAM_ERROR_PROPERTY_TYPE_INCOMPATIBLE );
        return nullptr;
    }

    auto rval = tcam_property_enumeration_get_value( TCAM_PROPERTY_ENUMERATION( ptr_base ), err );

    g_object_unref( ptr_base );
    return rval;
}

void tcamprop1_gobj::tcam_property_provider::set_boolean( tcam_property_provider* cont, const char* name, gboolean new_val, GError** err )
{
    auto ptr_base = get_tcam_property( cont, name, err );
    if( is_err( err ) || ptr_base == nullptr ) {
        return;
    }
    if( !TCAM_IS_PROPERTY_BOOLEAN( ptr_base ) ) {
        set_gerror( err, TCAM_ERROR_PROPERTY_TYPE_INCOMPATIBLE );
        return;
    }

    tcam_property_boolean_set_value( TCAM_PROPERTY_BOOLEAN( ptr_base ), new_val, err );

    g_object_unref( ptr_base );
}

void tcamprop1_gobj::tcam_property_provider::set_integer( tcam_property_provider* cont, const char* name, gint64 new_val, GError** err )
{
    auto ptr_base = get_tcam_property( cont, name, err );
    if( is_err( err ) || ptr_base == nullptr ) {
        return;
    }
    if( !TCAM_IS_PROPERTY_INTEGER( ptr_base ) ) {
        set_gerror( err, TCAM_ERROR_PROPERTY_TYPE_INCOMPATIBLE );
        return;
    }

    tcam_property_integer_set_value( TCAM_PROPERTY_INTEGER( ptr_base ), new_val, err );

    g_object_unref( ptr_base );
}

void tcamprop1_gobj::tcam_property_provider::set_float( tcam_property_provider* cont, const char* name, gdouble new_val, GError** err )
{
    auto ptr_base = get_tcam_property( cont, name, err );
    if( is_err( err ) || ptr_base == nullptr ) {
        return;
    }
    if( !TCAM_IS_PROPERTY_FLOAT( ptr_base ) ) {
        set_gerror( err, TCAM_ERROR_PROPERTY_TYPE_INCOMPATIBLE );
        return;
    }

    tcam_property_float_set_value( TCAM_PROPERTY_FLOAT( ptr_base ), new_val, err );

    g_object_unref( ptr_base );
}

void tcamprop1_gobj::tcam_property_provider::set_enumeration( tcam_property_provider* cont, const char* name, const gchar* new_val, GError** err )
{
    auto ptr_base = get_tcam_property( cont, name, err );
    if( is_err( err ) || ptr_base == nullptr ) {
        return;
    }
    if( !TCAM_IS_PROPERTY_ENUMERATION( ptr_base ) ) {
        set_gerror( err, TCAM_ERROR_PROPERTY_TYPE_INCOMPATIBLE );
        return;
    }

    tcam_property_enumeration_set_value( TCAM_PROPERTY_ENUMERATION( ptr_base ), new_val, err );

    g_object_unref( ptr_base );
}

void tcamprop1_gobj::tcam_property_provider::set_command( tcam_property_provider* cont, const char* name, GError** err )
{
    auto ptr_base = get_tcam_property( cont, name, err );
    if( is_err( err ) || ptr_base == nullptr ) {
        return;
    }
    if( !TCAM_IS_PROPERTY_COMMAND( ptr_base ) ) {
        set_gerror( err, TCAM_ERROR_PROPERTY_TYPE_INCOMPATIBLE );
        return;
    }

    tcam_property_command_set_command( TCAM_PROPERTY_COMMAND( ptr_base ), err );

    g_object_unref( ptr_base );
}

