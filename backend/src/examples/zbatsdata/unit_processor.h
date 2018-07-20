#ifndef SRC_MARKET_HANDLERS_ZBATSDATA_UNIT_PROCESSOR_H_
#define SRC_MARKET_HANDLERS_ZBATSDATA_UNIT_PROCESSOR_H_

#include <tr1/memory>
#include <queue>
#include <vector>

#include "zshared/lock.h"

#include "common/logging/topic_logger.h"

#include "market_handlers/zbatsdata/latency_logger.h"
#include "market_handlers/zbatsdata/message_processor.h"
#include "market_handlers/zbatsdata/tcp_server.h"
#include "market_handlers/zbatsdata/unit.h"

namespace bats {

class UnitProcessor {
 public:
  explicit UnitProcessor(
      const Unit & unit,
      const TcpServer & spin_server,
      ISnapshotPublisher* publisher,
      const SymbolFilter & filter,
      LatencyLogger* latency_logger,
      size_t max_cached_messages,
      TopicLogger* topic_logger);

  void StartSpin();
  void Process(const SequencedUnit* container);

 private:
  void Restart(uint32_t sequence_no);

  bool ProcessMessages(const SequencedUnit* container);
  void CheckCachedMessages();

  // Definitions
  typedef const char* SeqUnitPtr;
  struct Compare {
    bool operator()(const SeqUnitPtr & s1, const SeqUnitPtr & s2) const {
      const bats::SequencedUnit* p1 = reinterpret_cast<const bats::SequencedUnit*>(s1);
      const bats::SequencedUnit* p2 = reinterpret_cast<const bats::SequencedUnit*>(s2);
      // priority_queue stores items from greatest to smallest, and we want smallest first.
      return p1->header.sequence_no > p2->header.sequence_no;
    }
  };

  // Constants
  const Unit & unit_;
  const TcpServer & spin_server_;

  // State
  uint32_t sequence_no_;
  std::tr1::shared_ptr<MessageProcessor> processor_;

  ISnapshotPublisher* processor_publisher_;
  const SymbolFilter & processor_filter_;
  LatencyLogger* latency_logger_;

  size_t max_cached_messages_;
  std::priority_queue<SeqUnitPtr, std::vector<SeqUnitPtr>, Compare> cached_containers_;

  bool needs_restart_;

  TopicLogger* topic_logger_;
};

}  // namespace bats

#endif  // SRC_MARKET_HANDLERS_ZBATSDATA_UNIT_PROCESSOR_H_
