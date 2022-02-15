
#include "tcam_propnode_impl.h"
#include <gst-helper/gvalue_helper.h>

using namespace tcamprop1_gobj::impl;

namespace
{

    struct TcamPropHelperStringClass { GObjectClass parent_class; };

struct TcamPropHelperString
{
    GObject parent_instance;

    PropNodeImpl<tcamprop1::property_interface_string>  data_;

    static constexpr auto tcam_property_type = TCAM_PROPERTY_TYPE_STRING;

    static auto fetch_PropImpl( gpointer itf ) -> auto& { return cast_to_instance( itf )->data_; }

    static auto make_guard( TcamPropertyString* itf, GError** err ) -> auto { return fetch_PropImpl( itf ).make_guard( err ); }

    static TcamPropHelperString* cast_to_instance( gpointer );

    static auto set_value( TcamPropertyString* self, const char* value, GError** err ) -> void {
        auto self_ref = make_guard( self, err );
        if( !self_ref ) {
            return;
        }
        std::string_view val = value != nullptr ? std::string_view { value } : std::string_view {};
        fill_GError( self_ref->set_property_value( val ), err );
    }
    static auto get_value( TcamPropertyString* self, GError** err ) -> char* {
        auto self_ref = make_guard( self, err );
        if( !self_ref ) {
            return nullptr;
        }
        if( auto val_opt = self_ref->get_property_value(); val_opt.has_error() ) {
            fill_GError( val_opt.error(), err );
            return nullptr;
        } else {
            return gvalue::g_strdup_string( val_opt.value() );
        }
    }
};

struct TcamPropHelperStringClass_helper
{
    using class_type = TcamPropHelperStringClass;
    using instance_type = TcamPropHelperString;
    static const TcamPropertyType tcam_prop_type = TCAM_PROPERTY_TYPE_STRING;

    inline static gpointer parent_klass = nullptr;

    static std::string  get_type_class_name() { return make_module_unique_name( "TcamPropHelperString", &parent_klass ); }

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

    static GType            fetch_type() { return generate_and_fetch_type<TcamPropHelperStringClass_helper>(); }
    static instance_type* cast_to_instance( gpointer ptr ) { return G_TYPE_CHECK_INSTANCE_CAST( ptr, fetch_type(), instance_type ); }

    static void     register_interfaces( GType g_define_type_id )
    {
        G_IMPLEMENT_INTERFACE( TCAM_TYPE_PROPERTY_BASE, init_TcamPropBase );
        G_IMPLEMENT_INTERFACE( TCAM_TYPE_PROPERTY_STRING, init_TCamPropItf );
    }

    static void init_TcamPropBase( TcamPropertyBaseInterface* itf )
    {
        PropNodeImplBase::init_TcamPropertyBaseInterface<instance_type, tcam_prop_type>( itf );
    }

    static void init_TCamPropItf( TcamPropertyStringInterface* itf )
    {
        itf->set_value = instance_type::set_value;
        itf->get_value = instance_type::get_value;
    }
};

TcamPropHelperString* TcamPropHelperString::cast_to_instance( gpointer ptr ) {
    return TcamPropHelperStringClass_helper::cast_to_instance( ptr );
}

}

TcamPropertyBase* tcamprop1_gobj::impl::create_string( tcamprop1::property_interface_string* prop_itf_ptr, const guard_state_handle& handle )
{
    assert( prop_itf_ptr != nullptr );

    auto p_obj = g_object_new( TcamPropHelperStringClass_helper::fetch_type(), nullptr );
    if( p_obj == nullptr ) {
        g_warning( "Failed to allocate TCAMPROPHELPER_TYPE_STRING" );
        return nullptr;
    }

    TcamPropHelperStringClass_helper::cast_to_instance( p_obj )->data_.init( prop_itf_ptr, handle );
   
    return TCAM_PROPERTY_BASE(p_obj);
}
