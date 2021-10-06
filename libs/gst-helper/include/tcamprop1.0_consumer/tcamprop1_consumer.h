
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <outcome/result.hpp>

#include <Tcam-1.0.h>   // _TcamPropertyProvider TcamPropertyBase

#include <gst-helper/gobject_ptr.h>
#include <tcamprop1.0_base/tcamprop_property_interface.h>
#include <tcamprop1.0_base/tcamprop_errors.h>

struct _GstElement;

namespace tcamprop1_consumer
{
    TcamPropertyProvider*           get_TcamPropertyProvider( _GstElement* elem ) noexcept;
    inline TcamPropertyProvider*    get_TcamPropertyProvider( _GstElement& elem ) noexcept { return get_TcamPropertyProvider( &elem ); }

    bool    has_TcamPropertyProvider( _GstElement& elem ) noexcept;

    auto    get_property_names( TcamPropertyProvider* elem )->outcome::result<std::vector<std::string> >;
    auto    get_property_names_noerror( TcamPropertyProvider* elem )->std::vector<std::string>;
    auto    get_property_node( TcamPropertyProvider* elem, const char* name )->outcome::result<gobject_helper::gobject_ptr<TcamPropertyBase>>;

    bool    has_property_interface( TcamPropertyProvider* elem, const char* name );
    bool    has_property_interface( TcamPropertyProvider* elem, const char* name, TcamPropertyType type );
    bool    has_property_interface( TcamPropertyProvider* elem, const char* name, tcamprop1::prop_type type );

    auto    get_property_interface( TcamPropertyProvider* elem, const char* name )->outcome::result<std::unique_ptr<tcamprop1::property_interface>>;

    auto    convert_prop_type( tcamprop1::prop_type t ) -> TcamPropertyType;

    template<class TItf>
    auto    get_property_interface( TcamPropertyProvider* elem, const char* name )->outcome::result<std::unique_ptr<TItf>>
    {
        auto base_ptr_res = get_property_interface( elem, name );
        if( base_ptr_res.has_error() ) {
            return base_ptr_res.error();
        }

        auto ptr = std::move( base_ptr_res.value() );
        if( ptr->get_property_type() != TItf::itf_type ) {
            return tcamprop1::status::parameter_type_incompatible;
        }
        return std::unique_ptr<TItf>{ static_cast<TItf*>(ptr.release()) };
    }

    /**
     * Simplification of get_property_interface to only return the pointer if succeeded and dropping any error information.
     * This can ease implementations when error info is not required.
     */
    template<class TItf>
    auto get_property_interface_ptr( TcamPropertyProvider* elem, const char* name ) -> std::unique_ptr<TItf>
    {
        auto base_ptr_res = get_property_interface<TItf>( elem, name );
        if( base_ptr_res.has_error() )
        {
            return nullptr;
        }
        return std::move( base_ptr_res.value() );
    }

    inline bool    has_property_interface( TcamPropertyProvider* elem, const char* name, tcamprop1::prop_type type )
    {
        return has_property_interface( elem, name, convert_prop_type( type ) );
    }


    /**
     * Simple wrapper class to ease usage of TcamPropertyProvider tcamprop1_consumer functions.
     * Note: This is not copy or move-able
     */
    class TcamPropertyProvider_wrapper
    {
    public:
        explicit TcamPropertyProvider_wrapper( TcamPropertyProvider& provider ) noexcept : provider_( provider ) {}

        auto get_names() const noexcept -> outcome::result<std::vector<std::string>>
        {
            return tcamprop1_consumer::get_property_names( &provider_ );
        }
        auto get_property( const char* name ) -> outcome::result<std::unique_ptr<tcamprop1::property_interface>>
        {
            return tcamprop1_consumer::get_property_interface( &provider_, name );
        }
        template<class TItf>
        auto get_property( const char* name ) -> outcome::result<std::unique_ptr<TItf>>
        {
            auto base_ptr_res = get_property( name );
            if( base_ptr_res.has_error() )
            {
                return base_ptr_res.error();
            }
            auto ptr = std::move( base_ptr_res.value() );
            if( ptr->get_property_type() != TItf::itf_type )
            {
                return tcamprop1::status::parameter_type_incompatible;
            }
            return std::unique_ptr<TItf> { static_cast<TItf*>(ptr.release()) };
        }
        template<class TItf>
        auto get_property_ptr( const char* name ) -> std::unique_ptr<TItf>
        {
            auto base_ptr_res = get_property<TItf>( name );
            if( base_ptr_res.has_error() )
            {
                return nullptr;
            }
            return std::move( base_ptr_res.value() );
        }

        bool    has_property( const char* name )
        {
            return tcamprop1_consumer::has_property_interface( &provider_, name );
        }
        bool    has_property( const char* name, tcamprop1::prop_type type )
        {
            return tcamprop1_consumer::has_property_interface( &provider_, name, type );
        }
    private:
        TcamPropertyProvider& provider_;
    };
} // namespace tcamprop1_consumer
