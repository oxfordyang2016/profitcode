#ifndef SRC_TOOLS_LIMITS_PUBLISHER_LIMITS_PUBLISHER_H_
#define SRC_TOOLS_LIMITS_PUBLISHER_LIMITS_PUBLISHER_H_

#include <shq/shq.h>
#include <zmq.hpp>

#include <map>
#include <set>
#include <string>
#include <sstream>

#include "zshared/lock.h"

#define LIMITS_TOPIC "LIMITS_POSITION"

struct FileInfo;
class SharedState;

struct Limits {
  Limits():ticker_pattern(""), position_limit(0) {}
  Limits(const std::string& ticker, int position) {
    ticker_pattern = ticker;
    position_limit = position;
  }
  std::string ticker_pattern;
  int position_limit;
};

static std::string GetLimitsMsg(const Limits& data);

static std::string GetKey(const std::string & domain,
                   const std::string & file_path);

class LimitsPublisher {
 public:
  LimitsPublisher(const std::string & cmd_topic,
                  SharedState* shared_state,
                  const std::string & update_address,
                  const std::string & domain,
                  const std::string & limits_path,
                  const std::string & prefix);

  void Run();
  void SendLimits(zmq::socket_t* socket);
  void WriteLimitsConfig();

  void UpdateLimits(const Limits& limits);
  void ReloadLimits();

 private:
  void UpdateFile(FileInfo* file_info, bool send_message);

 private:
  shq::Publisher cmd_publisher;
  SharedState* shared_state_;

  zmq::context_t context_;
  zmq::socket_t socket_;

  std::string domain_;
  std::string limits_path_;
  std::string prefix_;
  int sequence_;

  typedef std::map<std::string, Limits> LimitsPositionMap;
  LimitsPositionMap limits_position_;
  // is limits map update and write to config file
  bool limits_updated_;

  zshared::Mutex mutex;
};

#endif  // SRC_TOOLS_LIMITS_PUBLISHER_LIMITS_PUBLISHER_H_
