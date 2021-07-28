#pragma once

#include "gst_ptr.h"

#include <gst/gst.h>
#include <functional>
#include <string>

namespace gst_helper
{
auto find_upstream_element(GstElement& start_element,
                                          std::function<bool(GstElement&)> pred) -> gst_ptr<GstElement>;

auto has_connected_element_upstream( const GstElement& element ) -> bool;

auto  get_plugin_version_from_gst_element( const GstElement& element ) ->std::string;
}
