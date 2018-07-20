#ifndef SRC_TOOLS_LOG_PUBLISHER_LOG_PUBLISHER_H_
#define SRC_TOOLS_LOG_PUBLISHER_LOG_PUBLISHER_H_

#include <zmq.hpp>

#include <map>
#include <set>
#include <string>

#include "zshared/lock.h"

struct FileInfo;
class SharedState;

class LogPublisher {
 public:
  LogPublisher(SharedState* shared_state,
               const std::string & update_address,
               const std::map<std::string, std::set<std::string> > & file_domains);
  void Run();

 private:
  void UpdateFile(FileInfo* file_info, bool send_message);

 private:
  SharedState* shared_state_;

  zmq::context_t context_;
  zmq::socket_t socket_;

  std::map<std::string, int> topic_line_;
  std::map<std::string, std::set<std::string> > file_domains_;
};

#endif  // SRC_TOOLS_LOG_PUBLISHER_LOG_PUBLISHER_H_
