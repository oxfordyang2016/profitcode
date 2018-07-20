#ifndef SRC_COMMON_KV_MESSAGE_H_
#define SRC_COMMON_KV_MESSAGE_H_

#include <stdlib.h>
#include <zmq.hpp>
#include <string>

#include "common/zhelpers.hpp"

// the has_data flag signals if the KVMessage
// contains a key, value, and seq_num or if its
// an empty message

class KVMessage {
 public:
  explicit KVMessage()
      : has_data_(false),
        has_more_(false),
        seq_num_() {
  }

  explicit KVMessage(const char* key,
                     int seq_num,
                     const char* value,
                     bool has_more = false)
      : has_data_(true),
        has_more_(has_more),
        key_(key),
        seq_num_(seq_num),
        value_(value) {
  }

  explicit KVMessage(const std::string & key,
                     int seq_num,
                     const std::string & value,
                     bool has_more = false)
      : has_data_(true),
        has_more_(has_more),
        key_(key),
        seq_num_(seq_num),
        value_(value) {
  }

  inline const std::string & key() const { return key_; }
  inline int seq_num() const { return seq_num_; }
  inline const std::string & value() const { return value_; }

  inline bool has_data() const { return has_data_; }
  inline bool has_more() const { return has_more_; }
  inline void set_has_more(bool has_more) { has_more_ = has_more; }

  void Send(zmq::socket_t* socket) {
    s_sendmore(*socket, key_);

    zmq::message_t message(sizeof(seq_num_));
    memcpy(message.data(), &seq_num_, sizeof(seq_num_));
    socket->send(message, ZMQ_SNDMORE);

    if (has_more_) {
      s_sendmore(*socket, value_);
    } else {
      s_send(*socket, value_);
    }
  }

  static KVMessage Receive(zmq::socket_t* socket) {
    std::string key = s_recv(*socket);
    zmq::message_t message;
    socket->recv(&message);
    int* seq_num = static_cast<int*>(message.data());
    std::string value = s_recv(*socket);

    bool has_data = (key != "");
    bool has_more = s_more(*socket);

    if (!has_data) {
      return KVMessage();
    }

    return KVMessage(key, *seq_num, value, has_more);
  }

  void Show() {
    printf("%s %d %s\n", key_.c_str(), seq_num_, value_.c_str());
  }

 private:
  bool has_data_;
  bool has_more_;
  std::string key_;
  int seq_num_;
  std::string value_;
};

#endif  // SRC_COMMON_KV_MESSAGE_H_
