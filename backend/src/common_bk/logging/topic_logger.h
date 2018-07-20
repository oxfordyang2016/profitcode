#ifndef SRC_COMMON_LOGGING_TOPIC_LOGGER_H_
#define SRC_COMMON_LOGGING_TOPIC_LOGGER_H_

#include "common/logging/base_logger.h"

class TopicLogger {
 public:
  explicit TopicLogger(FILE* stream);
  explicit TopicLogger(const char* file);

  // adds new line at end of message
  void Log(const char* topic,
           const char* format,
           ...);
  void Log(const timeval & time,
           const char* topic,
           const char* format,
           ...);
  void Log(const timeval & time,
           const char* topic,
           const char* format,
           va_list arglist);

 private:
  BaseLogger logger_;

  DISALLOW_COPY_AND_ASSIGN(TopicLogger);
};

#endif  // SRC_COMMON_LOGGING_TOPIC_LOGGER_H_
