#include "tools/log_publisher/log_publisher.h"

#include <poll.h>
#include <sys/inotify.h>

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "common/kv_message.h"
#include "common/limits.h"
#include "common/wanli_util.h"
#include "zshared/timer.h"

#include "tools/log_publisher/shared_state.h"
#include "tools/log_publisher/util.h"
#include "tools/log_publisher/zrisk_state.h"

struct FileInfo {
  FILE* f;
  std::string path;
  char line[MAX_LINE_LENGTH];
  long line_offset;  // line position when reading partial lines at end of file:w
  long pos;  // keeps track of previous position in file
  int wd;
};

LogPublisher::LogPublisher(SharedState* shared_state,
                           const std::string & update_address,
                           const std::map<std::string, std::set<std::string> > & file_domains)
  : shared_state_(shared_state),
    context_(1),
    socket_(context_, ZMQ_PUB),
    file_domains_(file_domains) {
  // 300k message buffer outgoing limit
  uint64_t hwm = 300000;
  socket_.setsockopt(ZMQ_HWM, &hwm, sizeof(hwm));

  try {
    socket_.bind(update_address.c_str());
  } catch(zmq::error_t error) {
    LOG_ERROR("binding error: %s", error.what());
    exit(1);
  }

  sleep(1);
}

void LogPublisher::Run() {
  int fd = inotify_init();
  if (fd < 0) {
    perror("inotify_init");
  }
  char buf[1 * sizeof(inotify_event)];

  std::vector<FileInfo> files;
  for (std::map<std::string, std::set<std::string> >::iterator it = file_domains_.begin(); it != file_domains_.end(); ++it) {
    FileInfo info;
    info.path = it->first;
    info.pos = 0;
    info.line_offset = 0;
    files.push_back(info);
  }

  // go through all files (don't publish while getting to the end..)
  for (size_t i = 0; i < files.size(); ++i) {
    files[i].wd = inotify_add_watch(fd, files[i].path.c_str(), (IN_MODIFY));
    files[i].f = fopen(files[i].path.c_str(), "r");
    if (!files[i].f) {
      LOG_ERROR("Could not open %s", files[i].path.c_str());
      exit(1);
    }

    UpdateFile(&files[i], false);
  }

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

    struct pollfd pfd = { fd, POLLIN, 0 };
    int ret = poll(&pfd, 1, 10);  // Sleep for up to 10ms
    if (ret < 0) {
      LOG_ERROR("poll failed: %s", strerror(errno));
      exit(1);
    } else if (ret != 0) {
      size_t len = read(fd, buf, sizeof(buf));

      for (size_t e = 0; e < len; ) {
        inotify_event *ev = reinterpret_cast<inotify_event*>(&buf[e]);

        int i = 0;
        while (files[i].wd != ev->wd) {
          ++i;
        }

        if (ev->mask & IN_MODIFY) {
          UpdateFile(&files[i], true);
        } else {
          close(files[i].wd);
          LOG_ERROR("Ending Publishing");
          return;
        }
        e += sizeof(inotify_event) + ev->len;
      }
    }
  }
}

void LogPublisher::UpdateFile(FileInfo* file_info, bool send_message) {
  fseek(file_info->f, file_info->pos, SEEK_SET);
  while (fgets(file_info->line + file_info->line_offset, MAX_LINE_LENGTH - file_info->line_offset, file_info->f)) {
    int length = strlen(file_info->line);
    if (file_info->line[length - 1] == '\n') {
      const std::set<std::string> domains = file_domains_[file_info->path];
      for (std::set<std::string>::const_iterator it = domains.begin(); it != domains.end(); ++it) {
        std::string domain = *it;
        std::string key;
        std::string value;
        GetKeyValue(domain, file_info->path, file_info->line, &key, &value);

        KVMessage message(key, topic_line_[key], value, false);
        ++topic_line_[key];

        // store latest
        std::string request_1 = "/" + domain + "/" + FileName(file_info->path) + "/1";
        shared_state_->StoreLatest(request_1, key, message);

        // zrisk smart update
        if (shared_state_->zrisk_domains_.count(domain)) {
          std::string request_zrisk = "/" + domain + "/" + FileName(file_info->path) + "/FASTZRISKLOG";
          shared_state_->zrisk_state_[request_zrisk].Update(&socket_, message);

          // Don't send message here intentionally, we are batching updates
        } else {
          if (send_message) {
            message.Send(&socket_);
          }
        }
      }
      file_info->line_offset = 0;
    } else {
      file_info->line_offset = length;
    }
  }
  file_info->pos = ftell(file_info->f);
}
