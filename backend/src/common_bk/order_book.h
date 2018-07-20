#ifndef SRC_COMMON_ORDER_BOOK_H_
#define SRC_COMMON_ORDER_BOOK_H_

#include <map>

#include "common/market_snapshot.h"

class OrderBook {
 public:
  explicit OrderBook(const char* ticker);

  inline size_t NumBids() const { return bids_.size(); }
  inline size_t NumAsks() const { return asks_.size(); }

  inline int volume() const {
    return snapshot_.volume;
  }

  inline void SetLastTrade(double last_trade) {
    snapshot_.last_trade = last_trade;
  }

  inline void SetLastTradeSize(int last_trade_size) {
    snapshot_.last_trade_size = last_trade_size;
  }

  inline void SetVolume(int volume) {
    snapshot_.volume = volume;
  }

  inline void SetIsTradeUpdate(bool is_trade_update) {
    snapshot_.is_trade_update = is_trade_update;
  }

  inline void SetTime(const timeval & time) {
    snapshot_.time = time;
  }

  inline void SetBid(double price, int size) {
    bids_[price] = size;
    bids_changed_ = true;
  }

  inline void SetAsk(double price, int size) {
    asks_[price] = size;
    asks_changed_ = true;
  }

  inline void SetImpliedBid(double price, int size) {
    implied_bids_[price] = size;
    bids_changed_ = true;  // same flag
  }

  inline void SetImpliedAsk(double price, int size) {
    implied_asks_[price] = size;
    asks_changed_ = true;  // same flag
  }

  inline void RemoveBid(double price) {
    bids_.erase(price);
    bids_changed_ = true;
  }

  inline void RemoveAsk(double price) {
    asks_.erase(price);
    asks_changed_ = true;
  }

  inline void RemoveImpliedBid(double price) {
    implied_bids_.erase(price);
    bids_changed_ = true;
  }

  inline void RemoveImpliedAsk(double price) {
    implied_asks_.erase(price);
    asks_changed_ = true;
  }

  const MarketSnapshot & GetMarketSnapshot();

 private:
  std::map<double, int> bids_;
  std::map<double, int> asks_;

  std::map<double, int> implied_bids_;
  std::map<double, int> implied_asks_;

  MarketSnapshot snapshot_;

  bool bids_changed_;
  bool asks_changed_;
};

#endif  // SRC_COMMON_ORDER_BOOK_H_
