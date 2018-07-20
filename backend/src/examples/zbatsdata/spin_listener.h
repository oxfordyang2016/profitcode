#ifndef SRC_MARKET_HANDLERS_ZBATSDATA_SPIN_LISTENER_H_
#define SRC_MARKET_HANDLERS_ZBATSDATA_SPIN_LISTENER_H_

#include <pthread.h>
#include <stdint.h>
#include <tr1/functional>
#include <tr1/memory>

#include "common/logging/topic_logger.h"
#include "market_handlers/zbatsdata/messages.h"

namespace zshared {
class TcpSocket;
}

namespace bats {

class MessageProcessor;
class Unit;
class TcpServer;

class SpinListener {
 public:
  // returns last sequence number processed
  // so you should start processing at the returned value + 1
  static uint32_t RebuildBook(const Unit &unit,
                              const TcpServer & spin_server,
                              uint32_t start_sequence_no,
                              MessageProcessor* processor,
                              TopicLogger* topic_logger);
};
}

#endif  // SRC_MARKET_HANDLERS_ZBATSDATA_SPIN_LISTENER_H_
