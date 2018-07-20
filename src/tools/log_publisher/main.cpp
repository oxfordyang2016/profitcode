#include <stdio.h>
#include <stdlib.h>
#include <zmq.hpp>
#include <libconfig.h++>

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

#include "tools/log_publisher/log_publisher.h"
#include "tools/log_publisher/shared_state.h"
#include "tools/log_publisher/zrisk_state.h"
#include "tools/log_publisher/util.h"

struct RequestInfo {
  std::string domain;
  std::string file_path;
};

typedef std::map<std::string, RequestInfo> RequestMap;

static void SendFile(zmq::socket_t* socket,
                     const std::string & domain,
                     const std::string & file_path);

static void HandleRequest(zmq::socket_t* socket,
                          const std::string & request,
                          const RequestMap & request_map);

static void* RunPublisher(void* arg);

SharedState shared_state;

////////////////////////////////

int main(int argc, char** argv) {
  if (argc != 2 && argc != 3) {
    fprintf(stderr, "Usage:   %s <config_file> [domain]\n", argv[0]);
    fprintf(stderr, "Examples: %s log_publisher.conf\n", argv[0]);
    fprintf(stderr, "          %s log_publisher.conf domainA\n", argv[0]);
    fprintf(stderr, "If domain is specified on command line, it overrides config file\n");
    exit(1);
  }

  bool has_domain = false;
  std::string domain;
  if (argc == 3) {
    has_domain = true;
    domain = argv[2];
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
  RequestMap request_map;
  std::map<std::string, std::set<std::string> > file_domains;
  std::map<std::string, bool> domain_history_flags;

  try {
    history_address = (const char*)config.lookup("history_address");
    update_address = (const char*)config.lookup("update_address");

    libconfig::Setting & domains = config.lookup("domains");
    for (int d = 0; d < domains.getLength(); ++d) {
      if (!has_domain) {
        domain = (const char*)domains[d]["domain"];
      }

      bool is_zrisk = false;
      if (domains[d].exists("is_zrisk")) {
        is_zrisk = domains[d]["is_zrisk"];
        shared_state.zrisk_domains_.insert(domain);
      }
      LOG_INFO("is_zrisk domain = %d", is_zrisk);

      for (int i = 0; i < domains[d]["file_paths"].getLength(); ++i) {
        std::string file_path = domains[d]["file_paths"][i];
        std::string file_name = FileName(file_path);
        std::string request = "/" + domain + "/" + file_name + "/*";
        std::string request_latest = "/" + domain + "/" + file_name + "/1";

        std::set<std::string> file_paths;
        file_domains[file_path].insert(domain);

        LOG_INFO("mapping request %s -> (%s, %s)",
                 request.c_str(),
                 domain.c_str(),
                 file_name.c_str());
        LOG_INFO("mapping request %s -> (%s, %s)",
                 request_latest.c_str(),
                 domain.c_str(),
                 file_name.c_str());
        RequestInfo info;
        info.domain = domain;
        info.file_path = file_path;
        request_map[request] = info;
        request_map[request_latest] = info;

        if (is_zrisk) {
          std::string request_fast_zrisk_log = "/" + domain + "/" + file_name + "/FASTZRISKLOG";
          LOG_INFO("mapping request %s -> (%s, %s)",
                   request_fast_zrisk_log.c_str(),
                   domain.c_str(),
                   file_name.c_str());
          shared_state.zrisk_state_[request_fast_zrisk_log] = ZRiskState();
          request_map[request_fast_zrisk_log] = info;
        }
      }
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
  LogPublisher log_publisher(&shared_state, update_address, file_domains);
  LOG_INFO("Running LogPublisher on '%s'", update_address.c_str());

  pthread_t publishing_thread;
  if (pthread_create(&publishing_thread,
                     NULL,
                     &RunPublisher,
                     static_cast<void*>(&log_publisher)) != 0) {
    LOG_ERROR("Could not create publishing thread");
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
    std::string request = s_recv(history_socket);
    LOG_DEBUG("received request '%s'", request.c_str());
    HandleRequest(&history_socket, request, request_map);
  }

  return 0;
}

static void* RunPublisher(void* arg) {
  LogPublisher* log_publisher = static_cast<LogPublisher*>(arg);
  log_publisher->Run();
  return NULL;
}

//////////////////////////////////////

// helper functions

static void SendFile(zmq::socket_t* socket,
                     const std::string & domain,
                     const std::string & file_path) {
  FILE* f = fopen(file_path.c_str(), "r");
  if (!f) {
    LOG_ERROR("could not open file: %s", file_path.c_str());
    return;
  }

  std::map<std::string, int> topic_line;
  char line[MAX_LINE_LENGTH];
  while (fgets(line, MAX_LINE_LENGTH, f)) {
    std::string key;
    std::string value;
    GetKeyValue(domain, file_path, line, &key, &value);

    // send KVMessage where has_last = true, since there are more messages coming
    KVMessage message(key, topic_line[key], value, true);
    LOG_DEBUG("Sending %s %d %s", key.c_str(), topic_line[key], value.c_str());
    ++topic_line[key];
    message.Send(socket);
  }

  LOG_DEBUG("Done sending file");
  fclose(f);
}

static void HandleRequest(zmq::socket_t* socket,
                          const std::string & request,
                          const RequestMap & request_map) {
  std::stringstream ss(request);
  std::string item;
  std::vector<std::string> elems;
  while (std::getline(ss, item, '/')) {
    elems.push_back(item);
  }

  if (elems.size() != 4) {  // first element should be blank
    LOG_WARNING("expected 4 elements (with first element blank) for '%s'", request.c_str());
    // for backwards compatibility
    while (s_more(*socket)) {
      zmq::message_t message;
      socket->recv(&message);
    }
    KVMessage message;  // send blank message (end of sequence)
    message.Send(socket);
    return;
  }

  std::string domain = elems[1];
  std::string file = elems[2];
  std::string request_type = elems[3];

  // handle wild-card domain case
  std::set<std::string> handled_requests;  // avoids duplicate responses
  if (domain == "*") {
    RequestMap::const_iterator it;
    for (it = request_map.begin(); it != request_map.end(); ++it) {
      std::string request2 = "/" + it->second.domain + "/" + file + "/" + request_type;
      if (handled_requests.find(request2) != handled_requests.end()) {
        continue;
      }
      if (request_type == "1") {
        shared_state.SendSnapshot(socket, request2);
      } else {
        RequestMap::const_iterator it = request_map.find(request2);
        if (it == request_map.end()) {
          LOG_WARNING("request %s not mapped", request2.c_str());
          continue;
        }
        if (request_type == "FASTZRISKLOG") {
          LOG_INFO("handling wilcard request fastzrisklog: '%s' -> '%s'", request.c_str(), request2.c_str());
          shared_state.zrisk_state_[request2].SendAllState(socket);
        } else {
          LOG_INFO("handling wilcard request: '%s' -> '%s'", request.c_str(), request2.c_str());
          SendFile(socket, it->second.domain, it->second.file_path);
        }
      }
      handled_requests.insert(request2);
    }
    KVMessage message;
    message.Send(socket);
    return;
  }

  RequestMap::const_iterator it = request_map.find(request);
  if (it == request_map.end()) {
    LOG_WARNING("request %s not mapped", request.c_str());
    KVMessage message;  // send blank message (end of sequence)
    message.Send(socket);
    return;
  }

  if (request_type == "1") {
    shared_state.SendSnapshot(socket, request);
    KVMessage message;
    message.Send(socket);
    return;
  } else if (request_type == "FASTZRISKLOG") {
    LOG_INFO("handling request fastzrisklog: '%s'", request.c_str());
    shared_state.zrisk_state_[request].SendAllState(socket);
    return;
  }
  LOG_INFO("handling request: '%s'", request.c_str());

  SendFile(socket, it->second.domain, it->second.file_path);
  KVMessage message;
  message.Send(socket);
}

