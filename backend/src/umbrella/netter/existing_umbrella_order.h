#ifndef SRC_UMBRELLA_NETTER_EXISTING_UMBRELLA_ORDER_H_
#define SRC_UMBRELLA_NETTER_EXISTING_UMBRELLA_ORDER_H_

// how the netter stores orders internally

struct ExistingUmbrellaOrderStatus {
  enum Enum {
    Acknowledged,
    PendingNew,
    PendingCancel,
    PendingReplace,
    Error
  };
};

struct ExistingUmbrellaOrder {
  int umbrella_order_id;
  int size;  // (total, includes partial fills)
  int user_data;
  int netter_group;

  // size that the pending order will eventually be
  // only applicable for pending orders
  int pending_size;

  double price;
  int total_size_filled;

  ExistingUmbrellaOrderStatus::Enum status;

  inline bool is_in_flight() const {
    return (status != ExistingUmbrellaOrderStatus::Acknowledged);
  }

  inline int size_left() const {
    return size - total_size_filled;
  }
};

inline bool ExistingUmbrellaOrderCompareAscending(
    const ExistingUmbrellaOrder & a,
    const ExistingUmbrellaOrder & b) {
  return a.price < b.price;
}

inline bool ExistingUmbrellaOrderCompareDescending(
    const ExistingUmbrellaOrder & a,
    const ExistingUmbrellaOrder & b) {
  return a.price > b.price;
}

#endif  // SRC_UMBRELLA_NETTER_EXISTING_UMBRELLA_ORDER_H_
