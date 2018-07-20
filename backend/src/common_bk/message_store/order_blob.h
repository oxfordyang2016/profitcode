#ifndef SRC_COMMON_MESSAGE_STORE_ORDER_BLOB_H_
#define SRC_COMMON_MESSAGE_STORE_ORDER_BLOB_H_

#include <string.h>

#include "common/orders/order_side.h"

struct OrderBlob {
  int msg_seq_num;
  OrderSide::Enum side;
  char order_id[24];
  char cl_order_id[20];

  OrderBlob() :
      msg_seq_num(0),
      side(OrderSide::Buy) {
    order_id[0] = 0;
    cl_order_id[0] = 0;
  }

  bool Equals(const OrderBlob & rhs) {
    if (msg_seq_num != rhs.msg_seq_num) {
      return false;
    }
    if (side != rhs.side) {
      return false;
    }
    if (strncmp(order_id, rhs.order_id, sizeof(order_id))) {
      return false;
    }
    if (strncmp(cl_order_id, rhs.cl_order_id, sizeof(cl_order_id))) {
      return false;
    }
    return true;
  }
};

#endif  // SRC_COMMON_MESSAGE_STORE_ORDER_BLOB_H_
