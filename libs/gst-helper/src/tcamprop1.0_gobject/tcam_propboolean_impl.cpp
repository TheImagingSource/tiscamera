
#include "tcam_propnode_impl.h"

using namespace tcamprop1_gobj::impl;

namespace
{

    struct TcamPropHelperBooleanClass { GObjectClass parent_class; };

struct TcamPropHelperBoolean
{
    GObject parent_instance;

    PropNodeImpl<tcamprop1::property_interface_boolean>  data_;

    static constexpr auto tcam_property_type = TCAM_PROPERTY_TYPE_BOOLEAN;

    static auto fetch_PropImpl( gpointer itf ) -> auto& { return cast_to_instance( itf )->data_; }

    static auto make_guard( TcamPropertyBoolean* itf, GError** err ) -> auto { return fetch_PropImpl( itf ).make_guard( err ); }

    static TcamPropHelperBoolean* cast_to_instance( gpointer );

    static auto set_value( TcamPropertyBoolean* self, gboolean value, GError** err ) -> void {
        auto self_ref = make_guard( self, err );
        if( !self_ref ) {
            return;
        }
        fill_GError( self_ref->set_property_value( value != FALSE ), err );
    };
    static auto get_value( TcamPropertyBoolean* self, GError** err ) -> gboolean {
        auto self_ref = make_guard( self, err );
        if( !self_ref ) {
            return FALSE;
        }
        if( auto range_opt = self_ref->get_property_value(); range_opt.has_error() ) {
            fill_GError( range_opt.error(), err );
            return FALSE;
        }
        else {
            return range_opt.value() ? TRUE : FALSE;
        }
    };
    static auto get_default( TcamPropertyBoolean* self, GError** err ) -> gboolean {
        auto self_ref = make_guard( self, err );
        if( !self_ref ) {
            return FALSE;
        }
        if( auto range_opt = self_ref->get_property_default(); range_opt.has_error() ) {
            fill_GError( range_opt.error(), err );
            return FALSE;
        }
        else {
            return range_opt.value() ? TRUE : FALSE;
        }
    };
};

struct TcamPropHelperBooleanClass_helper
{
    using class_type = TcamPropHelperBooleanClass;
    using instance_type = TcamPropHelperBoolean;
    static const TcamPropertyType tcam_prop_type = TCAM_PROPERTY_TYPE_BOOLEAN;

    inline static gpointer parent_klass = nullptr;

    static std::string  get_type_class_name() { return make_module_unique_name( "TcamPropHelperBoolean", &parent_klass ); }

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

    static GType            fetch_type() { return generate_and_fetch_type<TcamPropHelperBooleanClass_helper>(); }
    static instance_type* cast_to_instance( gpointer ptr ) { return G_TYPE_CHECK_INSTANCE_CAST( ptr, fetch_type(), instance_type ); }

    static void     register_interfaces( GType g_define_type_id )
    {
        G_IMPLEMENT_INTERFACE( TCAM_TYPE_PROPERTY_BASE, init_TcamPropBase );
        G_IMPLEMENT_INTERFACE( TCAM_TYPE_PROPERTY_BOOLEAN, init_TCamPropItf );
    }

    static void init_TcamPropBase( TcamPropertyBaseInterface* itf )
    {
        PropNodeImplBase::init_TcamPropertyBaseInterface<instance_type, tcam_prop_type>( itf );
    }

    static void init_TCamPropItf( TcamPropertyBooleanInterface* itf )
    {
        itf->set_value = instance_type::set_value;
        itf->get_value = instance_type::get_value;
        itf->get_default = instance_type::get_default;
    }
};

TcamPropHelperBoolean* TcamPropHelperBoolean::cast_to_instance( gpointer ptr ) {
    return TcamPropHelperBooleanClass_helper::cast_to_instance( ptr );
}

}

TcamPropertyBase* tcamprop1_gobj::impl::create_boolean( tcamprop1::property_interface_boolean* prop_itf_ptr, const guard_state_handle& handle )
{
    assert( prop_itf_ptr != nullptr );

    auto p_obj = g_object_new( TcamPropHelperBooleanClass_helper::fetch_type(), nullptr );
    if( p_obj == nullptr ) {
        g_warning( "Failed to allocate TCAMPROPHELPER_TYPE_BOOLEAN" );
        return nullptr;
    }

    TcamPropHelperBooleanClass_helper::cast_to_instance( p_obj )->data_.init( prop_itf_ptr, handle );
   
    return TCAM_PROPERTY_BASE(p_obj);
}
