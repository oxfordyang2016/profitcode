#ifndef SRC_MARKET_HANDLERS_ZBATSDATA_MESSAGE_PROCESSOR_H_
#define SRC_MARKET_HANDLERS_ZBATSDATA_MESSAGE_PROCESSOR_H_

#include <stdint.h>
#include <tr1/unordered_map>

#include "common/depth_feed_converter.h"
#include "common/logging/topic_logger.h"
#include "market_handlers/zbatsdata/messages.h"

namespace bats {

class ISnapshotPublisher;
class LatencyLogger;
class SymbolFilter;

// Processes BATS messages, keeping MarketSnapshots updated
class MessageProcessor {
  typedef uint64_t OrderId;

  typedef exchanges::DepthFeedConverter<OrderId> Converter;
  typedef std::tr1::unordered_map<SymbolIndex, Converter*> Converters;
  typedef std::tr1::unordered_map<OrderId, Converter*> Orders;

 public:
  explicit MessageProcessor(ISnapshotPublisher* publisher,
                            const SymbolFilter & filter,
                            LatencyLogger* latency_logger,
                            TopicLogger* topic_logger);
  ~MessageProcessor();

  void Process(const Message* message);

  time_t GetBaseTime() const;
  void SetBaseTime(time_t base_time);

  void SetPublishEnabled(bool publish_enabled);
  void SetPublishOurTime(bool publish_our_time);

 private:
  template<class AddMessageType>
  Converter* ProcessAdd(const void* data);

  template<class ReduceMessageType>
  Converter* ProcessReduceSize(const void* data);

  template<class ModifiedMessageType>
  Converter* ProcessModified(const void* data);

  template<class TradeMessageType>
  Converter* ProcessTrade(const void* data);

  Converter* GetConverter(const Symbol & symbol);
  Converter* GetConverter(const LongSymbol & symbol);

  void Publish(Converter* converter, const timeval & our_time);

  time_t base_time_;
  timeval current_time_;

  Converters converters_;
  Orders orders_;
  ISnapshotPublisher* publisher_;
  bool publish_enabled_;
  bool publish_our_time_;
  const SymbolFilter & filter_;
  LatencyLogger* latency_logger_;
  TopicLogger* topic_logger_;
};
}

#endif  // SRC_MARKET_HANDLERS_ZBATSDATA_MESSAGE_PROCESSOR_H_
