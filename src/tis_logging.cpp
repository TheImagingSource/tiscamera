
#include "tis_logging.h"

#include <stdio.h>              /* printf, fopen */
#include <stdarg.h>             /* va_args */
#include <string.h>             /* memcpy */
#include <time.h>               /* time_t */

struct tis_logging_info
{
    enum TIS_LOG_LEVEL  level;
    enum TIS_LOG_TARGET target;

    char  logfile_name[256];
    FILE* logfile;

    logging_callback callback;
};

/* static struct tis_logging_info logger = { ERROR, STDIO, "/tmp/tis.log", NULL, NULL}; */
static struct tis_logging_info logger = { TIS_LOG_DEBUG, STDIO, "/tmp/tis.log", NULL, NULL};


static const char* loglevel2string (const enum TIS_LOG_LEVEL level)
{
    switch (level)
    {
        case TIS_LOG_OFF:
            return "OFF";
        case TIS_LOG_DEBUG:
            return "DEBUG";
        case TIS_LOG_INFO:
            return "INFO";
        case TIS_LOG_WARNING:
            return "WARNING";
        case TIS_LOG_ERROR:
            return "ERROR";
        default:
            return NULL;
    }
}


Logger::Logger ():
    callback(nullptr),
    logfile(nullptr)
{
    // TODO: make environment variable name configurable
    char* log_def = getenv("TIS_LOG");
    if (log_def == nullptr)
    {
        load_default_settings();
    }
}

void Logger::load_default_settings ()
{
    level = TIS_LOG_DEBUG;
    target = STDIO;
    log_file = "tmp/tis.log";
}


void Logger::log (const char* module,
                  enum TIS_LOG_LEVEL level,
                  const char* function,
                  int line,
                  const char* message,
                  va_list args)
{
    if (level < logger.level)
    {
        return;
    }

    char msg[1024];
    char buffer [2056];

    /* fill user defined message */
    vsprintf(msg, message, args);

    clock_t t;
    t = clock();
    /* write complete message */
    sprintf(buffer,
            "%-10ld <%s> %s:%d: %s\n",
            /* ctime(&timer), */
            t,
            loglevel2string(level),
            function,
            line,
            msg);

    switch (logger.target)
    {
        case STDIO:
            log_to_stdout(buffer);
            break;
        case LOGFILE:
            log_to_file(buffer);
            break;
        case USER_DEFINED:
            //logger.callback(level, file, line, message, args);
            break;
        default:
            break;
    }
}


void Logger::log_to_stdout (const char* message)
{
    fprintf(stdout, "%s", message);
    fflush(stdout);
}


void Logger::log_to_file (const char* message)
{}


void Logger::set_log_level (enum TIS_LOG_LEVEL l)
{
    level = l;
}


enum TIS_LOG_LEVEL Logger::get_log_level () const
{
    return level;
}


void Logger::set_target (enum TIS_LOG_TARGET t)
{
    target = t;
}


enum TIS_LOG_TARGET Logger::get_target () const
{
    return target;
}


void Logger::set_log_file (const std::string& filename)
{
    log_file = filename;
}


std::string Logger::get_log_file () const
{
    return log_file;
}


void Logger::set_external_callback (logging_callback c)
{
    callback = c;
}


void Logger::delete_external_callback ()
{
    callback = nullptr;
}


void Logger::open_logfile ()
{
    if (!log_file.empty())
        logfile = fopen(log_file.c_str(), "a+");
}


void Logger::close_logfile ()
{
    if (logfile != NULL)
    {
        fclose(logfile);
        logfile = NULL;
    }
}


Logger& Logger::getInstance ()
{
    static Logger instance;

    return instance;
}



void tis_set_logging_level (enum TIS_LOG_LEVEL level)
{
    Logger::getInstance().set_log_level(level);
}


enum TIS_LOG_LEVEL tis_get_logging_level ()
{
    return Logger::getInstance().get_log_level();
}


void tis_logging_init(enum TIS_LOG_TARGET target, enum TIS_LOG_LEVEL level)
{
    tis_set_logging_target(target);
    tis_set_logging_level(level);
}





void tis_set_logging_target (enum TIS_LOG_TARGET target)
{
    Logger::getInstance().set_target(target);
}


void tis_set_logging_file (const char* logfile_name)
{
    Logger::getInstance().set_log_file(logfile_name);
}


const char* tis_get_logging_file ()
{
    return Logger::getInstance().get_log_file().c_str();
}


void tis_logging (enum TIS_LOG_LEVEL level, const char* file, int line, const char* message, ...)
{
    if (Logger::getInstance().get_log_level() > level ||
        Logger::getInstance().get_log_level() == TIS_LOG_OFF)
    {
        return;
    }

    va_list args;
    va_start(args, message);

    Logger::getInstance().log("", level, file, line, message, args);

    va_end(args);
}


void tis_logging (const char* module, enum TIS_LOG_LEVEL level, const char* function, int line, const char* message, ...)
{
    if (Logger::getInstance().get_log_level() > level ||
        Logger::getInstance().get_log_level() == TIS_LOG_OFF)
    {
        return;
    }

    va_list args;
    va_start(args, message);

    Logger::getInstance().log(module, level, function, line, message, args);

    va_end(args);
}


