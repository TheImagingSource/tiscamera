#pragma once

#include "gst_ptr.h"

#include <gst/gst.h>
#include <functional>
#include <string>

namespace gst_helper
{
    /**
     * Tries to find a upstream linked GstElement which satisfies the 'pred' function parameter.
     */
    auto find_upstream_element(GstElement& start_element,
                                          std::function<bool(GstElement&)> pred) -> gst_ptr<GstElement>;
    /**
     * Returns true if the GstElement has a pad named 'sink' is connected to a pad.
     */
    auto has_connected_element_upstream( const GstElement& element ) -> bool;

    /**
     * Looks-up the plugin which contains the GstElement and returns its 'version' field contents.
     * @return The result of gst_plugin_get_version or an empty string on error
     */
    auto  get_plugin_version_from_gst_element( const GstElement& element ) ->std::string;
}
