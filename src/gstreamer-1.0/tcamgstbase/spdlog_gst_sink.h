
#pragma once

#include "../../logging.h"

typedef struct _GstDebugCategory GstDebugCategory;

namespace tcam::gst::log
{
std::shared_ptr<spdlog::sinks::sink> create_gst_sink(GstDebugCategory* cat);

spdlog::level::level_enum level_from_category(GstDebugCategory* cat);
spdlog::level::level_enum level_from_gst_debug_min();

int convert_spdlog_level_to_gst( spdlog::level::level_enum lvl );

}