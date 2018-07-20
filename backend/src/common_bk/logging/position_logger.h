#ifndef SRC_COMMON_LOGGING_POSITION_LOGGER_H_
#define SRC_COMMON_LOGGING_POSITION_LOGGER_H_

#include <sys/time.h>
#include <vector>

#include "common/logging/topic_logger.h"

class PositionLogger {
 public:
  explicit PositionLogger(FILE* stream);

  void Log(const timeval & current_time,
           int strategy_id,
           const std::vector<int> & positions,
           int num_tradeable);

 private:
  TopicLogger logger_;

  DISALLOW_COPY_AND_ASSIGN(PositionLogger);
};

#endif  // SRC_COMMON_LOGGING_POSITION_LOGGER_H_
