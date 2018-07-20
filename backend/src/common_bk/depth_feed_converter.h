#ifndef SRC_COMMON_DEPTH_FEED_CONVERTER_H_
#define SRC_COMMON_DEPTH_FEED_CONVERTER_H_

#include <stdint.h>
#include <tr1/unordered_map>

#include <map>
#include <string>

#include "common/market_snapshot.h"
#include "zshared/unsigned_decimal.h"

namespace exchanges {

template<class OrderIdType>
class DepthFeedConverter {
 public:
  explicit DepthFeedConverter(const std::string & symbol);
  ~DepthFeedConverter();

  void ProcessAddOrder(OrderIdType order_id,
                       uint32_t size,
                       const zshared::UnsignedDecimal & price,
                       bool is_bid);
  void ProcessModifyOrAddOrder(OrderIdType order_id,
                               uint32_t size,
                               const zshared::UnsignedDecimal & price,
                               bool is_bid);

  void ProcessDeleteOrder(OrderIdType order_id);

  void ProcessModifyOrder(OrderIdType order_id,
                          uint32_t size,
                          const zshared::UnsignedDecimal & price);

  void ProcessReduceOrder(OrderIdType order_id,
                          uint32_t size);

  void ProcessTrade(uint32_t size,
                    const zshared::UnsignedDecimal & price);
  void ProcessTrade(uint32_t size,
                    double price);

  uint32_t GetOrderSize(OrderIdType order_id);
  zshared::UnsignedDecimal GetOrderPrice(OrderIdType order_id);

  inline MarketSnapshot & GetSnapshot() {
    return snapshot_;
  }

  inline const MarketSnapshot & GetPreviousSnapshot() const {
    return prev_snapshot_;
  }

  inline void SaveCurrentSnapshotToPrevious() {
    prev_snapshot_ = snapshot_;
  }

 private:
  void UpdateSnapshot(bool is_bid);

  MarketSnapshot snapshot_;
  MarketSnapshot prev_snapshot_;
  std::string symbol_;

  // Internal tracking
  struct Level;

  struct Order {
    uint32_t size;

    Order *next;
    Order *prev;

    Level *level;
  };

  struct Level {
    uint64_t volume;
    zshared::UnsignedDecimal price;
    bool is_bid;

    Order *head;
    Order *tail;
  };

  std::tr1::unordered_map<OrderIdType, Order*> orders_;

  typedef std::map<uint64_t, Level*> LevelMap;
  LevelMap bids_;
  LevelMap asks_;
};
}

#include "common/depth_feed_converter.inl"

#endif  // SRC_COMMON_DEPTH_FEED_CONVERTER_H_
