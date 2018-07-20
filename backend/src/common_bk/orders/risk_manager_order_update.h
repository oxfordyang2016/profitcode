#ifndef SRC_COMMON_ORDERS_RISK_MANAGER_ORDER_UPDATE_H_
#define SRC_COMMON_ORDERS_RISK_MANAGER_ORDER_UPDATE_H_

#include <string.h>
#include <sys/time.h>

#include "common/limits.h"
#include "common/wanli_util.h"
#include "common/orders/order_side.h"
#include "common/orders/order_status.h"
#include "common/orders/order_update_reason.h"

struct RiskManagerOrderUpdate {
  OrderUpdateReason::Enum reason;
  OrderStatus::Enum status;
  OrderSide::Enum side;

  int total_size_filled;  // total fills (including this update)
  int size_left;
  double price;

  int size_filled;  // any new fills
  double fill_price;  // price of the latest fill

  char cl_order_id[MAX_CL_ORDER_ID_LENGTH];
  char order_id[24];

  timeval time;

  RiskManagerOrderUpdate()
    : reason(),
      status(),
      side(),
      total_size_filled(),
      size_left(),
      price(),
      size_filled(),
      fill_price(),
      cl_order_id(),
      order_id(),
      time() {
  }

  RiskManagerOrderUpdate(OrderUpdateReason::Enum reason,
                         OrderStatus::Enum status,
                         OrderSide::Enum side,
                         int total_size_filled,  // total fills (including this update)
                         int size_left,
                         double price,
                         int size_filled,  // any new fills
                         double fill_price,  // price of the latest fill
                         const char* input_cl_order_id,
                         const char* input_order_id)
      : reason(reason),
        status(status),
        side(side),
        total_size_filled(total_size_filled),
        size_left(size_left),
        price(price),
        size_filled(size_filled),
        fill_price(fill_price) {
    if (input_cl_order_id != NULL) {
      strncpy(cl_order_id, input_cl_order_id, sizeof(cl_order_id));
    } else {
      cl_order_id[0] = 0;
    }
    if (input_order_id != NULL) {
      strncpy(order_id, input_order_id, sizeof(order_id));
    } else {
      order_id[0] = 0;
    }

    gettimeofday(&time, NULL);
  }
};

#endif  // SRC_COMMON_ORDERS_RISK_MANAGER_ORDER_UPDATE_H_
