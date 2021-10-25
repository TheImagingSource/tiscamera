
#pragma once

#include <gst/gst.h>
#include <string>
#include <functional>

#include <Tcam-1.0.h>

namespace tcamprop1_gobj
{
    using report_error_function = std::function<void ( const GError& err, const std::string& prop_name, const GValue* prop_value )>;

    void    apply_properties( TcamPropertyProvider* prop_provider, const GstStructure& data_struct, const report_error_function& report_func );
    void    serialize_properties( TcamPropertyProvider* prop_provider, GstStructure& data_struct );
}