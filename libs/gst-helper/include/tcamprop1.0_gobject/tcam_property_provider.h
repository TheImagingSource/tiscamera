
#pragma once

#include <shared_mutex>
#include <memory>

#include <Tcam-1.0.h>
#include "tcam_gerror.h"

namespace tcamprop1 {
    class property_list_interface;
}

namespace tcamprop1_gobj
{
    namespace impl {
        class tcam_property_provider_impl_data;
    }

    /**
     * Class to help implement a TcamPropertyProvider
     * This must be a (indirect) member of gobject which implements TcamPropertyProvider interface.
     * 
     * The gobject must populate a tcamprop1::property_list_interface derived container and then pass this to tcam_property_provider::create_list.
     * Until a list is created, all methods return TCAM_ERROR_NO_DEVICE_OPEN.
     * Call clear_list to inform all GObjects handed out to mark themselves as 'lost' and to return TCAM_ERROR_DEVICE_LOST.
     * After calling clear_list, the registered tcamprop1::property_list_interface is internally cleared and can be deleted (same goes for the property_interface derived interfaces handed
     * out by the property_list_interface.
     *
     * Use init_provider_interface to populate the interface methods. Because the parameter passed in interface methods is TcamPropertyProvider you have to implement a
     * convert method.
     * E.g:
        struct gobject_private {
            ....
            tcam_property_provider  provider_impl_;
        };
        static auto tcampimipisrc_get_provider_impl_from_interface( TcamPropertyProvider* self ) -> tcamprop1_gobj::tcam_property_provider*
        {
            gobject_private* obj = get_private_data( GST_TCAMPIMIPI_SRC( self ) );
            return &obj->provider_impl_;
        }

        static void tcampimipisrc_property_provider_init( TcamPropertyProviderInterface* iface )
        {
            tcamprop1_gobj::init_provider_interface<&tcampimipisrc_get_provider_impl_from_interface>( iface );
        }
     *
     */
    class tcam_property_provider
    {
    public:
        tcam_property_provider() = default;
        ~tcam_property_provider();

        void    create_list( tcamprop1::property_list_interface* );
        void    clear_list();

        static auto get_tcam_property_names( tcam_property_provider* cont, GError** err ) -> GSList*;
        static auto get_tcam_property( tcam_property_provider* cont, const char* name, GError** err ) -> TcamPropertyBase*;

        static auto get_boolean( tcam_property_provider* cont, const char* name, GError** err )->gboolean;
        static auto get_integer( tcam_property_provider* cont, const char* name, GError** err )->gint64;
        static auto get_float( tcam_property_provider* cont, const char* name, GError** err )->gdouble;
        static auto get_enumeration( tcam_property_provider* cont, const char* name, GError** err )->const gchar*;

        static void set_boolean( tcam_property_provider* cont, const char* name, gboolean new_val, GError** err );
        static void set_integer( tcam_property_provider* cont, const char* name, gint64 new_val, GError** err );
        static void set_float( tcam_property_provider* cont, const char* name, gdouble new_val, GError** err );
        static void set_enumeration( tcam_property_provider* cont, const char* name, const gchar* new_val, GError** err );
        static void set_command( tcam_property_provider* cont, const char* name, GError** err );
    private:
        std::shared_mutex   data_mtx_;

        std::shared_ptr<impl::tcam_property_provider_impl_data>   data_;

        TcamPropertyBase* fetch_item( const char* name, GError** err );
    };

    using get_container_from_iface_func = tcamprop1_gobj::tcam_property_provider* (*)(TcamPropertyProvider* self);

    template<get_container_from_iface_func get_container_func>
    void        init_provider_interface( TcamPropertyProviderInterface* iface )
    {
        iface->get_tcam_property_names = []( TcamPropertyProvider* self, GError** err ) { return tcam_property_provider::get_tcam_property_names( get_container_func( self ), err ); };
        iface->get_tcam_property = []( TcamPropertyProvider* self, const char* name, GError** err ) { return tcam_property_provider::get_tcam_property( get_container_func( self ), name, err ); };

        iface->set_tcam_boolean = []( TcamPropertyProvider* self, const char* name, auto value, GError** err ) { return tcam_property_provider::set_boolean( get_container_func( self ), name, value, err ); };
        iface->set_tcam_integer = []( TcamPropertyProvider* self, const char* name, auto value, GError** err ) { return tcam_property_provider::set_integer( get_container_func( self ), name, value, err ); };
        iface->set_tcam_float = []( TcamPropertyProvider* self, const char* name, auto value, GError** err ) { return tcam_property_provider::set_float( get_container_func( self ), name, value, err ); };
        iface->set_tcam_enumeration = []( TcamPropertyProvider* self, const char* name, auto value, GError** err ) { return tcam_property_provider::set_enumeration( get_container_func( self ), name, value, err ); };
        iface->set_tcam_command = []( TcamPropertyProvider* self, const char* name, GError** err ) { return tcam_property_provider::set_command( get_container_func( self ), name, err ); };

        iface->get_tcam_boolean = []( TcamPropertyProvider* self, const char* name, GError** err ) { return tcam_property_provider::get_boolean( get_container_func( self ), name, err ); };
        iface->get_tcam_integer = []( TcamPropertyProvider* self, const char* name, GError** err ) { return tcam_property_provider::get_integer( get_container_func( self ), name, err ); };
        iface->get_tcam_float = []( TcamPropertyProvider* self, const char* name, GError** err ) { return tcam_property_provider::get_float( get_container_func( self ), name, err ); };
        iface->get_tcam_enumeration = []( TcamPropertyProvider* self, const char* name, GError** err ) { return tcam_property_provider::get_enumeration( get_container_func( self ), name, err ); };
    }
}