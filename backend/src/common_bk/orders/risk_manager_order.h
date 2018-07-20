#ifndef SRC_COMMON_ORDERS_RISK_MANAGER_ORDER_H_
#define SRC_COMMON_ORDERS_RISK_MANAGER_ORDER_H_

#include <sys/time.h>

#include "common/limits.h"
#include "common/orders/order_action.h"
#include "common/orders/order_side.h"

struct RiskManagerOrder {
  char ticker[MAX_TICKER_LENGTH];
  OrderAction::Enum action;
  OrderSide::Enum side;
  int size;
  double price;
  int size_prev_filled;
  int size_prev_left;

  char cl_order_id[MAX_CL_ORDER_ID_LENGTH];
  char order_id[24];  // only needed for cancels / modifies
  timeval time;  // time created by risk_manager
  int message_id;

  // only applicable for new orders
  // used by equitiy exchanges to figure out if we might short-sell
  bool might_be_short_after_order;

  int user_data;

  // initialize with safe values
  RiskManagerOrder()
      : size(0),
        price(0),
        size_prev_filled(0),
        size_prev_left(0),
        message_id(-1),
        user_data(0) {
  }
};

#endif  // SRC_COMMON_ORDERS_RISK_MANAGER_ORDER_H_
