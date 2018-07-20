#ifndef SRC_COMMON_ORDERS_ORDER_STATUS_H_
#define SRC_COMMON_ORDERS_ORDER_STATUS_H_

struct OrderStatus {
  enum Enum {
    New,
    PartiallyFilled,
    Filled,
    Cancelled,
    Replaced,
    Rejected,
    Expired,  // similar to cancelled (but initiated by exchange, usually at end of day)
    Suspended,
    Error
  };

  static inline bool IsDone(OrderStatus::Enum status) {
    return (status == OrderStatus::Filled ||
            status == OrderStatus::Cancelled ||
            status == OrderStatus::Rejected ||
            status == OrderStatus::Expired);
  }

  static inline const char* ToString(Enum status) {
    if (status == OrderStatus::New) {
      return "new";
    } else if (status == OrderStatus::PartiallyFilled) {
      return "pfilled";
    } else if (status == OrderStatus::Filled) {
      return "filled";
    } else if (status == OrderStatus::Cancelled) {
      return "cancelled";
    } else if (status == OrderStatus::Replaced) {
      return "replaced";
    } else if (status == OrderStatus::Rejected) {
      return "rejected";
    } else if (status == OrderStatus::Expired) {
      return "expired";
    } else if (status == OrderStatus::Suspended) {
      return "suspended";
    } else if (status == OrderStatus::Error) {
      return "error";
    }
    return "unknown";
  }
};

#endif  // SRC_COMMON_ORDERS_ORDER_STATUS_H_
