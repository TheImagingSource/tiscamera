#pragma once

#pragma once

#include <functional>
#include <gst/gst.h>

#include "g_signal_helper.h"

namespace gst_helper
{
    /**
     * Simple wrapper over a GstElement* signal connection
     */
    class gst_device_connect_signal
    {
    public:
        gst_device_connect_signal() = default;

        using func_type = std::function<void( GstElement* emitter )>;

        bool	connect( GObject* instance, const char* name, func_type&& f ) noexcept
        {
            if( bool res = handle_.connect( instance, name, G_CALLBACK( fwd_func ), this ); res )
            {
                func_ = std::move( f );
                return res;
            }
            return false;
        }
        void	disconnect() noexcept
        {
            handle_.disconnect();
            func_ = nullptr;
        }
        bool    empty() const noexcept { return handle_.empty(); }
    private:
        static void fwd_func( GstElement* src, void* userptr ) {
            static_cast<gst_device_connect_signal*>(userptr)->func_( src );
        }

        gsignal_handle  handle_;
        func_type func_;
    };
}



