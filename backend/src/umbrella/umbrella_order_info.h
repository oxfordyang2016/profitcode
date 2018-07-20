#ifndef SRC_UMBRELLA_UMBRELLA_ORDER_INFO_H_
#define SRC_UMBRELLA_UMBRELLA_ORDER_INFO_H_

#include "common/orders/umbrella_order.h"

struct UmbrellaOrderInfo {
  int strategy_id;
  int strategy_instrument_id;

  UmbrellaOrderInfo(int strategy_id,
                    int strategy_instrument_id)
      : strategy_id(strategy_id),
        strategy_instrument_id(strategy_instrument_id) {
  }
};

#endif  // SRC_UMBRELLA_UMBRELLA_ORDER_INFO_H_
