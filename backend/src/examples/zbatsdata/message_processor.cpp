#include "market_handlers/zbatsdata/message_processor.h"

#include <math.h>
#include <ctime>

#include "market_handlers/zbatsdata/latency_logger.h"
#include "market_handlers/zbatsdata/snapshot_publisher.h"
#include "market_handlers/zbatsdata/symbol_filter.h"
#include "zshared/timer.h"

static const int kOrderTableReserveSize = 4000000;
static const int kSymolLookupReserveSize = 30000;

namespace bats {

MessageProcessor::MessageProcessor(
    ISnapshotPublisher* publisher,
    const SymbolFilter & filter,
    LatencyLogger* latency_logger,
    TopicLogger* topic_logger)
    : converters_(kSymolLookupReserveSize),
      orders_(kOrderTableReserveSize),
      publisher_(publisher),
      publish_enabled_(true),
      publish_our_time_(true),
      filter_(filter),
      latency_logger_(latency_logger),
      topic_logger_(topic_logger) {
  // Time messages are offset from midnight EST.  We want them in Unix UTC standard.
  time_t now;
  time(&now);
  time_t midnight = now / 86400 * 86400;

  // Use Midnight UTC instead of Midnight EST (as specified by BATS), so we
  // don't have to handle timezone changes.  We don't publish the time
  // anyway
  SetBaseTime(midnight);
}

MessageProcessor::~MessageProcessor() {
  for (Converters::iterator it = converters_.begin(); it != converters_.end(); ++it) {
    delete it->second;
  }
}

time_t MessageProcessor::GetBaseTime() const {
  return base_time_;
}

void MessageProcessor::SetBaseTime(time_t base_time) {
  base_time_ = base_time;
  current_time_.tv_sec = base_time_;
  current_time_.tv_usec = 0;
}

void MessageProcessor::SetPublishEnabled(bool publish_enabled) {
  publish_enabled_ = publish_enabled;
}

void MessageProcessor::SetPublishOurTime(bool publish_our_time) {
  publish_our_time_ = publish_our_time;
}

// Check if the trade is on BATS.  Execution id first digit of '0' means BATS local.
// '0' in base 36 means it has to be < 36^8
const uint64_t kPow36_8 = 36ULL * 36 * 36 * 36 * 36 * 36 * 36 * 36;
inline static bool IsTradeLocal(uint64_t execution_id) {
  return execution_id < kPow36_8;
}

void MessageProcessor::Process(const Message* raw_message) {
  const MessageHeader & header = raw_message->header;
  const void* data = raw_message;
  Converter* converter = 0;

  timeval our_time;
  gettimeofday(&our_time, NULL);

  switch (header.message_type) {
    case kMessageTypeTime:
      {
        const TimeMessage* message = static_cast<const TimeMessage*>(data);
        current_time_.tv_sec = base_time_ + message->time;
      }
      break;

    case kMessageTypeAddOrderLong:
      converter = ProcessAdd<AddOrderLongMessage>(data);
      break;

    case kMessageTypeAddOrderShort:
      converter = ProcessAdd<AddOrderShortMessage>(data);
      break;

    case kMessageTypeAddOrderExpanded:
      converter = ProcessAdd<AddOrderExpandedMessage>(data);
      break;

    case kMessageTypeOrderExecuted:
      {
        const OrderExecutedMessage* message = static_cast<const OrderExecutedMessage*>(data);
        Converters::iterator it = orders_.find(message->order_id);
        if (it != orders_.end() && it->second != NULL) {
          converter = it->second;
          current_time_.tv_usec = message->ns/1000;

          if (IsTradeLocal(message->execution_id)) {
            // Publish trade message before market update
            converter->ProcessTrade(message->size, converter->GetOrderPrice(message->order_id));
            Publish(converter, our_time);
          }

          converter->ProcessReduceOrder(message->order_id, message->size);
        }
      }
      break;

    case kMessageTypeOrderExecutedAtPrice:
      {
        const OrderExecutedAtPriceMessage* message =
            static_cast<const OrderExecutedAtPriceMessage*>(data);
        Converters::iterator it = orders_.find(message->order_id);
        if (it != orders_.end() && it->second != NULL) {
          converter = it->second;
          current_time_.tv_usec = message->ns/1000;

          if (IsTradeLocal(message->execution_id)) {
            // Publish trade message before market update
            converter->ProcessTrade(message->size, message->price.GetUnsignedDecimal());
            Publish(converter, our_time);
          }

          uint32_t existing_size = converter->GetOrderSize(message->order_id);
          if (existing_size != message->remaining_size + message->size) {
            // Special case - Order moves to the back of the level
            converter->ProcessModifyOrder(
                message->order_id,
                message->remaining_size,
                message->price.GetUnsignedDecimal());
          } else {
            converter->ProcessReduceOrder(
                message->order_id,
                message->size);
          }
        }
      }
      break;

    case kMessageTypeOrderReduceSizeLong:
      converter = ProcessReduceSize<OrderReduceSizeLongMessage>(data);
      break;

    case kMessageTypeOrderReduceSizeShort:
      converter = ProcessReduceSize<OrderReduceSizeShortMessage>(data);
      break;

    case kMessageTypeOrderModifiedLong:
      converter = ProcessModified<OrderModifiedLongMessage>(data);
      break;

    case kMessageTypeOrderModifiedShort:
      converter = ProcessModified<OrderModifiedShortMessage>(data);
      break;

    case kMessageTypeDeleteOrder:
      {
        const DeleteOrderMessage *message = static_cast<const DeleteOrderMessage*>(data);
        Converters::iterator it = orders_.find(message->order_id);
        if (it != orders_.end() && it->second != NULL) {
          converter = it->second;
          current_time_.tv_usec = message->ns/1000;
          converter->ProcessDeleteOrder(message->order_id);
          // erasing by iterator seems to have terrible benchmarking performance?!
          // so erase by value instead
          orders_.erase(message->order_id);
        }
      }
      break;

    case kMessageTypeTradeLong:
      converter = ProcessTrade<TradeLongMessage>(data);
      break;

    case kMessageTypeTradeShort:
      converter = ProcessTrade<TradeShortMessage>(data);
      break;

    case kMessageTypeTradeExpanded:
      converter = ProcessTrade<TradeExpandedMessage>(data);
      break;

    case kMessageTypeTradeBreak:
      break;

    case kMessageTypeEndOfSession:
      break;

    case kMessageTypeSymbolMapping:
      break;

    case kMessageTypeTradingStatus:
      break;

    case kMessageTypeUnitClear:
      {
        // Clear all the state for this unit
        topic_logger_->Log("ERROR", "Received unit clear message!");
        converters_.clear();
        orders_.clear();
      }
      break;
  }

  if (converter) {
    Publish(converter, our_time);
  }
}

// check if snapshot changed by looking at:
// -sizes
// -volume
// -prices
// note, this is BATS specific (i.e. trades always increase volume
// so we don't need to check those fields, etc.)
static bool BatsSnapshotChanged(const MarketSnapshot & prev_snapshot,
                                const MarketSnapshot & snapshot) {
  if (prev_snapshot.volume != snapshot.volume) {
    return true;
  }
  // check sizes first since faster (and more common)
  for (int i = 0; i < MARKET_DATA_DEPTH; ++i) {
    if (prev_snapshot.bid_sizes[i] != snapshot.bid_sizes[i]) {
      return true;
    }
    if (prev_snapshot.ask_sizes[i] != snapshot.ask_sizes[i]) {
      return true;
    }
  }
  for (int i = 0; i < MARKET_DATA_DEPTH; ++i) {
    if (fabs(prev_snapshot.bids[i] - snapshot.bids[i]) > EPS) {
      return true;
    }
    if (fabs(prev_snapshot.asks[i] - snapshot.asks[i]) > EPS) {
      return true;
    }
  }
  return false;
}

void MessageProcessor::Publish(Converter* converter, const timeval & our_time) {
  if (!publish_enabled_) {
    return;
  }

  MarketSnapshot & snapshot = converter->GetSnapshot();
  const MarketSnapshot & prev_snapshot = converter->GetPreviousSnapshot();

  if (!BatsSnapshotChanged(prev_snapshot, snapshot)) {
    return;
  }

  if (publish_our_time_) {
    snapshot.time = our_time;
  } else {
    snapshot.time = current_time_;
  }
  publisher_->Publish(snapshot);
  converter->SaveCurrentSnapshotToPrevious();

  // Record the difference between when we received the message, and
  // when the exchange transmitted.
  latency_logger_->RecordMarketDiff(
      zshared::Timer(our_time).Microseconds() -
      zshared::Timer(current_time_).Microseconds());
}

template<class AddMessageType>
MessageProcessor::Converter* MessageProcessor::ProcessAdd(const void* data) {
  const AddMessageType* message = static_cast<const AddMessageType*>(data);

  bool is_bid = false;
  if (message->side == 'B')
    is_bid = true;
  else if (message->side == 'S')
    is_bid = false;
  else
    throw std::runtime_error("Unsupported bid/ask type in ADD!");

  Converter* converter = GetConverter(message->symbol);
  if (converter) {
    current_time_.tv_usec = message->ns/1000;
    converter->ProcessAddOrder(message->order_id,
                               message->size,
                               message->price.GetUnsignedDecimal(),
                               is_bid);

    orders_[message->order_id] = converter;
  }

  return converter;
}

template<class ReduceMessageType>
MessageProcessor::Converter* MessageProcessor::ProcessReduceSize(const void* data) {
  const ReduceMessageType* message = static_cast<const ReduceMessageType*>(data);

  Converters::iterator it = orders_.find(message->order_id);
  if (it == orders_.end() || it->second == NULL) {
    return NULL;
  }
  Converter* converter = it->second;
  current_time_.tv_usec = message->ns/1000;
  converter->ProcessReduceOrder(message->order_id, message->size);

  return converter;
}

template<class ModifiedMessageType>
MessageProcessor::Converter* MessageProcessor::ProcessModified(const void* data) {
  const ModifiedMessageType* message = static_cast<const ModifiedMessageType*>(data);

  Converters::iterator it = orders_.find(message->order_id);
  if (it == orders_.end() || it->second == NULL) {
    return NULL;
  }
  Converter* converter = it->second;
  current_time_.tv_usec = message->ns/1000;
  converter->ProcessModifyOrder(message->order_id,
                                message->size,
                                message->price.GetUnsignedDecimal());

  return converter;
}

template<class TradeMessageType>
MessageProcessor::Converter* MessageProcessor::ProcessTrade(const void* data) {
  const TradeMessageType *message = static_cast<const TradeMessageType*>(data);

  Converter* converter = GetConverter(message->symbol);
  if (converter && IsTradeLocal(message->execution_id)) {
    current_time_.tv_usec = message->ns/1000;
    converter->ProcessTrade(message->size, message->price.GetUnsignedDecimal());
  }

  return converter;
}

MessageProcessor::Converter* MessageProcessor::GetConverter(const Symbol & symbol) {
  const uint64_t symbol_index = symbol.GetAsIndex();
  Converters::const_iterator it = converters_.find(symbol_index);
  // a match (non-valid symbols stored as NULL for speed)
  if (it != converters_.end()) {
    return it->second;
  }

  if (filter_.IsValid(symbol)) {
    char ticker[64];
    snprintf(ticker, sizeof(ticker), "%s/%s/",
             publisher_->GetPrefix().c_str(), symbol.GetAsString().c_str());
    Converter* converter = new Converter(ticker);
    converters_[symbol_index] = converter;
    return converter;
  }

  // no match
  converters_[symbol_index] = NULL;
  return NULL;
}

MessageProcessor::Converter* MessageProcessor::GetConverter(const LongSymbol & symbol) {
  const uint64_t symbol_index = symbol.GetAsIndex();
  Converters::const_iterator it = converters_.find(symbol_index);
  // a match (non-valid symbols stored as NULL for speed)
  if (it != converters_.end()) {
    return it->second;
  }

  if (filter_.IsValid(symbol)) {
    char ticker[64];
    snprintf(ticker, sizeof(ticker), "%s/%s/",
             publisher_->GetPrefix().c_str(), symbol.GetAsString().c_str());
    Converter* converter = new Converter(ticker);
    converters_[symbol_index] = converter;
    return converter;
  }

  // no match
  converters_[symbol_index] = NULL;
  return NULL;
}

}  // namespace bats
