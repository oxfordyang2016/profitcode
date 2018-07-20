#ifndef SRC_TOOLS_LOG_PUBLISHER_ZRISK_STATE_H_
#define SRC_TOOLS_LOG_PUBLISHER_ZRISK_STATE_H_

#include <string.h>
#include <stdio.h>

#include <zmq.hpp>

#include <vector>
#include <map>
#include <set>
#include <string>

#include "common/kv_message.h"
#include "zshared/lock.h"

#include "tools/log_publisher/util.h"

class ZRiskState {
 public:
  void Update(zmq::socket_t* socket, const KVMessage & message) {
    zshared::LockGuard<zshared::Mutex> guard(&mutex_);

    kv_message_batch_.push_back(message);

    const int batch_size = 200000;
    if (static_cast<int>(kv_message_batch_.size()) >= batch_size) {
      SendKVMessageBatch(socket);
    }
  }

  void SendResidual(zmq::socket_t* socket) {
    zshared::LockGuard<zshared::Mutex> guard(&mutex_);

    // send any residual kv messages
    if (kv_message_batch_.size() > 0) {
      SendKVMessageBatch(socket);
    }
  }

  void SendAllState(zmq::socket_t* socket) {
    zshared::LockGuard<zshared::Mutex> guard(&mutex_);

    // Non-order history
    for (size_t i = 0; i < history_.size(); ++i) {
      history_[i].set_has_more(true);
      history_[i].Send(socket);
    }

    // Order history needs to be sent in sequence number order
    std::map<int, KVMessage> orders;
    for (std::map<std::string, KVMessage>::iterator it = active_orders_.begin(); it != active_orders_.end(); ++it) {
      orders[it->second.seq_num()] = it->second;
    }
    for (std::map<int, KVMessage>::iterator it = orders.begin(); it != orders.end(); ++it) {
      it->second.set_has_more(true);
      it->second.Send(socket);
    }

    // Empty message terminates the stream
    KVMessage message;
    message.Send(socket);
  }

 private:
  static std::string GetWord(const std::string & value, int word) {
    bool in_space_state = true;
    int word_count = 0;
    for (size_t j = 0; j < value.size(); ++j) {
      if (value[j] == ' ') {
        in_space_state = true;
      } else {
        if (in_space_state) {
          in_space_state = false;
          ++word_count;
        }
      }
      if (word_count == word) {
        int end_idx = j+1;
        for (; end_idx < static_cast<int>(value.size()); ++end_idx) {
          if (value[end_idx] == ' ') {
            break;
          }
        }
        int client_id_size = end_idx - j;
        return std::string(value.c_str() + j, client_id_size);
      }
    }
    return "";
  }

  static inline bool HasEnding(const std::string & full_string, const std::string & ending) {
    if (full_string.length() >= ending.length()) {
      return (0 == full_string.compare(full_string.length() - ending.length(), ending.length(), ending));
    } else {
      return false;
    }
  }

  void SendKVMessageBatch(zmq::socket_t* socket) {
    // example value: 1401366625 894022 SFIT/CUF6/ cancelled SELL 0 0 0 1.36174 W17070901000EBD 252
    const int order_status_idx = 4;
    const int client_order_id_idx = 10;

    // first pass to figure out order statuses
    std::map<std::string, bool> order_in_done_status;
    for (size_t i = 0; i < kv_message_batch_.size(); ++i) {
      if (!HasEnding(kv_message_batch_[i].key(), "/ORDER")) {
        LOG_DEBUG("skipping type %s", kv_message_batch_[i].key().c_str());
        continue;
      }
      std::string order_status = GetWord(kv_message_batch_[i].value(), order_status_idx);
      std::string client_order_id = GetWord(kv_message_batch_[i].value(), client_order_id_idx);
      // LOG_DEBUG("status:%s client_order_id:%s", order_status.c_str(), client_order_id.c_str());
      if (order_status.size() == 0 ||
          client_order_id.size() == 0) {
        continue;
      }

      bool is_done = order_status == "filled" ||
                     order_status == "cancelled";
      order_in_done_status[client_order_id] = is_done;
    }

    for (size_t i = 0; i < kv_message_batch_.size(); ++i) {
      if (HasEnding(kv_message_batch_[i].key(), "/ORDER")) {
        std::string client_order_id = GetWord(kv_message_batch_[i].value(), client_order_id_idx);

        if (order_in_done_status[client_order_id]) {
          // can skip orders that are in done status (that we haven't already sent out a message for)
          std::map<std::string, KVMessage>::iterator it = active_orders_.find(client_order_id);
          if (it == active_orders_.end()) {
            continue;
          }

          std::string order_status = GetWord(kv_message_batch_[i].value(), order_status_idx);
          bool is_done = order_status == "filled" ||
                         order_status == "cancelled";

          if (is_done) {
            // Remove active orders when they are done, so we don't overflow memory
            active_orders_.erase(it);
          }
        } else {
          active_orders_[client_order_id] = kv_message_batch_[i];
        }
      } else {
        history_.push_back(kv_message_batch_[i]);
      }
      kv_message_batch_[i].Send(socket);
    }

    kv_message_batch_.clear();
  }

 private:
  std::vector<KVMessage> kv_message_batch_;

  std::map<std::string, KVMessage> active_orders_;
  std::vector<KVMessage> history_;

  zshared::Mutex mutex_;
};

#endif  // SRC_TOOLS_LOG_PUBLISHER_ZRISK_STATE_H_
