#include "market_handlers/zbatsdata/latency_logger.h"

#include <string>

namespace bats {

LatencyLogger::LatencyLogger(const std::string & latency_log)
  : latency_logger_(latency_log.c_str()) {
}

void LatencyLogger::RecordMarketDiff(int64_t diff) {
  market_latency_.Event(LatencyLogger::NormalizeDiff(diff));
}

void LatencyLogger::LogMarketDiff(const timeval & current_time) {
  latency_logger_.Log(current_time, "latency", &market_latency_);
}

int64_t LatencyLogger::NormalizeDiff(int64_t diff) {
  const int64_t one_hour = 60 * 60 * 1000000ULL;
  if (diff < -one_hour / 2 || diff > one_hour / 2) {
    // Because of daylight savings, we aren't guaranteed to be in the
    // same timezone.  So just fix the difference to be within half
    // an hour no matter how far it is off.
    diff += one_hour / 2;
    diff -= ((diff / one_hour) - (diff < 0 ? 1 : 0)) * one_hour;
    diff -= one_hour / 2;
  }
  return diff;
}
}
