#ifndef SRC_COMMON_ORDERS_STRATEGY_ORDER_H_
#define SRC_COMMON_ORDERS_STRATEGY_ORDER_H_

// How a strategy communicates its ideal orders
// to the netter

#include "common/wanli_util.h"

struct StrategyOrder {
  int size;
  double price;
  int user_data;
  int netter_group;

  StrategyOrder(int size, double price, int user_data, int netter_group)
      : size(size),
        price(price),
        user_data(user_data),
        netter_group(netter_group) {
  }
};

inline bool StrategyOrderCompareAscending(const StrategyOrder & a,
                                          const StrategyOrder & b) {
  return a.price < b.price;
}

inline bool StrategyOrderCompareDescending(const StrategyOrder & a,
                                           const StrategyOrder & b) {
  return a.price > b.price;
}


#endif  // SRC_COMMON_ORDERS_STRATEGY_ORDER_H_
