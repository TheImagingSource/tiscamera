#pragma once

#include "gst_helper.h"
#include "gst_ptr.h"

#include <functional>

namespace gst_helper
{
gst_ptr<GstElement> find_upstream_element(GstElement* start_element,
                                          std::function<bool(GstElement*)> pred);
}
