#ifndef SRC_TOOLS_LOG_PUBLISHER_SHARED_STATE_H_
#define SRC_TOOLS_LOG_PUBLISHER_SHARED_STATE_H_

#include <map>
#include <set>
#include <string>

#include "tools/log_publisher/zrisk_state.h"

class SharedState {
 public:
  void StoreLatest(const std::string & request, const std::string & key, const KVMessage & message) {
    zshared::LockGuard<zshared::Mutex> guard(&mutex_latest_);
    if (request_to_latest_[request].find(key) == request_to_latest_[request].end()) {
      int idx = request_to_idx_[request].size();
      request_to_idx_[request][idx] = key;
    }
    request_to_latest_[request][key] = message;
  }

  void SendSnapshot(zmq::socket_t* socket,
                   const std::string & request) {
    LOG_INFO("send latest snapshot (no history) for request %s", request.c_str());

    zshared::LockGuard<zshared::Mutex> guard(&mutex_latest_);
    std::map<int, std::string>::iterator it;
    for (it = request_to_idx_[request].begin();
         it != request_to_idx_[request].end();
         ++it) {
      KVMessage message = request_to_latest_[request][it->second];
      message.set_has_more(true);
      message.Send(socket);
    }
  }

 public:
  std::set<std::string> zrisk_domains_;
  // request -> ZRiskState
  std::map<std::string, ZRiskState> zrisk_state_;

 private:
  zshared::Mutex mutex_latest_;

  // request,key -> value
  std::map<std::string, std::map<std::string, KVMessage> > request_to_latest_;
  // request,idx -> key (used to preserve order of parameters)
  std::map<std::string, std::map<int, std::string> > request_to_idx_;
};

#endif  // SRC_TOOLS_LOG_PUBLISHER_SHARED_STATE_H_
