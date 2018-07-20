#ifndef SRC_MARKET_HANDLERS_ZBATSDATA_TCP_SERVER_H_
#define SRC_MARKET_HANDLERS_ZBATSDATA_TCP_SERVER_H_

#include <cstring>
#include <string>
#include "market_handlers/zbatsdata/messages.h"

namespace bats {

class TcpServer {
 public:
  TcpServer(const std::string & address,
            const int port,
            const std::string & session_sub_id,
            const std::string & username,
            const std::string & password)
    : address_(address),
      port_(port),
      session_sub_id_(session_sub_id),
      username_(username),
      password_(password) {
  }

  std::string GetAddress() const {
    return address_;
  }

  int GetPort() const {
    return port_;
  }

  LoginMessage BuildLoginMessage() const {
    LoginMessage message;

    if (session_sub_id_.length() > sizeof(message.session_sub_id)) {
      throw std::runtime_error("Session sub id is too long");
    }
    if (username_.length() > sizeof(message.username)) {
      throw std::runtime_error("Username is too long");
    }
    if (password_.length() > sizeof(message.password)) {
      throw std::runtime_error("Password is too long");
    }

    memset(&message, ' ', sizeof(LoginMessage));
    message.header.length = sizeof(LoginMessage);
    message.header.message_type = kMessageTypeLogin;

    strncpy(message.session_sub_id, session_sub_id_.c_str(), session_sub_id_.length());
    strncpy(message.username, username_.c_str(), username_.length());
    strncpy(message.password, password_.c_str(), password_.length());

    return message;
  }


 private:
  std::string address_;
  int port_;
  std::string session_sub_id_;
  std::string username_;
  std::string password_;
};
}

#endif  // SRC_MARKET_HANDLERS_ZBATSDATA_TCP_SERVER_H_
