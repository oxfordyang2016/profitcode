#ifndef SRC_COMMON_LOGGING_PARAMETER_LOGGER_H_
#define SRC_COMMON_LOGGING_PARAMETER_LOGGER_H_

#include "common/logging/topic_logger.h"

class ParameterLogger {
 public:
  explicit ParameterLogger(FILE* stream);

  void Log(const timeval & current_time,
           int strategy_id,
           const char* parameter_name,
           const char* value);
  void Log(const timeval & current_time,
           int strategy_id,
           const char* parameter_name,
           int value);
  void Log(const timeval & current_time,
           int strategy_id,
           const char* parameter_name,
           double value);
  void Log(const timeval & current_time,
           int strategy_id,
           const char* parameter_name,
           bool value);

 private:
  TopicLogger logger_;

  DISALLOW_COPY_AND_ASSIGN(ParameterLogger);
};

#endif  // SRC_COMMON_LOGGING_PARAMETER_LOGGER_H_
