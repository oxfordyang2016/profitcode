#ifndef SRC_MARKET_HANDLERS_ZBATSDATA_UNIT_MANAGER_H_
#define SRC_MARKET_HANDLERS_ZBATSDATA_UNIT_MANAGER_H_

#include <libconfig.h++>
#include <tr1/memory>
#include <string>
#include <vector>

#include "zshared/socket_listener.h"
#include "zshared/socket.h"
#include "zshared/timer.h"

#include "common/logging/topic_logger.h"

#include "market_handlers/zbatsdata/latency_logger.h"
#include "market_handlers/zbatsdata/symbol_filter.h"
#include "market_handlers/zbatsdata/tcp_server.h"
#include "market_handlers/zbatsdata/unit.h"
#include "market_handlers/zbatsdata/unit_processor.h"

namespace bats {

class UnitManager {
 public:
  UnitManager(const libconfig::Setting & settings,
              const libconfig::Setting & unit,
              const libconfig::Setting & spin,
              const SymbolFilter & filter);
  ~UnitManager();

  void Run();
  void RunListenerThread();

 private:
  void BindSocket(const std::string & address, int port);
  bool ReadNormalMessage(zshared::Socket *socket);

  Unit unit_;
  TcpServer spin_;

  std::tr1::shared_ptr<UnitProcessor> processor_;
  const SymbolFilter & filter_;
  std::tr1::shared_ptr<ISnapshotPublisher> publisher_;

  size_t max_cached_messages_;
  bool use_busy_wait_;

  zshared::SocketListener listener_;
  std::vector<zshared::UdpSocket*> normal_sockets_;
  std::string interface_;
  int receive_buffer_size_;

  zshared::Timer latency_timer_;
  std::tr1::shared_ptr<LatencyLogger> latency_logger_;

  std::tr1::shared_ptr<TopicLogger> topic_logger_;
};
}

#endif  // SRC_MARKET_HANDLERS_ZBATSDATA_UNIT_MANAGER_H_
