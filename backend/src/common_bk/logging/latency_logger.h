#ifndef SRC_COMMON_LOGGING_LATENCY_LOGGER_H_
#define SRC_COMMON_LOGGING_LATENCY_LOGGER_H_

#include "common/logging/topic_logger.h"
#include "zshared/basic_stats.h"

class LatencyLogger {
 public:
  explicit LatencyLogger(FILE* stream);
  explicit LatencyLogger(const char* file);

  void Log(const timeval & current_time,
           const char* component,
           zshared::BasicStats<int64_t>* stats);

 private:
  TopicLogger logger_;

  DISALLOW_COPY_AND_ASSIGN(LatencyLogger);
};

#endif  // SRC_COMMON_LOGGING_LATENCY_LOGGER_H_
