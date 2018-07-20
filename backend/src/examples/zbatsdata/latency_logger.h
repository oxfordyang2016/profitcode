#ifndef SRC_MARKET_HANDLERS_ZBATSDATA_LATENCY_LOGGER_H_
#define SRC_MARKET_HANDLERS_ZBATSDATA_LATENCY_LOGGER_H_

#include <string>

#include "common/logging/latency_logger.h"
#include "zshared/basic_stats.h"

namespace bats {

class LatencyLogger {
 public:
  explicit LatencyLogger(const std::string & latency_log);

  void RecordMarketDiff(int64_t diff);
  void LogMarketDiff(const timeval & current_time);

  static int64_t NormalizeDiff(int64_t diff);

 private:
  ::LatencyLogger latency_logger_;
  zshared::BasicStats<int64_t> market_latency_;
};
}

#endif  // SRC_MARKET_HANDLERS_ZBATSDATA_LATENCY_LOGGER_H_
