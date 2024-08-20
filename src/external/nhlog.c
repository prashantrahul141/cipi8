#include "nhlog.h"

const char *level_strings[] = {"TRACE", "DEBUG", "INFO",
                               "WARN",  "ERROR", "FATAL"};

const char *level_colors[] = {"\x1b[94m", "\x1b[36m", "\x1b[32m",
                              "\x1b[33m", "\x1b[31m", "\x1b[35m"};

#ifdef __cplusplus
#include <cstdio>
#include <iostream>
#else
#include <stdio.h>
#include <time.h>
#endif

static void nhlog_stdout(LogEvent *event) {
  char time_buffer[16];
  time_buffer[strftime(time_buffer, sizeof(time_buffer), "%H:%M:%S",
                       event->time)] = '\0';

#ifdef __cplusplus
  std::printf("%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ", time_buffer,
              level_colors[event->level], level_strings[event->level],
              event->file, event->line);

  std::vsprintf((char *)event->udata, event->fmt, event->ap);
  std::printf((const char *)event->udata, "\n\x1b[0m");
  std::printf("\n");
#else
  fprintf(event->udata, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m %s",
          time_buffer, level_colors[event->level], level_strings[event->level],
          event->file, event->line, level_colors[event->level]);
  vfprintf(event->udata, event->fmt, event->ap);
  fprintf(event->udata, "\n\x1b[0m");
  fflush(event->udata);
#endif
}

static struct {
  int level;
} LoggerState = {.level = 0};

void nhlog_set_level(int level) { LoggerState.level = level; }

void nhlog_log(int level, const char *file, int line, const char *fmt, ...) {
  time_t t = time(NULL);

  LogEvent event = {
      .fmt = fmt,
      .file = file,
      .time = localtime(&t),
      .udata = stderr,
      .line = line,
      .level = level,
  };

  if (level >= LoggerState.level) {
    va_start(event.ap, fmt);
    nhlog_stdout(&event);
    va_end(event.ap);
  }
}
