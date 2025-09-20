#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

enum log_level
{
    LOG_TRACE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL,
};

#define STELLAR_LOG_TRACE(logger, module, format, ...)                     \
    if (log_check_level((logger), LOG_TRACE))                              \
    {                                                                      \
        log_print((logger), LOG_TRACE, (module), (format), ##__VA_ARGS__); \
    }

#define STELLAR_LOG_DEBUG(logger, module, format, ...)                     \
    if (log_check_level((logger), LOG_DEBUG))                              \
    {                                                                      \
        log_print((logger), LOG_DEBUG, (module), (format), ##__VA_ARGS__); \
    }

#define STELLAR_LOG_INFO(logger, module, format, ...)                     \
    if (log_check_level((logger), LOG_INFO))                              \
    {                                                                     \
        log_print((logger), LOG_INFO, (module), (format), ##__VA_ARGS__); \
    }

#define STELLAR_LOG_WARN(logger, module, format, ...)                     \
    if (log_check_level((logger), LOG_WARN))                              \
    {                                                                     \
        log_print((logger), LOG_WARN, (module), (format), ##__VA_ARGS__); \
    }

#define STELLAR_LOG_ERROR(logger, module, format, ...)                     \
    if (log_check_level((logger), LOG_ERROR))                              \
    {                                                                      \
        log_print((logger), LOG_ERROR, (module), (format), ##__VA_ARGS__); \
    }

#define STELLAR_LOG_FATAL(logger, module, format, ...)                     \
    if (log_check_level((logger), LOG_FATAL))                              \
    {                                                                      \
        log_print((logger), LOG_FATAL, (module), (format), ##__VA_ARGS__); \
    }

struct logger;
int log_check_level(struct logger *logger, enum log_level level);
void log_print(struct logger *logger, enum log_level level, const char *module, const char *fmt, ...);

#ifdef __cplusplus
}
#endif
