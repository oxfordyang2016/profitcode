#ifndef EXTERNEL_SIMTRADE_MARKETDATA_H_
#define EXTERNEL_SIMTRADE_MARKETDATA_H_

#define MARKET_DEPTH 5

struct MarketData {
  double bid_price[MARKET_DEPTH];
};

#endif  // SRC_COMMON_MARKET_SNAPSHOT_H_
