

#ifndef _LOGGING_H_
#define _LOGGING_H_

#include <stdarg.h>             /* va_args */
#include <string>


enum TIS_LOG_TARGET
{
    NONE         = 0,
    STDIO        = 1,
    LOGFILE      = 2,
    USER_DEFINED = 3,
};

/*
  @brief Define target for logging output
*/
void tis_set_logging_target (enum TIS_LOG_TARGET target);

void tis_set_logging_file (const char* logfile);

const char* tis_get_logging_file ();


enum TIS_LOG_LEVEL
{
    TIS_LOG_OFF     = 0,
    TIS_LOG_DEBUG   = 1,
    TIS_LOG_INFO    = 2,
    TIS_LOG_WARNING = 3,
    TIS_LOG_ERROR   = 4,
};

typedef void (*logging_callback) (enum TIS_LOG_LEVEL, const char*, int, const char*, ...);


class Logger
{

public:

    static Logger& getInstance ();

    void log (const char* module,
              enum TIS_LOG_LEVEL level,
              const char* function,
              int line,
              const char* message,
              va_list);

    void set_log_level (enum TIS_LOG_LEVEL);
    enum TIS_LOG_LEVEL get_log_level () const;

    void set_target (enum TIS_LOG_TARGET);
    enum TIS_LOG_TARGET get_target () const;

    void set_log_file (const std::string& filename);
    std::string get_log_file () const;

    void set_external_callback (logging_callback);
    void delete_external_callback ();

private:

    Logger ();

    Logger (const Logger&) = delete;
    Logger& operator= (const Logger&) = delete;

    void load_default_settings ();

    void log_to_stdout (const char* message);
    void log_to_file (const char* message);

    void open_logfile ();
    void close_logfile ();

    TIS_LOG_LEVEL level;
    std::string log_file;
    TIS_LOG_TARGET target;
    logging_callback callback;
    FILE* logfile;
};

/*
  @brief Set the general log level. Everything lower will be discarded.
*/
void tis_set_logging_level (enum TIS_LOG_LEVEL level);


enum TIS_LOG_LEVEL tis_get_logging_level ();

/*
  @brief Convenience function; wraps definitions of log-level, target,
  into one function
*/
void tis_logging_init(enum TIS_LOG_TARGET target, enum TIS_LOG_LEVEL level);


/*
  @brief logging function; follows printf syntax
*/
void tis_logging (enum TIS_LOG_LEVEL level, const char* function, int line, const char* message, ...);

void tis_logging (const char* module,
                  enum TIS_LOG_LEVEL level,
                  const char* function,
                  int line,
                  const char* message,
                  ...);

/*
  Convience wrapper macro
*/
#define tis_log(level, message, ...) (tis_logging(level, __FILE__ , __LINE__, message, ##__VA_ARGS__))

#define tis__log(module, level, message, ...) (tis_logging(module, level, __FILE__ , __LINE__, message, ##__VA_ARGS__))


#endif /* _LOGGING_H_ */

