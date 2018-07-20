#ifndef SRC_COMMON_DAILY_MARKET_SNAPSHOT_H_
#define SRC_COMMON_DAILY_MARKET_SNAPSHOT_H_

#include <stdio.h>
#include <sys/time.h>

#include "common/limits.h"

struct DailyMarketSnapshot {
  char ticker[MAX_TICKER_LENGTH];
  double pre_sett_price;
  double upper_limit;
  double lower_limit;

  DailyMarketSnapshot()
      : pre_sett_price(0.0),
        upper_limit(0.0),
        lower_limit(0.0) {
    ticker[0] = 0;
  }
};

#endif  // SRC_COMMON_DAILY_MARKET_SNAPSHOT_H_
