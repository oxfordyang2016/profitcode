#include "market_handlers/zbatsdata/unit_manager.h"

#include <errno.h>
#include <libconfig.h++>
#include <stdlib.h>
#include <string>
#include <vector>

#include "zshared/socket.h"
#include "common/zentero_util.h"
#include "market_handlers/zbatsdata/snapshot_publisher.h"
#include "market_handlers/zbatsdata/symbol_filter.h"

namespace bats {

UnitManager::UnitManager(const libconfig::Setting & settings,
                         const libconfig::Setting & unit,
                         const libconfig::Setting & spin,
                         const SymbolFilter & filter)
  : unit_(static_cast<int>(unit["unit"])),
    spin_(spin["address"],
          spin["port"],
          spin["session_sub_id"],
          spin["username"],
          spin["password"]),
    filter_(filter) {
  // Initialize latency log
  char latency_filename[250];
  snprintf(latency_filename, sizeof(latency_filename),
    settings["latency_log"].c_str(), unit_.GetId());
  latency_logger_.reset(new LatencyLogger(latency_filename));

  // Initialize topic log
  char topic_log_filename[250];
  snprintf(topic_log_filename, sizeof(topic_log_filename),
    settings["log"].c_str(), unit_.GetId());
  topic_logger_.reset(new TopicLogger(topic_log_filename));

  // Shared settings
  interface_ = static_cast<const char*>(settings["interface"]);
  max_cached_messages_ = static_cast<int>(settings["max_cached_messages"]);
  receive_buffer_size_ = settings["receive_buffer_size"];
  use_busy_wait_ = settings["use_busy_wait"];

  // Bind all sockets
  libconfig::Setting & feeds = unit["feeds"];
  for (int j = 0; j < feeds.getLength(); j++) {
    BindSocket(feeds[j]["address"], feeds[j]["port"]);
  }

  // Initialize publisher
  std::string topic = settings["topic"].c_str();
  std::string prefix = settings["prefix"].c_str();

  if (topic == "stdout") {
    publisher_.reset(new FileSnapshotPublisher(stdout, prefix));
  } else {
    char publisher_filename[250];
    snprintf(publisher_filename, sizeof(publisher_filename),
      topic.c_str(), unit_.GetId());
    topic_logger_->Log("INFO", "Connecting to %s", publisher_filename);
    publisher_.reset(new ShqSnapshotPublisher(publisher_filename, prefix));
  }

  processor_.reset(
    new UnitProcessor(
        unit_,
        spin_,
        publisher_.get(),
        filter_,
        latency_logger_.get(),
        max_cached_messages_,
        topic_logger_.get()));
}

UnitManager::~UnitManager() {
  for (size_t i = 0; i < normal_sockets_.size(); ++i) {
    delete normal_sockets_[i];
  }
}

void UnitManager::BindSocket(const std::string & address, int port) {
  zshared::UdpSocket* socket = new zshared::UdpSocket();
  normal_sockets_.push_back(socket);

  socket->SetReuseAddr();
  socket->SetNonBlocking();
  socket->SetReceiveBufferSize(receive_buffer_size_);
  socket->Bind(zshared::kSocketAnyAddress, port);
  socket->ListenMulticast(address, ConvertInterfaceToInetAddress(interface_.c_str()));

  listener_.AddDescriptor(
      socket,
      std::bind1st(std::tr1::mem_fn(&UnitManager::ReadNormalMessage), this));
}

static void* RunUnitThread(void* arg) {
  UnitManager* manager = static_cast<UnitManager*>(arg);
  manager->RunListenerThread();
  return 0;
}

void UnitManager::Run() {
  pthread_t unit_thread;
  if (pthread_create(&unit_thread,
                     0,
                     RunUnitThread,
                     this) != 0) {
    LOG_ERROR("Unable to create thread!");
    exit(1);
  }
}

void UnitManager::RunListenerThread() {
  latency_timer_.Reset();

  while (true) {
    if (use_busy_wait_) {
      int count = 0;
      // Process up to 20 messages on the highest priority socket
      // before checking other sockets.
      while (++count < 20 &&
             ReadNormalMessage(normal_sockets_[0]));

      for (size_t i = 1; i < normal_sockets_.size(); ++i) {
        ReadNormalMessage(normal_sockets_[i]);
      }
    } else {
      // Block on socket
      listener_.Process(-1);
    }

    if (latency_timer_.ElapsedSeconds() > 2.0) {
      latency_timer_.Reset();
      latency_logger_->LogMarketDiff(latency_timer_.GetTimeval());
    }
  }
}

bool UnitManager::ReadNormalMessage(zshared::Socket *raw_socket) {
  zshared::UdpSocket *socket = static_cast<zshared::UdpSocket*>(raw_socket);

  char bytes[2048];
  ssize_t bytes_read = socket->Receive(bytes, sizeof(bytes), 0);
  if (bytes_read < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      // Socket read would block, so skip this read for now
      return false;
    }

    throw std::runtime_error("Socket recv error:" + std::string(strerror(errno)));
  }

  const bats::SequencedUnit *container = reinterpret_cast<const bats::SequencedUnit*>(bytes);
  if (container->header.length != bytes_read) {
    topic_logger_->Log(
        "ERROR", "Read message with invalid length (Ex:%d, Ac:%d)",
        container->header.length,
        bytes_read);
    return false;
  }

  if (container->header.unit != unit_.GetId()) {
    topic_logger_->Log(
        "ERROR", "Unknown unit id: %d", container->header.unit);
    return false;
  }
  processor_->Process(container);

  return true;
}
}
