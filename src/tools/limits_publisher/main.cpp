#include <stdio.h>
#include <stdlib.h>
#include <zmq.hpp>
#include <libconfig.h++>
#include <sys/time.h>

#include <cstdatomic>
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <utility>
#include <vector>

#include "common/limits.h"
#include "common/kv_message.h"
#include "common/wanli_util.h"
#include "zshared/lock.h"
#include "zshared/timer.h"

#include "tools/log_publisher/shared_state.h"
#include "tools/limits_publisher/limits_publisher.h"

#define EXCEED_SEC  60
#define SLEEP_USEC  60000000

struct UserInfo {
  UserInfo() {
    is_logon = false;
    seconds = 0;
  }
  UserInfo(const UserInfo& u) {
    username = u.username;
    password = u.password;
    is_logon = u.is_logon;
    seconds = u.seconds.load();
  }
  UserInfo& operator=(const UserInfo& u) {
    username = u.username;
    password = u.password;
    is_logon = u.is_logon;
    seconds = u.seconds.load();
    return (*this);
  }

  std::string username;
  std::string password;
  bool is_logon;
  // if long time no heartbeat, user to logout
  std::atomic_llong seconds;
};
typedef std::vector<UserInfo> UserList;

////////////////////////////////////////////////////////////////
// save user info
UserList users;
UserList login_users;
zshared::Mutex lock;
// user_lists_path
std::string user_lists;

////////////////////////////////////////////////////////////////
static void HandleRequest(zmq::socket_t* socket,
                          LimitsPublisher* limits,
                          const KVMessage & message);
static void* RunPublisher(void* arg);
static void* RunUserWorker(void* arg);
SharedState shared_state;

int split(const std::string& str, std::vector<std::string>& ret_, std::string sep = " ") {
  if (str.empty()) {
      return 0;
  }

  std::string tmp;
  std::string::size_type pos_begin = str.find_first_not_of(sep);
  std::string::size_type comma_pos = 0;

  while (pos_begin != std::string::npos) {
    comma_pos = str.find(sep, pos_begin);
    if (comma_pos != std::string::npos) {
      tmp = str.substr(pos_begin, comma_pos - pos_begin);
      pos_begin = comma_pos + sep.length();
    } else {
      tmp = str.substr(pos_begin);
      pos_begin = comma_pos;
    }

    if (!tmp.empty()) {
      ret_.push_back(tmp);
      tmp.clear();
    }
  }
  return 0;
}

inline UserInfo* GetUser(UserList* list, const std::string& username) {
  for (auto it = list->begin(); it != list->end(); it++) {
    if (it->username == username) {
      return &(*it);
    }
  }
  return NULL;
}

inline void PopUser(UserList* list, const std::string& username) {
  zshared::LockGuard<zshared::Mutex> guard(&lock);
  for (auto it = list->begin(); it != list->end(); it++) {
    if (it->username == username) {
      list->erase(it);
      return;
    }
  }
}

inline void PushUser(UserList* list, UserInfo* user) {
  zshared::LockGuard<zshared::Mutex> guard(&lock);
  list->push_back(*user);
}

inline int CheckPassword(UserList* list, UserInfo* user) {
  for (auto it = list->begin(); it != list->end(); it++) {
    if (it->username == user->username) {
      if (it->password == user->password) {
        return 0;
      } else {
        return 1;
      }
    }
  }
  return 2;
}

inline bool IsLogined(UserList* list, const std::string& username) {
  for (auto it = list->begin(); it != list->end(); it++) {
    if (it->username == username && it->is_logon) {
      return true;
    }
  }
  return false;
}

inline void UpdateTime(UserInfo* user) {
  timeval tv;
  gettimeofday(&tv, NULL);
  user->seconds = tv.tv_sec;
}

int ChangePassword(const std::string& user, const std::string& pwd) {
    LOG_DEBUG("user chpwd: %s|%s", user.c_str(), pwd.c_str());
  try {
    bool flag = false;
    libconfig::Config conf_users;
    conf_users.readFile(user_lists.c_str());

    libconfig::Setting& set_users = conf_users.lookup("users");
    for (int i = 0; i < set_users.getLength(); i++) {
      if (user == set_users[i]["username"].c_str()) {
        set_users[i]["password"] = pwd;
        flag = true;
        for (auto it = users.begin(); it != users.end(); it++) {
          it->password = pwd;
        }
      }
    }

    if (flag) {
      conf_users.writeFile(user_lists.c_str());
    } else {
      // not found user
      return -2;
    }
  } catch (const std::exception& e) {
    return -1;
  }
  return 0;
}
////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage:   %s <config_file>\n", argv[0]);
    fprintf(stderr, "Examples: %s limit_publisher.conf\n", argv[0]);
    exit(1);
  }

  libconfig::Config config;
  try {
    config.readFile(argv[1]);
  } catch (const libconfig::FileIOException & fioex) {
    LOG_ERROR("Could not read file '%s'", argv[1]);
    exit(1);
  } catch (const libconfig::ParseException & pex) {
    LOG_ERROR("Error parsing file, check syntax in the config file");
    exit(1);
  }

  std::string history_address;
  std::string update_address;
  std::string limits_path;
  std::string domain;
  std::string command_topic;
  std::string prefix;
  try {
    history_address = (const char*)config.lookup("history_address");
    update_address = (const char*)config.lookup("update_address");
    limits_path = config.lookup("limits_path").c_str();
    domain = config.lookup("domain").c_str();
    command_topic = config.lookup("command_topic").c_str();
    user_lists = config.lookup("user_lists").c_str();
    prefix = config.lookup("prefix").c_str();

    LOG_DEBUG("user_lists:%s", user_lists.c_str());
    {
      libconfig::Config conf_users;
      conf_users.readFile(user_lists.c_str());

      libconfig::Setting& set_users = conf_users.lookup("users");
      for (int i = 0; i < set_users.getLength(); i++) {
        UserInfo info;
        info.username = set_users[i]["username"].c_str();
        info.password = set_users[i]["password"].c_str();

        users.push_back(info);
      }
    }

    for (auto it = users.begin(); it != users.end(); it++) {
      LOG_INFO("user: %s|%s", it->username.c_str(), it->password.c_str());
    }
  } catch (const libconfig::SettingNotFoundException &nfex) {
    LOG_ERROR("Setting '%s' is missing", nfex.getPath());
    exit(1);
  } catch (const libconfig::SettingTypeException &tex) {
    LOG_ERROR("Setting '%s' has the wrong type", tex.getPath());
    exit(1);
  }

  ///////////////////////////////////////
  // live updates
  LimitsPublisher publisher(command_topic, &shared_state, update_address, domain, limits_path, prefix);
  LOG_INFO("Running LimitsPublisher on '%s'", update_address.c_str());

  pthread_t publishing_thread;
  if (pthread_create(&publishing_thread,
                     NULL,
                     &RunPublisher,
                     static_cast<void*>(&publisher)) != 0) {
    LOG_ERROR("Could not create publishing thread");
    exit(1);
  }
  pthread_t userwork_thread;
  if (pthread_create(&userwork_thread,
                     NULL,
                     &RunUserWorker,
                     NULL) != 0) {
    LOG_ERROR("Could not create userworker thread");
    exit(1);
  }

  ///////////////////////////////////////
  // history
  zmq::context_t context(1);
  zmq::socket_t history_socket(context, ZMQ_REP);

  // 300k message buffer outgoing limit
  uint64_t hwm = 300000;
  history_socket.setsockopt(ZMQ_HWM, &hwm, sizeof(hwm));

  try {
    history_socket.bind(history_address.c_str());
  } catch(zmq::error_t error) {
    LOG_ERROR("binding error: %s", error.what());
    exit(1);
  }

  LOG_INFO("Running History Store on '%s'", history_address.c_str());
  while (true) {
    KVMessage message = KVMessage::Receive(&history_socket);
    HandleRequest(&history_socket, &publisher, message);
  }

  return 0;
}

////////////////////////////////////////////////////////////////
static void* RunPublisher(void* arg) {
  LimitsPublisher* publisher = static_cast<LimitsPublisher*>(arg);
  publisher->Run();
  return NULL;
}

static void* RunUserWorker(void* arg) {
  while (true) {
    {
      timeval tv;
      gettimeofday(&tv, NULL);

      zshared::LockGuard<zshared::Mutex> guard(&lock);
      for (auto it = login_users.begin(); it != login_users.end(); it++) {
        if (tv.tv_sec - it->seconds.load() > EXCEED_SEC) {
          LOG_DEBUG("[%s] no heartbeats long time", it->username.c_str());
          login_users.erase(it);
          it--;
        }
      }
    }
    usleep(SLEEP_USEC);
  }
  return NULL;
}

static void HandleRequest(zmq::socket_t* socket,
                          LimitsPublisher* limits,
                          const KVMessage & message) {
  if (!message.has_data()) return;

  if (message.key() == "query_limits") {
    limits->SendLimits(socket);

    KVMessage null_msg;
    null_msg.Send(socket);
  }
  if (message.key() == "update_limits") {
    bool has_more = message.has_more();
    bool is_first = true;
    while (has_more) {
      if (is_first) {
        std::vector<std::string> limits_list;
        split(message.value(), limits_list, "|");
        LOG_DEBUG("[update_limits] value:%s 1:%s 2:%s", message.value().c_str(), limits_list[0].c_str(), limits_list[1].c_str());

        Limits data(limits_list[0], atoi(limits_list[1].c_str()));
        limits->UpdateLimits(data);
        is_first = false;
      } else {
        KVMessage msg = KVMessage::Receive(socket);
        has_more = msg.has_more();

        if (msg.has_data()) {
          std::vector<std::string> limits_list;
          split(msg.value(), limits_list, "|");
          LOG_DEBUG("[update_limits] value:%s 1:%s 2:%s", msg.value().c_str(), limits_list[0].c_str(), limits_list[1].c_str());

          Limits data(limits_list[0], atoi(limits_list[1].c_str()));
          limits->UpdateLimits(data);
        }
      }
    }
    limits->WriteLimitsConfig();

    // send a response
    KVMessage null_msg;
    null_msg.Send(socket);
  }
  if (message.key() == "reload_limits") {
    limits->ReloadLimits();

    KVMessage null_msg;
    null_msg.Send(socket);
  }
  if (message.key() == "heartbeat") {
    if (message.value().size() > 0) {
      std::string username = message.value();
      UserInfo* user = GetUser(&login_users, username);
      if (user) {
        // update time
        UpdateTime(user);
        LOG_DEBUG("[%s] recv heartbeat, sec:%zd", username.c_str(), user->seconds.load());
      }
    }
    KVMessage null_msg;
    null_msg.Send(socket);
  }
  if (message.key() == "login") {
    std::string ret_msg;
    if (message.value().size() > 0) {
      std::vector<std::string> datas;
      split(message.value(), datas, "|");
      LOG_DEBUG("[Login] value:%s", message.value().c_str());

      UserInfo user;
      user.username = datas[0];
      user.password = datas[1];
      int ret = CheckPassword(&users, &user);
      if (ret == 0) {
        if (IsLogined(&login_users, user.username)) {
          ret_msg = "User Already Logined";
        } else {
          UpdateTime(&user);
          user.is_logon = true;
          PushUser(&login_users, &user);
          ret_msg = "success";
        }
      } else {
        if (ret == 1) {
          ret_msg = "Password Error";
        } else if (ret == 2) {
          ret_msg = "User Not Exists";
        }
      }
    } else {
      ret_msg = "Wrong Login Params";
    }

    KVMessage null_msg("loginrsp", 0, ret_msg);
    null_msg.Send(socket);
  }
  if (message.key() == "logout") {
    if (message.value().size() > 0) {
      std::vector<std::string> datas;
      split(message.value(), datas, "|");
      LOG_DEBUG("[Logout] value:%s", message.value().c_str());

      PopUser(&login_users, datas[0]);
    }

    KVMessage null_msg;
    null_msg.Send(socket);
  }
  if (message.key() == "chpwd") {
    std::string ret_msg;
    if (message.value().size() > 0) {
      std::vector<std::string> datas;
      split(message.value(), datas, "|");
      if (datas.size() < 2) {
        ret_msg = "receive wrong parameters";
      } else {
        int ret = ChangePassword(datas[0], datas[1]);
        if (ret == 0) {
          ret_msg = "success";
        } else if (ret == -1) {
          ret_msg = "update password failed";
        } else if (ret == -2) {
          ret_msg = "update unexists username";
        }
      }
    }

    KVMessage null_msg("chpwdrsp", 0, ret_msg);
    null_msg.Send(socket);
  }
}
