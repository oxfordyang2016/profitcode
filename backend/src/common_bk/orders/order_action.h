#ifndef SRC_COMMON_ORDERS_ORDER_ACTION_H_
#define SRC_COMMON_ORDERS_ORDER_ACTION_H_

struct OrderAction {
  enum Enum {
    NewOrder,
    ReplaceOrder,
    CancelOrder,
    ManualCancelOrder,
    StatusRequest
  };

  static inline const char* ToString(Enum action) {
    if (action == OrderAction::NewOrder)
      return "new_order";
    if (action == OrderAction::ReplaceOrder)
      return "replace_order";
    if (action == OrderAction::CancelOrder)
      return "cancel_order";
    if (action == OrderAction::StatusRequest)
      return "status_request";
    if (action == OrderAction::ManualCancelOrder)
      return "manual_cancel_order";
    return "error_unknown_order";
  }
};

#endif  // SRC_COMMON_ORDERS_ORDER_ACTION_H_
