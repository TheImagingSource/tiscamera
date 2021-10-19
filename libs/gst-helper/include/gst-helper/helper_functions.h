
#pragma once

#include "gst_ptr.h"

#include <string>
#include <optional>

namespace gst_helper
{
    /** Fetches the type name of the passed GstElement. */
    std::string             get_type_name( GstElement& element );
    /** Generates a string from the passed in caps */
    std::string             to_string( const GstCaps& caps );
    /** Generates a string from the passed in GstStructure */
    std::string             to_string( const GstStructure& strct );
    /** Generates a string from the passed in GstState */
    constexpr const char*   to_string( GstState state ) noexcept;

    /** Gets the named GstPad from the elem */
    auto    get_static_pad( GstElement& elem, std::string name ) -> gst_ptr<GstPad>;
    /** Gets the peer pad of pad */
    auto    get_peer_pad( GstPad& pad ) noexcept -> gst_ptr<GstPad>;
    /** Fetches the caps from the passed in pad  */
    auto    query_caps( GstPad& pad ) noexcept -> gst_ptr<GstCaps>;
    /** Returns true when the passed in caps are empty or any */
    auto    caps_empty_or_any( const GstCaps& caps ) noexcept -> bool;
    /** Returns the current state of the passed in elem
     * @param wait If false then does not wait for the state, otherwise waits for a pending state change.
     * @return Returns std::null_opt on error. (When GST_STATE_CHANGE_ASYNC is returnd, the current GstState is returned).
     */
    auto    get_gststate( GstElement& elem, bool wait = false ) noexcept -> std::optional<GstState>;
    
    inline std::optional<GstState>    get_gststate( GstElement& elem, bool wait ) noexcept
    {
        GstState state = GST_STATE_NULL, pending = GST_STATE_NULL;
        auto res = gst_element_get_state( &elem, &state, &pending, static_cast<GstClockTime>(wait ? GST_CLOCK_TIME_NONE : 0) );
        switch( res )
        {
        case GST_STATE_CHANGE_FAILURE:      return {};
        case GST_STATE_CHANGE_SUCCESS:      return state;
        case GST_STATE_CHANGE_ASYNC:        return state;
        case GST_STATE_CHANGE_NO_PREROLL:   return state;
        }
        return state;
    }

    constexpr const char* to_string( GstState state ) noexcept
    {
        switch( state )
        {
        case GST_STATE_VOID_PENDING:    return "GST_STATE_VOID_PENDING";
        case GST_STATE_NULL:            return "GST_STATE_NULL";
        case GST_STATE_READY:           return "GST_STATE_READY";
        case GST_STATE_PAUSED:          return "GST_STATE_PAUSED";
        case GST_STATE_PLAYING:         return "GST_STATE_PLAYING";
        }
        return nullptr;
    }


    inline gst_ptr<GstPad> get_static_pad( GstElement& elem, std::string name )
    {
        return gst_ptr<GstPad>::wrap( gst_element_get_static_pad( &elem, name.c_str() ) );
    }
    inline gst_ptr<GstPad> get_peer_pad( GstPad& pad ) noexcept
    {
        return gst_ptr<GstPad>::wrap( ::gst_pad_get_peer( &pad ) );
    }
    inline gst_ptr<GstCaps> query_caps( GstPad& pad ) noexcept
    {
        return gst_ptr<GstCaps>::wrap( gst_pad_query_caps( &pad, NULL ) );
    }

    inline std::string to_string( const GstCaps& caps )
    {
        auto tmp = gst_caps_to_string( &caps );
        if( tmp == nullptr )
        {
            return {};
        }
        std::string rval = tmp;
        g_free( tmp );
        return rval;
    }

    inline std::string to_string( const GstStructure& strct )
    {
        auto tmp = gst_structure_to_string( &strct );
        if( tmp == nullptr )
        {
            return {};
        }
        std::string rval = tmp;
        g_free( tmp );
        return rval;
    }

    inline bool caps_empty_or_any( const GstCaps& caps ) noexcept
    {
        return gst_caps_is_any( &caps ) || gst_caps_is_empty( &caps );
    }

    inline std::string get_type_name( GstElement& element )
    {
        // this does not leak memory!!
        const char* name =
            g_type_name( gst_element_factory_get_element_type( gst_element_get_factory( &element ) ) );
        return name;
    }

} // namespace gst_helper

