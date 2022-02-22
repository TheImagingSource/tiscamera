
#include "tcam_propnode_impl.h"

using namespace tcamprop1_gobj::impl;

namespace
{
    struct TcamPropHelperIntegerClass { GObjectClass parent_class; };

struct TcamPropHelperInteger
{
    GObject parent_instance;

    PropNodeImpl<tcamprop1::property_interface_integer>  data_;

    void init( tcamprop1::property_interface_integer* itf, const guard_state_handle& handle ) noexcept
    {
        data_.init( itf, handle );
        data_.unit_cache_ = itf->get_unit();
    }

    static auto fetch_PropImpl( gpointer itf ) -> auto& { return cast_to_instance( itf )->data_; }
    static auto make_guard( TcamPropertyInteger* itf, GError** err ) -> auto { return fetch_PropImpl( itf ).make_guard( err ); }

    static TcamPropHelperInteger* cast_to_instance( gpointer );

    static auto set_value( TcamPropertyInteger* self, gint64 value, GError** err ) -> void {
        auto self_ref = TcamPropHelperInteger::make_guard( self, err );
        if( !self_ref ) {
            return;
        }
        fill_GError( self_ref->set_property_value( value ), err );
    }
    static auto get_value( TcamPropertyInteger* self, GError** err ) -> gint64 {
        auto self_ref = TcamPropHelperInteger::make_guard( self, err );
        if( !self_ref ) {
            return -1;
        }
        if( auto range_opt = self_ref->get_property_value(); range_opt.has_error() ) {
            fill_GError( range_opt.error(), err );
            return -1;
        } else {
            return range_opt.value();
        }
    }
    static auto get_range( TcamPropertyInteger* self, gint64* min_value, gint64* max_value, gint64* stp_value, GError** err ) -> void {
        auto self_ref = TcamPropHelperInteger::make_guard( self, err );
        if( !self_ref ) {
            return;
        }
        if( auto range_opt = self_ref->get_property_range(); range_opt.has_error() ) {
            fill_GError( range_opt.error(), err );
        } else {
            auto range = range_opt.value();
            auto assign = []( gint64* ptr, int64_t val ) { if( ptr ) *ptr = val; };
            
            assign( min_value, range.min );
            assign( max_value, range.max );
            assign( stp_value, range.stp );
        }
    }
    static auto get_default( TcamPropertyInteger* self, GError** err ) -> gint64 {
        auto self_ref = TcamPropHelperInteger::make_guard( self, err );
        if( !self_ref ) {
            return 0;
        }
        if( auto range_opt = self_ref->get_property_default(); range_opt.has_error() ) {
            fill_GError( range_opt.error(), err );
            return 0;
        } else {
            return range_opt.value();
        }
    }
    static auto get_unit( TcamPropertyInteger* self ) -> const char* {
        auto self_ref = TcamPropHelperInteger::make_guard( self, nullptr );
        if( !self_ref ) {
            return nullptr;
        }
        if( self_ref.parent.unit_cache_.empty() ) {
            return nullptr;
        }
        return self_ref.parent.unit_cache_.c_str();
    }
    static auto get_representation( TcamPropertyInteger* self ) -> TcamPropertyIntRepresentation {
        auto self_ref = TcamPropHelperInteger::make_guard( self, nullptr );
        if( !self_ref ) {
            return TCAM_PROPERTY_INTREPRESENTATION_LINEAR;
        }
        switch( self_ref->get_representation() )
        {
        case tcamprop1::IntRepresentation_t::Linear:        return TCAM_PROPERTY_INTREPRESENTATION_LINEAR;
        case tcamprop1::IntRepresentation_t::Logarithmic:   return TCAM_PROPERTY_INTREPRESENTATION_LOGARITHMIC;
        case tcamprop1::IntRepresentation_t::PureNumber:    return TCAM_PROPERTY_INTREPRESENTATION_PURENUMBER;
        case tcamprop1::IntRepresentation_t::HexNumber:     return TCAM_PROPERTY_INTREPRESENTATION_HEXNUMBER;

        case tcamprop1::IntRepresentation_t::Boolean:
        case tcamprop1::IntRepresentation_t::IPV4Address:
        case tcamprop1::IntRepresentation_t::MACAddress:
            return TCAM_PROPERTY_INTREPRESENTATION_LINEAR;
        }
        return TCAM_PROPERTY_INTREPRESENTATION_LINEAR;
    }
};


struct TcamPropHelperIntegerClass_helper
{
    using class_type = TcamPropHelperIntegerClass;
    using instance_type = TcamPropHelperInteger;
    static const TcamPropertyType tcam_prop_type = TCAM_PROPERTY_TYPE_INTEGER;

    inline static gpointer parent_klass = nullptr;

    static std::string  get_type_class_name() { return make_module_unique_name( "TcamPropHelperInteger", &parent_klass ); }

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

    static GType            fetch_type() { return generate_and_fetch_type<TcamPropHelperIntegerClass_helper>(); }
    static instance_type*   cast_to_instance( gpointer ptr ) { return G_TYPE_CHECK_INSTANCE_CAST( ptr, fetch_type(), instance_type ); }

    static void     register_interfaces( GType g_define_type_id )
    {
        G_IMPLEMENT_INTERFACE( TCAM_TYPE_PROPERTY_BASE, init_TcamPropBase );
        G_IMPLEMENT_INTERFACE( TCAM_TYPE_PROPERTY_INTEGER, init_TCamPropInteger );
    }

    static void init_TcamPropBase( TcamPropertyBaseInterface* itf )
    {
        PropNodeImplBase::init_TcamPropertyBaseInterface<instance_type, tcam_prop_type>( itf );
    }

    static void init_TCamPropInteger( TcamPropertyIntegerInterface* itf )
    {
        itf->set_value = instance_type::set_value;
        itf->get_value = instance_type::get_value;
        itf->get_range = instance_type::get_range;
        itf->get_default = instance_type::get_default;
        itf->get_unit = instance_type::get_unit;
        itf->get_representation = instance_type::get_representation;
    }
};

TcamPropHelperInteger* TcamPropHelperInteger::cast_to_instance( gpointer ptr ) {
    return TcamPropHelperIntegerClass_helper::cast_to_instance( ptr );
}

}

TcamPropertyBase* tcamprop1_gobj::impl::create_integer( tcamprop1::property_interface_integer* prop_itf_ptr, const guard_state_handle& handle )
{
    assert( prop_itf_ptr != nullptr );

    auto p_obj = g_object_new( TcamPropHelperIntegerClass_helper::fetch_type(), nullptr );
    if( p_obj == nullptr ) {
        g_warning( "Failed to allocate TCAMPROPHELPER_TYPE_INTEGER" );
        return nullptr;
    }

    TcamPropHelperIntegerClass_helper::cast_to_instance( p_obj )->init( prop_itf_ptr, handle );

    return TCAM_PROPERTY_BASE( p_obj );
}
