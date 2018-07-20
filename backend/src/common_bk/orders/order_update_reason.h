#ifndef SRC_COMMON_ORDERS_ORDER_UPDATE_REASON_H_
#define SRC_COMMON_ORDERS_ORDER_UPDATE_REASON_H_

struct OrderUpdateReason {
  enum Enum {
    New,
    PartiallyFilled,
    Filled,
    Cancelled,
    Replaced,
    Rejected,
    Expired,
    Restated,
    Error
  };

  static inline const char* ToString(Enum reason) {
    if (reason == OrderUpdateReason::New) {
      return "new";
    } else if (reason == OrderUpdateReason::PartiallyFilled) {
      return "pfilled";
    } else if (reason == OrderUpdateReason::Filled) {
      return "filled";
    } else if (reason == OrderUpdateReason::Cancelled) {
      return "cancelled";
    } else if (reason == OrderUpdateReason::Replaced) {
      return "replaced";
    } else if (reason == OrderUpdateReason::Rejected) {
      return "rejected";
    } else if (reason == OrderUpdateReason::Expired) {
      return "expired";
    } else if (reason == OrderUpdateReason::Restated) {
      return "restated";
    } else if (reason == OrderUpdateReason::Error) {
      return "error";
    }
    return "unknown";
  }
};

#endif  // SRC_COMMON_ORDERS_ORDER_UPDATE_REASON_H_
