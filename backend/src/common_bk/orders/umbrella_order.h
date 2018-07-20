#ifndef SRC_COMMON_ORDERS_UMBRELLA_ORDER_H_
#define SRC_COMMON_ORDERS_UMBRELLA_ORDER_H_

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <string>

#include "common/limits.h"
#include "common/orders/order_action.h"
#include "common/orders/order_side.h"

#include "common/wanli_util.h"

// how the netter communicates its orders to the umbrella

struct UmbrellaOrder {
  char umbrella_identity[MAX_IDENTITY_LENGTH];  // must be filled in prior to use!
  char ticker[MAX_TICKER_LENGTH];
  OrderAction::Enum action;
  OrderSide::Enum side;
  int size;
  double price;

  int umbrella_order_id;
  timeval time;  // time of creation

  int user_data;

  UmbrellaOrder()
    : umbrella_identity(),
      ticker(),
      action(),
      side(),
      size(),
      price(),
      umbrella_order_id(),
      time(),
      user_data() {
  }

  UmbrellaOrder(const char* input_ticker,
                OrderAction::Enum action,
                OrderSide::Enum side,
                int size,
                double price,
                int umbrella_order_id,
                timeval time,
                int user_data) :
      action(action),
      side(side),
      size(size),
      price(price),
      umbrella_order_id(umbrella_order_id),
      time(time),
      user_data(user_data) {
    strncpy(ticker, input_ticker, sizeof(ticker));
  }

  void SetIdentity(const std::string & in_identity) {
    strncpy(umbrella_identity, in_identity.c_str(), sizeof(umbrella_identity));
  }
};

#endif  // SRC_COMMON_ORDERS_UMBRELLA_ORDER_H_
