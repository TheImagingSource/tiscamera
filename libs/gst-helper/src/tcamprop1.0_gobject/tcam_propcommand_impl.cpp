
#include "tcam_propnode_impl.h"

using namespace tcamprop1_gobj::impl;

namespace
{
    struct TcamPropHelperCommandClass { GObjectClass parent_class; };

struct TcamPropHelperCommand
{
    GObject parent_instance;

    PropNodeImpl<tcamprop1::property_interface_command>  data_;

    static constexpr auto tcam_property_type = TCAM_PROPERTY_TYPE_COMMAND;

    static auto fetch_PropImpl( gpointer itf ) -> auto& { return cast_to_instance( itf )->data_; }
    static auto make_guard( TcamPropertyCommand* itf, GError** err ) -> auto { return fetch_PropImpl( itf ).make_guard( err ); }

    static TcamPropHelperCommand* cast_to_instance( gpointer );

    static auto execute_command( TcamPropertyCommand* self, GError** err ) -> void {
        auto self_ref = TcamPropHelperCommand::make_guard( self, err );
        if( !self_ref ) {
            return;
        }
        fill_GError( self_ref->execute_command(), err );
    }
};

struct TcamPropHelperCommandClass_helper
{
    using class_type = TcamPropHelperCommandClass;
    using instance_type = TcamPropHelperCommand;
    static const TcamPropertyType tcam_prop_type = TCAM_PROPERTY_TYPE_COMMAND;

    inline static gpointer parent_klass = nullptr;

    static std::string  get_type_class_name() { return make_module_unique_name( "TcamPropHelperCommand", &parent_klass ); }

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

    static GType            fetch_type() { return generate_and_fetch_type<TcamPropHelperCommandClass_helper>(); }
    static instance_type* cast_to_instance( gpointer ptr ) { return G_TYPE_CHECK_INSTANCE_CAST( ptr, fetch_type(), instance_type ); }

    static void     register_interfaces( GType g_define_type_id )
    {
        G_IMPLEMENT_INTERFACE( TCAM_TYPE_PROPERTY_BASE, init_TcamPropBase );
        G_IMPLEMENT_INTERFACE( TCAM_TYPE_PROPERTY_COMMAND, init_TCamPropCommand );
    }

    static void init_TcamPropBase( TcamPropertyBaseInterface* itf )
    {
        PropNodeImplBase::init_TcamPropertyBaseInterface<instance_type, tcam_prop_type>( itf );
    }

    static void init_TCamPropCommand( TcamPropertyCommandInterface* itf )
    {
        itf->set_command = instance_type::execute_command;
    }
};

TcamPropHelperCommand* TcamPropHelperCommand::cast_to_instance( gpointer ptr ) {
    return TcamPropHelperCommandClass_helper::cast_to_instance( ptr );
}

}

TcamPropertyBase* tcamprop1_gobj::impl::create_command( tcamprop1::property_interface_command* prop_itf_ptr, const guard_state_handle& handle )
{
    assert( prop_itf_ptr != nullptr );
    
    auto p_obj = g_object_new( TcamPropHelperCommandClass_helper::fetch_type(), nullptr );
    if( p_obj == nullptr ) {
        g_warning( "Failed to allocate TCAMPROPHELPER_TYPE_COMMAND" );
        return nullptr;
    }

    auto* ptr = TcamPropHelperCommandClass_helper::cast_to_instance( p_obj );
    ptr->data_.init( prop_itf_ptr, handle );
    return TCAM_PROPERTY_BASE( ptr );
}
