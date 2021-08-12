#pragma once

#include <gobject/gsignal.h>

namespace gst_helper
{
    /**
     * Tests if the passed in GObject has the according signal.
     */
    inline bool has_signal( GObject* instance, const char* name ) noexcept
    {
        return g_signal_lookup( name, G_OBJECT_TYPE( instance ) ) != 0;
    }
    /**
     * Simple wrapper for a GObject signal connection.
     */
    class gsignal_handle
    {
    public:
        gsignal_handle() = default;
        gsignal_handle( gsignal_handle&& ) = delete;
        gsignal_handle( const gsignal_handle& ) = delete;
        gsignal_handle& operator=( gsignal_handle&& ) = delete;
        gsignal_handle& operator=( const gsignal_handle& ) = delete;
        
        ~gsignal_handle() {
            disconnect();
        }
        
        bool	connect( GObject* instance, const char* name, GCallback cb, gpointer userpointer ) noexcept
        {
            if( !empty() ) {
                return false;
            }
            handler_id_ = g_signal_connect_data( instance, name, cb, userpointer, nullptr, GConnectFlags( 0 ) );
            if( handler_id_ > 0 ) {
                instance_ = instance;
            }
            return handler_id_ > 0;
        }
        void	disconnect() noexcept
        {
            if( !empty() ) {
                g_signal_handler_disconnect( instance_, handler_id_ );
                handler_id_ = 0;
                instance_ = nullptr;
            }
        }
        bool empty() const noexcept { return handler_id_ == 0; }
    private:
        GObject* instance_ = nullptr;
        gulong	handler_id_ = 0;
    };

}

