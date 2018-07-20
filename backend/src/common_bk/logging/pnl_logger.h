#ifndef SRC_COMMON_LOGGING_PNL_LOGGER_H_
#define SRC_COMMON_LOGGING_PNL_LOGGER_H_

#include <sys/time.h>
#include <vector>

#include "common/logging/topic_logger.h"

class PnlLogger {
 public:
  explicit PnlLogger(FILE* stream);

  void Log(const timeval & current_time,
           int strategy_id,
           double pnl);

 private:
  TopicLogger logger_;

  DISALLOW_COPY_AND_ASSIGN(PnlLogger);
};

#endif  // SRC_COMMON_LOGGING_PNL_LOGGER_H_
