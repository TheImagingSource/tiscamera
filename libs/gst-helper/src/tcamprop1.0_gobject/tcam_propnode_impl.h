#pragma once

#include <tcam-property-1.0.h>
#include <tcamprop1.0_base/tcamprop_property_interface.h>
#include <tcamprop1.0_base/tcamprop_errors.h>
#include "guard_state.h"

#include <mutex>

namespace tcamprop1_gobj::impl
{
    TcamPropertyBase* create_float( tcamprop1::property_interface_float* prop_itf_ptr, const guard_state_handle& handle );
    TcamPropertyBase* create_integer( tcamprop1::property_interface_integer* prop_itf_ptr, const guard_state_handle& handle );
    TcamPropertyBase* create_boolean( tcamprop1::property_interface_boolean* prop_itf_ptr, const guard_state_handle& handle );
    TcamPropertyBase* create_enumeration( tcamprop1::property_interface_enumeration* prop_itf_ptr, const guard_state_handle& handle );
    TcamPropertyBase* create_command( tcamprop1::property_interface_command* prop_itf_ptr, const guard_state_handle& handle );
    TcamPropertyBase* create_string(tcamprop1::property_interface_string* prop_itf_ptr, const guard_state_handle& handle);

    void    fill_GError( GError** gerr_loc, tcamprop1::status errc );

    void    fill_GError( const std::error_code& errc, GError** gerr_loc );
    void    fill_GError_device_lost( GError** gerr_loc );

    std::string         number_to_hexstr( uint64_t w );
    std::string         make_module_unique_name( std::string_view base, gpointer module_static_pointer );

    struct PropNodeImplBase
    {
        PropNodeImplBase() = default;

        PropNodeImplBase( const PropNodeImplBase& ) = delete;
        PropNodeImplBase( PropNodeImplBase&& ) = delete;

        void    init_base( tcamprop1::property_interface* itf, const guard_state_handle& handle ) noexcept
        {
            base_itf_ptr_ = itf;
            static_info_ = itf->get_property_info();

            guard_state_handle_ = handle;
        }
        void    dispose() noexcept
        {
            guard_state_handle_.reset();
            base_itf_ptr_ = nullptr;
        }

        tcamprop1::prop_static_info_str             static_info_;
        tcamprop1_gobj::impl::guard_state_handle    guard_state_handle_;

        std::string                                 unit_cache_;    // this cache is always initialized in the init method and will never change

        std::mutex                                          enum_range_cache_mutex_;   // Because range requests can fail, we have to have a mutex to guard against this here
        std::optional<tcamprop1::prop_range_enumeration>    enum_range_cache_;
    private:
        tcamprop1::property_interface*  base_itf_ptr_;
    public:

        const char* get_name() const noexcept { return static_info_.name.c_str(); }
        const char* get_category() const noexcept { return static_info_.iccategory.c_str(); }
        const char* get_display_name() const noexcept { return static_info_.display_name.c_str(); }
        const char* get_description() const noexcept { return static_info_.description.c_str(); }

        auto make_raii_guard() { return tcamprop1_gobj::impl::guard_state_raii{ guard_state_handle_ }; }

        TcamPropertyVisibility get_visibility() const noexcept
        {
            switch( static_info_.visibility )
            {
            case tcamprop1::Visibility_t::Beginner:    return TCAM_PROPERTY_VISIBILITY_BEGINNER;
            case tcamprop1::Visibility_t::Expert:      return TCAM_PROPERTY_VISIBILITY_EXPERT;
            case tcamprop1::Visibility_t::Guru:        return TCAM_PROPERTY_VISIBILITY_GURU;
            case tcamprop1::Visibility_t::Invisible:   return TCAM_PROPERTY_VISIBILITY_INVISIBLE;
            }
            return TCAM_PROPERTY_VISIBILITY_INVISIBLE;
        }
        TcamPropertyAccess get_access() const noexcept
        {
            switch (static_info_.access)
            {
            case tcamprop1::Access_t::RW:
                return TCAM_PROPERTY_ACCESS_RW;
            case tcamprop1::Access_t::RO:
                return TCAM_PROPERTY_ACCESS_RO;
            case tcamprop1::Access_t::WO:
                return TCAM_PROPERTY_ACCESS_WO;
            }
            return TCAM_PROPERTY_ACCESS_RW;
        }
        bool        get_is_available( GError** err ) noexcept
        {
            auto guard = make_raii_guard();
            if( !guard ) {
                fill_GError_device_lost( err );
                return false;
            }

            if( auto state = base_itf_ptr_->get_property_state(); state.has_error() ) {
                fill_GError( state.error(), err );
                return false;
            } else {
                return state.value().is_available;
            }
        }
        bool        get_is_locked( GError** err ) noexcept
        {
            auto guard = make_raii_guard();
            if( !guard ) {
                fill_GError_device_lost( err );
                return false;
            }

            if( auto state = base_itf_ptr_->get_property_state(); state.has_error() ) {
                fill_GError( state.error(), err );
                return false;
            } else {
                return state.value().is_locked;
            }
        }

        template<class TImplStruct,TcamPropertyType Tprop_type>
        static void init_TcamPropertyBaseInterface( TcamPropertyBaseInterface* iface )
        {
            iface->get_name = []( TcamPropertyBase* self ) -> const char* { return TImplStruct::fetch_PropImpl( self ).get_name(); };
            iface->get_display_name = []( TcamPropertyBase* self ) -> const char* { return TImplStruct::fetch_PropImpl( self ).get_display_name(); };
            iface->get_description = []( TcamPropertyBase* self ) -> const char* { return TImplStruct::fetch_PropImpl( self ).get_description(); };
            iface->get_category = []( TcamPropertyBase* self ) -> const char* { return TImplStruct::fetch_PropImpl( self ).get_category(); };

            iface->get_visibility = []( TcamPropertyBase* self ) -> TcamPropertyVisibility { return TImplStruct::fetch_PropImpl( self ).get_visibility(); };
            iface->get_access = []( TcamPropertyBase* self ) -> TcamPropertyAccess { return TImplStruct::fetch_PropImpl( self ).get_access(); };
            iface->get_property_type = []( TcamPropertyBase* ) { return Tprop_type; };

            iface->is_available = []( TcamPropertyBase* self, GError** err ) -> gboolean { return TImplStruct::fetch_PropImpl( self ).get_is_available( err ); };
            iface->is_locked = []( TcamPropertyBase* self, GError** err ) -> gboolean { return TImplStruct::fetch_PropImpl( self ).get_is_locked( err ); };
        }
    };

    template<class TItf>
    struct PropNodeImpl : PropNodeImplBase
    {
        using self_type = PropNodeImpl<TItf>;

        void    init( TItf* itf, const guard_state_handle& handle ) noexcept
        {
            prop_itf_ = itf;
            init_base( itf, handle );
        }

        TItf*               prop_itf_ = nullptr;

        struct guard
        {
            PropNodeImpl&       parent;
            tcamprop1_gobj::impl::guard_state_raii    access_guard;

            auto    fetch_interface() -> auto& { return *parent.prop_itf_; }

            auto operator->() noexcept { return parent.prop_itf_; }
            const auto operator->() const noexcept { return parent.prop_itf_; }

            explicit operator bool() const noexcept { return access_guard.owning_lock(); }
        };

        guard       make_guard( GError** err )
        {
            guard res{ *this, make_raii_guard() };
            if( !res ) {
                fill_GError_device_lost( err );
            }
            return res;
        }
    };


    template<class TDataStruct>
    GType   generate_and_fetch_type()
    {
        static gsize g_define_type_id_volatile = 0;
        if( g_once_init_enter( &g_define_type_id_volatile ) )
        {
            GType g_define_type_id =
                g_type_register_static_simple( G_TYPE_OBJECT,
                                               TDataStruct::get_type_class_name().c_str(),
                                               sizeof( typename TDataStruct::class_type ),
                                               (GClassInitFunc)(void (*)(void)) TDataStruct::class_init,
                                               sizeof( typename TDataStruct::instance_type ),
                                               (GInstanceInitFunc)(void (*)(void)) TDataStruct::instance_init,
                                               (GTypeFlags)0 );

            TDataStruct::register_interfaces( g_define_type_id );
            g_once_init_leave( &g_define_type_id_volatile, g_define_type_id );
        }
        return g_define_type_id_volatile;
    }
}

