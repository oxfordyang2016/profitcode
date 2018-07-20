#ifndef SRC_COMMON_SNAPSHOT_UTIL_H_
#define SRC_COMMON_SNAPSHOT_UTIL_H_

#include <cmath>
#include "common/market_snapshot.h"

MarketSnapshot MergeSnapshots(const MarketSnapshot & s,
                              const MarketSnapshot & t) {
  MarketSnapshot snapshot = s;  // copy
  for (int d = 0; d < MARKET_DATA_DEPTH; ++d) {
    if (t.bid_sizes[d] > 0) {
      // add in bid
      int idx = 0;  // level to insert at
      for (; idx < MARKET_DATA_DEPTH; ++idx) {
        if (snapshot.bid_sizes[idx] == 0 ||
            snapshot.bids[idx] <= t.bids[d] + EPS) {
          break;
        }
      }
      if (idx >= MARKET_DATA_DEPTH) {
        continue;
      }
      // shift down if this level doesn't match
      if (fabs(snapshot.bids[idx] - t.bids[d]) > EPS) {
        for (int i = MARKET_DATA_DEPTH-1; i >= idx+1; --i) {
          snapshot.bids[i] = snapshot.bids[i-1];
          snapshot.bid_sizes[i] = snapshot.bid_sizes[i-1];
        }
        snapshot.bids[idx] = t.bids[d];
        snapshot.bid_sizes[idx] = t.bid_sizes[d];
      } else {
        snapshot.bid_sizes[idx] += t.bid_sizes[d];
      }
    }
    if (t.ask_sizes[d] > 0) {
      // add in ask
      int idx = 0;  // level to insert at
      for (; idx < MARKET_DATA_DEPTH; ++idx) {
        if (snapshot.ask_sizes[idx] == 0 ||
            snapshot.asks[idx] >= t.asks[d] - EPS) {
          break;
        }
      }
      if (idx >= MARKET_DATA_DEPTH) {
        continue;
      }
      // shift down if this level doesn't match
      if (fabs(snapshot.asks[idx] - t.asks[d]) > EPS) {
        for (int i = MARKET_DATA_DEPTH-1; i >= idx+1; --i) {
          snapshot.asks[i] = snapshot.asks[i-1];
          snapshot.ask_sizes[i] = snapshot.ask_sizes[i-1];
        }
        snapshot.asks[idx] = t.asks[d];
        snapshot.ask_sizes[idx] = t.ask_sizes[d];
      } else {
        snapshot.ask_sizes[idx] += t.ask_sizes[d];
      }
    }
  }
  return snapshot;
}

#endif  // SRC_COMMON_SNAPSHOT_UTIL_H_
