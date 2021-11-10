
#include "libtcam_base.h"

#include "version.h"

#include <cstdlib>
#include <spdlog/sinks/stdout_color_sinks.h>

#ifndef TCAM_LOG_ENV_NAME
#define TCAM_LOG_ENV_NAME "TCAM_LOG"
#endif

namespace
{
static constexpr const char* tcam_log_env_name = TCAM_LOG_ENV_NAME;
static constexpr auto tcam_default_loglevel_release = spdlog::level::err;
static constexpr auto tcam_default_loglevel_debug = spdlog::level::info;


auto string2loglevel(std::string_view level) noexcept -> spdlog::level::level_enum
{
    if ("OFF" == level)
    {
        return spdlog::level::off;
    }
    else if ("TRACE" == level)
    {
        return spdlog::level::trace;
    }
    else if ("DEBUG" == level)
    {
        return spdlog::level::debug;
    }
    else if ("INFO" == level)
    {
        return spdlog::level::info;
    }
    else if ("WARNING" == level)
    {
        return spdlog::level::warn;
    }
    else if ("ERROR" == level)
    {
        return spdlog::level::err;
    }
    else if ("CRITICAL" == level)
    {
        return spdlog::level::critical;
    }
    return spdlog::level::err;
}

auto fetch_log_env_setting() noexcept -> std::optional<spdlog::level::level_enum>
{
    char* log_def = getenv(tcam_log_env_name);
    if (log_def != nullptr)
    {
        return string2loglevel(log_def);
    }
    return {};
}


auto fetch_default_log_level() noexcept
{
#if defined _DEBUG
    auto lvl = tcam_default_loglevel_debug;
#else
    auto lvl = tcam_default_loglevel_release;
#endif
    return fetch_log_env_setting().value_or(lvl);
}


struct default_logger_init
{
    default_logger_init()
    {
        default_logger_ = std::make_shared<spdlog::logger>("libtcam");

        spdlog::set_level(fetch_default_log_level());
        spdlog::set_error_handler(
            [](const std::string& msg)
            { fprintf(stderr, "Error while handling logging message: %s\n", msg.c_str()); });
        spdlog::set_pattern("[%Y%m%dT%T] [%^%-7l%$] %s:%#: %v");

        spdlog::set_default_logger(default_logger_);
    }

    void add_stdout_logger_sink()
    {
        if (!has_stdout_logger)
        {
            default_logger_->sinks().push_back(
                std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        }
        has_stdout_logger = true;
    }

    bool has_stdout_logger = false;

    std::shared_ptr<spdlog::logger> default_logger_;
};

auto& get_tcam_default_logger()
{
    static default_logger_init def_logger;
    return def_logger;
}

static void print_setup_to_logger(const std::shared_ptr<spdlog::logger>& log)
{
    SPDLOG_LOGGER_CALL(log,
                       spdlog::level::info,
                       "\nThe following library versions are "
                       "used:\n\tTcam:\t{}\n\tAravis:\t{}\n\tModules:\t{}",
                       get_version(),
                       get_aravis_version(),
                       get_enabled_modules());
}

} // namespace

void libtcam::setup_default_logger(bool add_stdout_logger)
{
    auto& log = get_tcam_default_logger();
    if (add_stdout_logger)
    {
        log.add_stdout_logger_sink();
    }
}

std::optional<spdlog::level::level_enum> libtcam::get_env_log_level() noexcept
{
    return fetch_log_env_setting();
}

auto libtcam::get_spdlog_logger() -> std::shared_ptr<spdlog::logger>
{
    return spdlog::default_logger();
}

void libtcam::print_version_info_once()
{
    static bool logging_initialized_for_module = false;
    if( !logging_initialized_for_module ) {
        print_setup_to_logger( spdlog::default_logger() );
        logging_initialized_for_module = true;
    }
}
