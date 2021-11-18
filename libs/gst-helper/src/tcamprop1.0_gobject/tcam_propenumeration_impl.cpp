
#include "tcam_propnode_impl.h"
#include <gst-helper/gvalue_helper.h>
#include <gst-helper/gst_gvalue_helper.h>

using namespace tcamprop1_gobj::impl;
namespace
{
    struct TcamPropHelperEnumerationClass { GObjectClass parent_class; };

struct TcamPropHelperEnumeration
{
    GObject parent_instance;

    PropNodeImpl<tcamprop1::property_interface_enumeration>  data_;

    static constexpr auto tcam_property_type = TCAM_PROPERTY_TYPE_ENUMERATION;

    void init( tcamprop1::property_interface_enumeration* itf, const guard_state_handle& handle ) noexcept
    {
        data_.init( itf, handle );
        if( auto range_opt = itf->get_property_range(); !range_opt.has_error() ) {
            data_.enum_range_cache_ = range_opt.value();
        }
    }

    static auto fetch_PropImpl( gpointer itf ) -> auto& { return cast_to_instance( itf )->data_; }
    static auto make_guard( TcamPropertyEnumeration* itf, GError** err ) -> auto { return fetch_PropImpl( itf ).make_guard( err ); }

    static TcamPropHelperEnumeration* cast_to_instance( gpointer );

    // Helper function to fetch the according std::string::c_str from the enum_range cache
    template<class T>
    static const char*    get_range_entry( T& self_ref, outcome::result<std::string_view>&& op_result,  GError** err )
    {
        if( op_result.has_error() ) {
            fill_GError( op_result.error(), err );
            return nullptr;
        }

        auto entry = op_result.value();

        {
            std::lock_guard lck{ self_ref.parent.enum_range_cache_mutex_ };
            if( !self_ref.parent.enum_range_cache_ ) {
                if( auto range_opt = self_ref->get_property_range(); range_opt.has_error() ) {
                    fill_GError( range_opt.error(), err );
                    return nullptr;
                } else {
                    self_ref.parent.enum_range_cache_ = range_opt.value();
                }
            }
        }

        int index = self_ref.parent.enum_range_cache_.value().get_index_of( entry );
        if( index < 0 ) {
            fill_GError( err, tcamprop1::status::enumeration_property_list_error );
            return nullptr;
        }
        return self_ref.parent.enum_range_cache_.value().enum_entries.at( index ).c_str();
    }

    static auto set_value( TcamPropertyEnumeration* self, const char* value, GError** err ) -> void {
        auto self_ref = TcamPropHelperEnumeration::make_guard( self, err );
        if( !self_ref ) {
            return;
        }
        fill_GError( self_ref->set_property_value( value ), err );
    }
    static auto get_value( TcamPropertyEnumeration* self, GError** err ) -> const char* {
        auto self_ref = TcamPropHelperEnumeration::make_guard( self, err );
        if( !self_ref ) {
            return nullptr;
        }

        return get_range_entry( self_ref, self_ref->get_property_value(), err );
    }
    static auto get_enum_entries( TcamPropertyEnumeration* self, GError** err ) -> GSList* {
        auto self_ref = TcamPropHelperEnumeration::make_guard( self, err );
        if( !self_ref ) {
            return nullptr;
        }

        if( !self_ref.parent.enum_range_cache_ ) {
            if( auto range_opt = self_ref->get_property_range(); range_opt.has_error() ) {
                fill_GError( range_opt.error(), err );
                return nullptr;
            } else {
                self_ref.parent.enum_range_cache_ = range_opt.value();
            }
        }
        return gst_helper::gst_string_vector_to_GSList( self_ref.parent.enum_range_cache_.value().enum_entries );
    }
    static auto get_default( TcamPropertyEnumeration* self, GError** err ) -> const char* {
        auto self_ref = TcamPropHelperEnumeration::make_guard( self, err );
        if( !self_ref ) {
            return nullptr;
        }

        return get_range_entry( self_ref, self_ref->get_property_default(), err );
    }
};

struct TcamPropHelperEnumerationClass_helper
{
    using class_type = TcamPropHelperEnumerationClass;
    using instance_type = TcamPropHelperEnumeration;
    static const TcamPropertyType tcam_prop_type = TCAM_PROPERTY_TYPE_ENUMERATION;

    inline static gpointer parent_klass = nullptr;

    static std::string  get_type_class_name() { return make_module_unique_name( "TcamPropHelperEnumeration", &parent_klass ); }

    static void     class_init( class_type* klass )
    {
        parent_klass = g_type_class_peek_parent( klass );

        GObjectClass* object_class = G_OBJECT_CLASS( klass );

        //object_class->set_property = &_nc_object_name_set_property;
        //object_class->get_property = &_nc_object_name_get_property;
        //object_class->dispose
        object_class->finalize = []( GObject* self ) {
            std::destroy_at( &cast_to_instance( self )->data_ );
            G_OBJECT_CLASS( parent_klass )->finalize( self );
        };
    }

    static void     instance_init( instance_type* self )
    {
        new(&self->data_) decltype(self->data_);
    }

    static GType            fetch_type() { return generate_and_fetch_type<TcamPropHelperEnumerationClass_helper>(); }
    static instance_type* cast_to_instance( gpointer ptr ) { return G_TYPE_CHECK_INSTANCE_CAST( ptr, fetch_type(), instance_type ); }

    static void     register_interfaces( GType g_define_type_id )
    {
        G_IMPLEMENT_INTERFACE( TCAM_TYPE_PROPERTY_BASE, init_TcamPropBase );
        G_IMPLEMENT_INTERFACE( TCAM_TYPE_PROPERTY_ENUMERATION, init_TCamPropEnumeration );
    }

    static void init_TcamPropBase( TcamPropertyBaseInterface* itf )
    {
        PropNodeImplBase::init_TcamPropertyBaseInterface<instance_type, tcam_prop_type>( itf );
    }

    static void init_TCamPropEnumeration( TcamPropertyEnumerationInterface* itf )
    {
        itf->set_value = instance_type::set_value;
        itf->get_value = instance_type::get_value;
        itf->get_enum_entries = instance_type::get_enum_entries;
        itf->get_default = instance_type::get_default;
    }
};


TcamPropHelperEnumeration* TcamPropHelperEnumeration::cast_to_instance( gpointer ptr ) {
    return TcamPropHelperEnumerationClass_helper::cast_to_instance( ptr );
}

}

TcamPropertyBase* tcamprop1_gobj::impl::create_enumeration( tcamprop1::property_interface_enumeration* prop_itf_ptr, const guard_state_handle& handle )
{
    assert( prop_itf_ptr != nullptr );

    auto p_obj = g_object_new( TcamPropHelperEnumerationClass_helper::fetch_type(), nullptr );
    if( p_obj == nullptr ) {
        g_warning( "Failed to allocate TCAM_IS_PROPENUMERATION" );
        return nullptr;
    }

    auto* ptr = TcamPropHelperEnumerationClass_helper::cast_to_instance( p_obj );
    ptr->init( prop_itf_ptr, handle );
    return TCAM_PROPERTY_BASE(ptr);
}
