
#pragma once

#include <memory>
#include <string>
#include <gst/gst.h>

namespace gst_helper
{
    inline void    unref_object( GstPad* ptr ) noexcept         { gst_object_unref( ptr ); }
    inline void    unref_object( GstElement* ptr ) noexcept     { gst_object_unref( ptr ); }
    inline void    unref_object( GstCaps* ptr ) noexcept        { gst_caps_unref( ptr ); }
    namespace
    {
        template<typename TObj>
        struct delete_gobject_helper
        {
            void operator()( TObj* ptr ) noexcept { unref_object( ptr ); }
        };
    }

    template<typename T>
    using gst_unique_ptr = std::unique_ptr<T, delete_gobject_helper<T>>;

    inline gst_unique_ptr<GstPad> get_static_pad( GstElement* elem, std::string name )
    {
        return gst_unique_ptr<GstPad>{ gst_element_get_static_pad( elem, name.c_str() ) };
    }
    inline gst_unique_ptr<GstPad> get_static_pad( const gst_unique_ptr<GstElement>& elem, std::string name ) 
    {
        return get_static_pad( elem.get(), name );
    }


    inline gst_unique_ptr<GstCaps>  query_caps( GstPad* pad ) noexcept
    {
        return gst_unique_ptr<GstCaps>{ gst_pad_query_caps( pad, NULL ) };
    }
    inline gst_unique_ptr<GstCaps>  query_caps( const gst_unique_ptr<GstPad>& pad ) noexcept
    {
        return query_caps( pad.get() );
    }


    inline std::string      to_string( const GstCaps* caps )
    {
        auto tmp = gst_caps_to_string( caps );
        if( tmp == nullptr ) {
            return {};
        }

        std::string rval = tmp;
        g_free( tmp );
        return rval;
    }
    inline std::string      to_string( const gst_unique_ptr<GstCaps>& caps )
    {
        return to_string( caps.get() );
    }


    inline bool             caps_empty_or_any( const gst_unique_ptr<GstCaps>& caps ) noexcept
    {
        return gst_caps_is_any( caps.get() ) || gst_caps_is_empty( caps.get() );
    }

}