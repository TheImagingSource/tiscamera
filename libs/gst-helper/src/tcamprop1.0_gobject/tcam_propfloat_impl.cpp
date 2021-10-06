
#include "tcam_propnode_impl.h"

using namespace tcamprop1_gobj::impl;

namespace
{

struct TcamPropHelperFloatClass { GObjectClass parent_class; };

struct TcamPropHelperFloat
{
    GObject parent_instance;

    PropNodeImpl<tcamprop1::property_interface_float>  data_;

    static auto fetch_PropImpl( gpointer itf ) -> auto& { return cast_to_instance( itf )->data_; }

    static auto make_guard( TcamPropertyFloat* itf, GError** err ) -> auto { return fetch_PropImpl( itf ).make_guard( err ); }

    static TcamPropHelperFloat* cast_to_instance( gpointer );

    void init( tcamprop1::property_interface_float* itf, const guard_state_handle& handle ) noexcept
    {
        data_.init( itf, handle );
        data_.unit_cache_ = itf->get_unit();
    }

    static auto set_value( TcamPropertyFloat* self, gdouble value, GError** err ) -> void
    {
        auto self_ref = make_guard( self, err );
        if( !self_ref ) {
            return;
        }
        fill_GError( self_ref->set_property_value( value ), err );
    }

    static auto get_value( TcamPropertyFloat* self, GError** err ) -> gdouble
    {
        auto self_ref = make_guard( self, err );
        if( !self_ref ) {
            return -1;
        }
        if( auto range_opt = self_ref->get_property_value(); range_opt.has_error() ) {
            fill_GError( range_opt.error(), err );
            return -1;
        }
        else
        {
            return range_opt.value();
        }
    };
    static auto get_range( TcamPropertyFloat* self, gdouble* min_value, gdouble* max_value, gdouble* stp_value, GError** err ) -> void
    {
        auto self_ref = make_guard( self, err );
        if( !self_ref ) {
            return;
        }
        if( auto range_opt = self_ref->get_property_range(); range_opt.has_error() ) {
            fill_GError( range_opt.error(), err );
        }
        else
        {
            auto range = range_opt.value();
            auto assign = []( gdouble* ptr, double val ) { if( ptr ) *ptr = val; };

            assign( min_value, range.min );
            assign( max_value, range.max );
            assign( stp_value, range.stp );
        }
    }
    static auto get_default( TcamPropertyFloat* self, GError** err ) -> gdouble {
        auto self_ref = TcamPropHelperFloat::make_guard( self, err );
        if( !self_ref ) {
            return 0;
        }
        if( auto def_res = self_ref->get_property_default(); def_res.has_error() ) {
            fill_GError( def_res.error(), err );
            return 0;
        }
        else {
            return def_res.value();
        }
    }
    static auto get_unit( TcamPropertyFloat* self ) -> const char*
    {
        auto self_ref = make_guard( self, nullptr );
        if( !self_ref ) {
            return nullptr;
        }
        if( self_ref.parent.unit_cache_.empty() ) {
            return nullptr;
        }
        return self_ref.parent.unit_cache_.c_str();
    };
    static auto get_representation( TcamPropertyFloat* self ) -> TcamPropertyFloatRepresentation
    {
        auto self_ref = make_guard( self, nullptr );
        if( !self_ref ) {
            return TcamPropertyFloatRepresentation::TCAM_PROPERTY_FLOATREPRESENTATION_LINEAR;
        }
        switch( self_ref->get_representation() )
        {
        case tcamprop1::FloatRepresentation_t::Linear:      return TCAM_PROPERTY_FLOATREPRESENTATION_LINEAR;
        case tcamprop1::FloatRepresentation_t::Logarithmic: return TCAM_PROPERTY_FLOATREPRESENTATION_LOGARITHMIC;
        case tcamprop1::FloatRepresentation_t::PureNumber:  return TCAM_PROPERTY_FLOATREPRESENTATION_PURENUMBER;
        }
        return TCAM_PROPERTY_FLOATREPRESENTATION_LINEAR;
    };
};

struct TcamPropHelperFloatClass_helper
{
    using class_type = TcamPropHelperFloatClass;
    using instance_type = TcamPropHelperFloat;
    static const TcamPropertyType tcam_prop_type = TCAM_PROPERTY_TYPE_FLOAT;

    inline static gpointer parent_klass = nullptr;

    static std::string  get_type_class_name() { return make_module_unique_name( "TcamPropHelperFloat", &parent_klass ); }

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

    static GType            fetch_type() { return generate_and_fetch_type<TcamPropHelperFloatClass_helper>(); }
    static instance_type*   cast_to_instance( gpointer ptr ) { return G_TYPE_CHECK_INSTANCE_CAST( ptr, fetch_type(), instance_type ); }

    static void     register_interfaces( GType g_define_type_id )
    {
        G_IMPLEMENT_INTERFACE( TCAM_TYPE_PROPERTY_BASE, init_TcamPropBase );
        G_IMPLEMENT_INTERFACE( TCAM_TYPE_PROPERTY_FLOAT, init_TCamPropFloat );
    }

    static void init_TcamPropBase( TcamPropertyBaseInterface* itf )
    {
        PropNodeImplBase::init_TcamPropertyBaseInterface<instance_type, tcam_prop_type>( itf );
    }

    static void init_TCamPropFloat( TcamPropertyFloatInterface* itf )
    {
        itf->set_value = instance_type::set_value;
        itf->get_value = instance_type::get_value;
        itf->get_range = instance_type::get_range;
        itf->get_default = instance_type::get_default;
        itf->get_unit = instance_type::get_unit;
        itf->get_representation = instance_type::get_representation;
    }
};

TcamPropHelperFloat* TcamPropHelperFloat::cast_to_instance(gpointer ptr) { 
    return TcamPropHelperFloatClass_helper::cast_to_instance( ptr );
}

}

TcamPropertyBase* tcamprop1_gobj::impl::create_float( tcamprop1::property_interface_float* prop_itf_ptr, const guard_state_handle& handle )
{
    assert( prop_itf_ptr != nullptr );

    auto p_obj = g_object_new( TcamPropHelperFloatClass_helper::fetch_type(), nullptr );
    if( p_obj == nullptr ) {
        g_warning( "Failed to allocate TCAMPROPHELPER_TYPE_FLOAT" );
        return nullptr;
    }

    TcamPropHelperFloatClass_helper::cast_to_instance( p_obj )->init( prop_itf_ptr, handle );

    return TCAM_PROPERTY_BASE( p_obj );
}
