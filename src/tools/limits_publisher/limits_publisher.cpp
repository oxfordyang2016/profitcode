#include "tools/limits_publisher/limits_publisher.h"

#include <poll.h>
#include <sys/inotify.h>
#include <libconfig.h++>

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include <map>

#include "common/kv_message.h"
#include "common/limits.h"
#include "common/wanli_util.h"
#include "zshared/timer.h"
#include "zshared/shq_multipart_message.h"

#include "tools/log_publisher/shared_state.h"
#include "tools/log_publisher/util.h"
#include "tools/log_publisher/zrisk_state.h"

// Limits Publisher will publish all content
struct FileInfo {
  FILE* f;
  std::string path;
  char line[MAX_LINE_LENGTH];
  long line_offset;  // line position when reading partial lines at end of file:w
  long pos;  // keeps track of previous position in file
  int wd;
};

static std::string GetLimitsMsg(const Limits& data) {
  std::stringstream value;
  value << data.ticker_pattern << "|" << data.position_limit;
  return value.str();
}

static std::string GetKey(const std::string & domain,
                   const std::string & file_path) {
  std::string key = "/" +  domain + "/" + FileName(file_path) + "/" + LIMITS_TOPIC;
  return key;
}

LimitsPublisher::LimitsPublisher(const std::string & cmd_topic,
                  SharedState* shared_state,
                  const std::string & update_address,
                  const std::string & domain,
                  const std::string & limits_path,
                  const std::string & prefix)
  : cmd_publisher(cmd_topic),
    shared_state_(shared_state),
    context_(1),
    socket_(context_, ZMQ_PUB),
    domain_(domain),
    limits_path_(limits_path),
    prefix_(prefix),
    limits_updated_(true) {
  // 300k message buffer outgoing limit
  uint64_t hwm = 300000;
  socket_.setsockopt(ZMQ_HWM, &hwm, sizeof(hwm));

  sequence_ = 0;
  try {
    socket_.bind(update_address.c_str());
  } catch(zmq::error_t error) {
    LOG_ERROR("binding error: %s", error.what());
    exit(1);
  }

  sleep(1);
}

void LimitsPublisher::Run() {
  int fd = inotify_init();
  if (fd < 0) {
    perror("inotify_init");
  }

  char buf[1 * sizeof(inotify_event)];

  FileInfo info;
  info.path = limits_path_;
  info.pos = 0;
  info.line_offset = 0;
  // LOG_INFO("info.path=%s", info.path.c_str());

  // go through all files (don't publish while getting to the end..)
  info.wd = inotify_add_watch(fd, info.path.c_str(), (IN_MODIFY));
  info.f = fopen(info.path.c_str(), "r");
  if (!info.f) {
    LOG_ERROR("Could not open %s", info.path.c_str());
    exit(1);
  }
  UpdateFile(&info, false);

  // poll for updates and publish new lines
  zshared::Timer heartbeat_timer;
  zshared::Timer zrisk_timer;
  while (true) {
    if (fabs(zrisk_timer.ElapsedSeconds()) > 0.25) {
      zrisk_timer.Reset();

      for (std::map<std::string, ZRiskState>::iterator it = shared_state_->zrisk_state_.begin(); it != shared_state_->zrisk_state_.end(); ++it) {
        it->second.SendResidual(&socket_);
      }
    }

    if (fabs(heartbeat_timer.ElapsedSeconds()) > 10.0) {
      heartbeat_timer.Reset();

      KVMessage message;
      message.Send(&socket_);  // send heartbeat
    }
    //////////////////////////////////////////////////////////////////
    // check file updated
    struct pollfd pfd = { fd, POLLIN, 0 };
    int ret = poll(&pfd, 1, 10);  // Sleep for up to 10ms
    // LOG_INFO("POLL ret:%d fd:%d", ret, fd);
    if (ret < 0) {
      LOG_ERROR("poll failed: %s", strerror(errno));
      exit(1);
    } else if (ret != 0) {
      size_t len = read(fd, buf, sizeof(buf));

      for (size_t e = 0; e < len; ) {
        inotify_event *ev = reinterpret_cast<inotify_event*>(&buf[e]);

        if (info.wd != ev->wd)
          continue;

        if (ev->mask & IN_MODIFY) {
          UpdateFile(&info, true);
        } else {
          close(info.wd);
          LOG_ERROR("Ending Publishing");
          return;
        }
        e += sizeof(inotify_event) + ev->len;
      }
    }
  }  // end while
}

void LimitsPublisher::SendLimits(zmq::socket_t* socket) {
  int line = 0;
  for (LimitsPositionMap::iterator it = limits_position_.begin(); it != limits_position_.end(); it++) {
    std::string ticker = it->first;
    std::string key = GetKey(domain_, limits_path_);
    std::string value = GetLimitsMsg(limits_position_[ticker]);
    KVMessage message(key, line, value.c_str(), true);
    LOG_DEBUG("message: key=%s value=%s", key.c_str(), value.c_str());
    ++line;
    message.Send(socket);
  }
}

void LimitsPublisher::UpdateLimits(const Limits& limits) {
  if (limits_position_.find(limits.ticker_pattern) == limits_position_.end()) {
    LOG_ERROR("not found ticker:%s in limits list", limits.ticker_pattern.c_str());
    return;
  }
  limits_position_[limits.ticker_pattern].position_limit = limits.position_limit;
  limits_updated_ = false;
}

void LimitsPublisher::ReloadLimits() {
  zshared::MultipartMessageBuilder message;
  timeval tv;
  std::string action_string = "RELOAD_LIMITS";

  gettimeofday(&tv, NULL);
  message.AddMessage(&tv, sizeof(tv));
  message.AddMessage(action_string.c_str(), action_string.size());
  cmd_publisher.Send(message.GetData(), message.GetSize());
}

void LimitsPublisher::UpdateFile(FileInfo* file_info, bool send_message) {
  zshared::LockGuard<zshared::Mutex> guard(&mutex);

  libconfig::Config config;
  try {
    config.readFile(file_info->path.c_str());
  } catch (const libconfig::FileIOException & fioex) {
    LOG_ERROR("Could not read file '%s'", file_info->path.c_str());
    exit(1);
  } catch (const libconfig::ParseException & pex) {
    LOG_ERROR("Error parsing file, check syntax in the config file");
    exit(1);
  }
  try {
    libconfig::Setting& limits = config.lookup("instrument_limits");
    bool flag = false;
    for (int i = 0; i < limits.getLength(); i++) {
      flag = false;
      std::string ticker = limits[i]["ticker_pattern"].c_str();
      int position = limits[i]["position_limit"];
      LimitsPositionMap::iterator it = limits_position_.find(ticker);
      if (it == limits_position_.end()) {
        // ignore unset prefix
        if (ticker.substr(0, 3) != prefix_)
          continue;

        Limits data(ticker, position);
        limits_position_[ticker] = data;

        flag = true;
      } else if (position != limits_position_[ticker].position_limit) {
        limits_position_[ticker].position_limit = position;
        flag = true;
      }

      std::string key = GetKey(domain_, limits_path_);
      std::string value = GetLimitsMsg(limits_position_[ticker]);
      if ((send_message && flag) ||
          // recv update and write to file, send all to client
          !limits_updated_) {
        // LOG_INFO("send message: ticker:%s position:%d", ticker.c_str(), position);
        KVMessage message(key, sequence_, value.c_str(), false);
        ++sequence_;
        message.Send(&socket_);
      }
    }
    // update for pnl_limit
    flag = false;
    int pnl = config.lookup("pnl_limit");
    std::string ticker = "pnl_limit";
    LimitsPositionMap::iterator it = limits_position_.find(ticker);
    if (it == limits_position_.end()) {
      Limits data(ticker, pnl);
      limits_position_[ticker] = data;

      flag = true;
    } else if (pnl != limits_position_[ticker].position_limit) {
      limits_position_[ticker].position_limit = pnl;
      flag = true;
    }
    std::string key = GetKey(domain_, limits_path_);
    std::string value = GetLimitsMsg(limits_position_[ticker]);
    if ((send_message && flag) || !limits_updated_) {
      KVMessage message(key, sequence_, value.c_str(), false);
      ++sequence_;
      message.Send(&socket_);
    }

    if (!limits_updated_) {
      limits_updated_ = true;
    }
  } catch (const libconfig::SettingNotFoundException &nfex) {
    LOG_ERROR("Setting '%s' is missing", nfex.getPath());
    return;
  } catch (const libconfig::SettingTypeException &tex) {
    LOG_ERROR("Setting '%s' has the wrong type", tex.getPath());
    return;
  }
}

void LimitsPublisher::WriteLimitsConfig() {
  zshared::LockGuard<zshared::Mutex> guard(&mutex);
  try {
    libconfig::Config config;
    config.readFile(limits_path_.c_str());

    libconfig::Setting& limits = config.lookup("instrument_limits");
    for (int i = 0; i < limits.getLength(); i++) {
      int position = limits[i]["position_limit"];
      LimitsPositionMap::iterator it_limits = limits_position_.find(limits[i]["ticker_pattern"]);
      if (it_limits == limits_position_.end()) {
        LOG_ERROR("Can't find ticker:%s in limits.conf", limits[i]["ticker_pattern"].c_str());
        return;
      }
      if (it_limits->second.position_limit != position) {
        // update config
        limits[i]["position_limit"] = it_limits->second.position_limit;
      }
    }
    // update pnl_limit
    int pnl = config.lookup("pnl_limit");
    LimitsPositionMap::iterator it_limits = limits_position_.find("pnl_limit");
    if (it_limits != limits_position_.end() && it_limits->second.position_limit != pnl) {
      config.lookup("pnl_limit") = it_limits->second.position_limit;
    }

    config.writeFile(limits_path_.c_str());
  } catch (const libconfig::FileIOException & fioex) {
    LOG_ERROR("Could not read file '%s'", limits_path_.c_str());
    return;
  } catch (const libconfig::ParseException & pex) {
    LOG_ERROR("Error parsing file, check syntax in the config file");
    return;
  } catch (const libconfig::SettingNotFoundException &nfex) {
    LOG_ERROR("Setting '%s' is missing", nfex.getPath());
    return;
  } catch (const libconfig::SettingTypeException &tex) {
    LOG_ERROR("Setting '%s' has the wrong type", tex.getPath());
    return;
  }
}
