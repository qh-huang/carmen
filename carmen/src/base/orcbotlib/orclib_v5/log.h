#ifndef _LOG_H
#define _LOG_H

#include "clog.h"

#define LOG_SOURCE "LIBORC"

#define LOG_MESSAGE(level, fmt, ...) if (level < clog_log_level) \
		clog_log_message(LOG_SOURCE, level, __FILE__, __LINE__, clog_mallocf(fmt, ##__VA_ARGS__), 1);

#define LOG_FATAL(fmt, ...)    LOG_MESSAGE(0, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)    LOG_MESSAGE(1, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)     LOG_MESSAGE(2, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)     LOG_MESSAGE(3, fmt, ##__VA_ARGS__)
#define LOG_VERBOSE(fmt, ...)  LOG_MESSAGE(4, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...)    LOG_MESSAGE(5, fmt, ##__VA_ARGS__)

#endif
