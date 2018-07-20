#ifndef SRC_COMMON_ORDERS_ORDER_SIDE_H_
#define SRC_COMMON_ORDERS_ORDER_SIDE_H_

struct OrderSide {
  enum Enum {
    Buy,
    Sell
  };

  static inline const char* ToString(Enum side) {
    if (side == OrderSide::Buy) {
      return "BUY";
    } else if (side == OrderSide::Sell) {
      return "SELL";
    }
    return "UNKNOWN_SIDE";
  }
};

#endif  // SRC_COMMON_ORDERS_ORDER_SIDE_H_
