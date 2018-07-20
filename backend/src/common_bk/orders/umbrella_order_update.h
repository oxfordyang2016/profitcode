#ifndef SRC_COMMON_ORDERS_UMBRELLA_ORDER_UPDATE_H_
#define SRC_COMMON_ORDERS_UMBRELLA_ORDER_UPDATE_H_

#include <assert.h>
#include <sys/time.h>

#include "common/wanli_util.h"
#include "common/limits.h"
#include "common/orders/order_status.h"
#include "common/orders/order_side.h"
#include "common/orders/order_update_reason.h"

struct UmbrellaOrderUpdate {
  char umbrella_identity[MAX_IDENTITY_LENGTH];
  char ticker[MAX_TICKER_LENGTH];
  OrderUpdateReason::Enum reason;
  OrderStatus::Enum status;

  OrderSide::Enum side;
  int size_filled;  // any new fills
  double fill_price;  // price of the latest fill

  int total_size_filled;  // total fills (including this update)
  int size_left;
  double price;

  int umbrella_order_id;
  timeval time;

  //////////////////////////

  UmbrellaOrderUpdate();

  UmbrellaOrderUpdate(const char* umbrella_identity,
                      const char* ticker,
                      OrderUpdateReason::Enum reason,
                      OrderStatus::Enum status,
                      OrderSide::Enum side,
                      int size_filled,
                      double fill_price,
                      int total_size_filled,
                      int size_left,
                      double price,
                      int umbrella_order_id,
                      const timeval & time);

  // fills in time with current time
  UmbrellaOrderUpdate(const char* umbrella_identity,
                      const char* ticker,
                      OrderUpdateReason::Enum reason,
                      OrderStatus::Enum status,
                      OrderSide::Enum side,
                      int size_filled,
                      double fill_price,
                      int total_size_filled,
                      int size_left,
                      double price,
                      int umbrella_order_id);

  bool IsDone() const;
  void Show() const;

 private:
  DISALLOW_COPY_AND_ASSIGN(UmbrellaOrderUpdate);
};

#endif  // SRC_COMMON_ORDERS_UMBRELLA_ORDER_UPDATE_H_
