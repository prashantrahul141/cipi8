#ifndef _NHLOG_H_
#define _NHLOG_H_

#include <stdarg.h>

typedef struct {
  va_list ap;
  const char *fmt;
  const char *file;
  struct tm *time;
  void *udata;
  int line;
  int level;
} LogEvent;

enum LogLevel {
  NHLOG_TRACE,
  NHLOG_DEBUG,
  NHLOG_INFO,
  NHLOG_WARN,
  NHLOG_ERROR,
  NHLOG_FATAL
};

void nhlog_set_level(int level);

void nhlog_log(int level, const char *file, int line, const char *fmt, ...);

#define nhlog_trace(...) nhlog_log(NHLOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define nhlog_debug(...) nhlog_log(NHLOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define nhlog_info(...) nhlog_log(NHLOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define nhlog_warn(...) nhlog_log(NHLOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define nhlog_error(...) nhlog_log(NHLOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define nhlog_fatal(...) nhlog_log(NHLOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

#endif