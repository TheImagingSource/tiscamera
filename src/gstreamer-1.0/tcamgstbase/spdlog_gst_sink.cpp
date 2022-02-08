

#include "spdlog_gst_sink.h"

#include <gst/gst.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/base_sink.h>

namespace
{
constexpr GstDebugLevel map_levels(spdlog::level::level_enum lvl) noexcept
{
    switch (lvl)
    {
        case spdlog::level::trace:
            return GstDebugLevel::GST_LEVEL_TRACE;
        case spdlog::level::debug:
            return GstDebugLevel::GST_LEVEL_DEBUG;
        case spdlog::level::info:
            return GstDebugLevel::GST_LEVEL_INFO;
        case spdlog::level::warn:
            return GstDebugLevel::GST_LEVEL_WARNING;
        case spdlog::level::err:
        case spdlog::level::critical:
            return GstDebugLevel::GST_LEVEL_ERROR;
        case spdlog::level::off:
            return GstDebugLevel::GST_LEVEL_NONE;
        case spdlog::level::n_levels:
            return GstDebugLevel::GST_LEVEL_COUNT;
    }
    return GstDebugLevel::GST_LEVEL_NONE;
}

constexpr spdlog::level::level_enum map_levels(GstDebugLevel lvl) noexcept
{
    switch (lvl)
    {
        case GstDebugLevel::GST_LEVEL_MEMDUMP:
        // case 8: // this has no id, I have no idea why ...
        case GstDebugLevel::GST_LEVEL_TRACE:
        case GstDebugLevel::GST_LEVEL_LOG:
            return spdlog::level::trace;
        case GstDebugLevel::GST_LEVEL_DEBUG:
            return spdlog::level::debug;
        case GstDebugLevel::GST_LEVEL_INFO:
            return spdlog::level::info;
        case GstDebugLevel::GST_LEVEL_FIXME:
        case GstDebugLevel::GST_LEVEL_WARNING:
            return spdlog::level::warn;
        case GstDebugLevel::GST_LEVEL_ERROR:
            return spdlog::level::err;
        case GstDebugLevel::GST_LEVEL_NONE:
            return spdlog::level::off;
        case GstDebugLevel::GST_LEVEL_COUNT:
            return spdlog::level::n_levels;
    }
    if (lvl > GstDebugLevel::GST_LEVEL_LOG)
    {
        return spdlog::level::trace;
    }
    return spdlog::level::off;
}

class gst_sink : public spdlog::sinks::base_sink<spdlog::details::null_mutex>
{
public:
    gst_sink(GstDebugCategory* cat) noexcept : cat_(cat) {}

protected:
    void sink_it_(const spdlog::details::log_msg& msg) final
    {
        if (msg.level < tcam::gst::log::level_from_category(cat_))
        {
            return;
        }

        // we skip string formatting here
        if (msg.source.filename != nullptr)
        {
            gst_debug_log(cat_,
                          map_levels(msg.level),
                          msg.source.filename,
                          msg.source.funcname,
                          msg.source.line,
                          nullptr,
                          "%.*s",
                          (int)msg.payload.size(),
                          msg.payload.data());
        }
        else
        {
            GST_CAT_LEVEL_LOG(cat_,
                              map_levels(msg.level),
                              nullptr,
                              "%.*s",
                              (int)msg.payload.size(),
                              msg.payload.data());
        }
    }
    void flush_() final {}

private:
    GstDebugCategory* cat_ = nullptr;
};
} // namespace

std::shared_ptr<spdlog::sinks::sink> tcam::gst::log::create_gst_sink(GstDebugCategory* cat)
{
    return std::make_shared<gst_sink>(cat);
}

spdlog::level::level_enum tcam::gst::log::level_from_category(GstDebugCategory* cat)
{
    return map_levels(gst_debug_category_get_threshold(cat));
}

spdlog::level::level_enum tcam::gst::log::level_from_gst_debug_min()
{
    return map_levels(_gst_debug_min);
}

int tcam::gst::log::convert_spdlog_level_to_gst(spdlog::level::level_enum lvl)
{
    return map_levels(lvl);
}
