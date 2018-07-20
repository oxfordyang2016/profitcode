#ifndef SRC_COMMON_MESSAGE_STORE_MESSAGE_STORE_H_
#define SRC_COMMON_MESSAGE_STORE_MESSAGE_STORE_H_

#include <vector>

#include "common/message_store/order_blob.h"
#include "zshared/lock.h"

class MessageStore {
 public:
  explicit MessageStore(int store_size = 500);

  void Store(int msg_seq_num,
             OrderSide::Enum side,
             const char* order_id,
             const char* cl_order_id);

  OrderBlob Retrieve(int msg_seq_num);

 private:
  size_t current_idx_;
  zshared::SpinLock spin_lock_;
  std::vector<OrderBlob> order_blobs_;
};

#endif  // SRC_COMMON_MESSAGE_STORE_MESSAGE_STORE_H_
