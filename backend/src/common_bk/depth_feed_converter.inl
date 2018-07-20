#include <cassert>
#include <cstring>
#include <stdexcept>

namespace exchanges {
static const int kDepthFeedHashSize = 1000;

template<class OrderIdType>
DepthFeedConverter<OrderIdType>::DepthFeedConverter(const std::string &symbol)
  : orders_(kDepthFeedHashSize) {
  // Initialize snapshot
  strncpy(snapshot_.ticker, symbol.c_str(), MAX_TICKER_LENGTH);
  snapshot_.is_initialized = true;
}

template<class OrderIdType>
DepthFeedConverter<OrderIdType>::~DepthFeedConverter() {
  for (typename LevelMap::iterator it = bids_.begin(); it != bids_.end(); ++it) {
    delete it->second;
  }
  for (typename LevelMap::iterator it = asks_.begin(); it != asks_.end(); ++it) {
    delete it->second;
  }
  for (typename std::tr1::unordered_map<OrderIdType, Order*>::iterator it = orders_.begin();
       it != orders_.end();
       ++it) {
    delete it->second;
  }
}

template<class OrderIdType>
void DepthFeedConverter<OrderIdType>::ProcessAddOrder(
    OrderIdType order_id,
    uint32_t size,
    const zshared::UnsignedDecimal &price,
    bool is_bid) {
  Level *level = is_bid ? bids_[price.GetRawValue()] : asks_[price.GetRawValue()];
  if (0 == level) {
    level = new Level;
    level->price = price;
    level->volume = 0;
    level->is_bid = is_bid;
    level->head = level->tail = 0;
    if (is_bid) {
      bids_[price.GetRawValue()] = level;
    } else {
      asks_[price.GetRawValue()] = level;
    }
  }

  if (orders_.count(order_id)) {
    throw std::runtime_error("Order already exists!");
  }

  Order *order = new Order;
  order->size = size;
  order->level = level;
  order->next = order->prev = 0;

  orders_[order_id] = order;

  // Find the existing orders
  if (0 == level->head) {
    assert(level->tail == 0);
    assert(level->volume == 0);

    level->head = order;
    level->tail = order;
  } else {
    assert(level->tail->next == 0);

    level->tail->next = order;
    order->prev = level->tail;

    level->tail = order;
  }

  level->volume += size;

  UpdateSnapshot(is_bid);
}

template<class OrderIdType>
void DepthFeedConverter<OrderIdType>::ProcessDeleteOrder(OrderIdType order_id) {
  Order *order = orders_[order_id];
  if (0 == order) {
    fprintf(stderr, "CRIT: Reducing unknown order id %zu\n", static_cast<uint64_t>(order_id));
    throw std::runtime_error("Unable to find order");
  }

  orders_.erase(order_id);

  Level *level = order->level;
  bool is_bid = level->is_bid;

  if (order->prev) {
    order->prev->next = order->next;
  }
  if (order->next) {
    order->next->prev = order->prev;
  }

  if (order == level->head) {
    level->head = order->next;
  }
  if (order == level->tail) {
    level->tail = order->prev;
  }

  level->volume -= order->size;

  if (0 == level->head && 0 == level->tail) {
    assert(level->volume == 0);

    if (is_bid) {
      bids_.erase(level->price.GetRawValue());
    } else {
      asks_.erase(level->price.GetRawValue());
    }

    delete level;
  }

  delete order;

  UpdateSnapshot(is_bid);
}

template<class OrderIdType>
void DepthFeedConverter<OrderIdType>::ProcessModifyOrder(
    OrderIdType order_id,
    uint32_t size,
    const zshared::UnsignedDecimal &price) {
  Order *order = orders_[order_id];
  if (0 == order) {
    fprintf(stderr, "CRIT: Reducing unknown order id %zu\n", static_cast<uint64_t>(order_id));
    throw std::runtime_error("Unable to find order");
  }

  bool is_bid = order->level->is_bid;
  ProcessDeleteOrder(order_id);
  if (size != 0) {
    ProcessAddOrder(order_id, size, price, is_bid);
  }
}

template<class OrderIdType>
void DepthFeedConverter<OrderIdType>::ProcessModifyOrAddOrder(
    OrderIdType order_id,
    uint32_t size,
    const zshared::UnsignedDecimal &price,
    bool is_bid) {
/*
  Order *order = orders_[order_id];
  if (0 == order) {
    ProcessAddOrder(order_id, size, price, is_bid);
    return;
  }
*/
  if (orders_.count(order_id) == 0) {
    ProcessAddOrder(order_id, size, price, is_bid);
    return;
  }
  Order *order = orders_[order_id];
  if (0 == order) {
    throw std::runtime_error("ModifyOrAddOrder error, couldn't find order_id");
  }

  ProcessDeleteOrder(order_id);
  if (size != 0) {
    ProcessAddOrder(order_id, size, price, order->level->is_bid);
  }
}


template<class OrderIdType>
void DepthFeedConverter<OrderIdType>::ProcessReduceOrder(
    OrderIdType order_id,
    uint32_t size) {
  Order *order = orders_[order_id];
  if (0 == order) {
    fprintf(stderr, "CRIT: Reducing unknown order id %zu\n", static_cast<uint64_t>(order_id));
    throw std::runtime_error("Unable to find order");
  }

  bool is_bid = order->level->is_bid;
  int64_t reduced_size = static_cast<int64_t>(order->size) - size;
  if (reduced_size < 0) {
    throw std::runtime_error("Attempt to reduce order size below zero");
  }
  zshared::UnsignedDecimal price = order->level->price;

  ProcessDeleteOrder(order_id);
  if (reduced_size != 0) {
    ProcessAddOrder(order_id, reduced_size, price, is_bid);
  }
}

template<class OrderIdType>
void DepthFeedConverter<OrderIdType>::ProcessTrade(
    uint32_t size,
    const zshared::UnsignedDecimal &price) {
  snapshot_.last_trade = price.GetDoubleValue();
  snapshot_.last_trade_size = size;
  snapshot_.volume += size;
  snapshot_.is_trade_update = true;
}

template<class OrderIdType>
void DepthFeedConverter<OrderIdType>::ProcessTrade(
    uint32_t size,
    double price) {
  snapshot_.last_trade = price;
  snapshot_.last_trade_size = size;
  snapshot_.volume += size;
  snapshot_.is_trade_update = true;
}

template<class OrderIdType>
uint32_t DepthFeedConverter<OrderIdType>::GetOrderSize(OrderIdType order_id) {
  Order *order = orders_[order_id];
  if (0 == order) {
    throw std::runtime_error("Order does not exist");
  }

  return order->size;
}

template<class OrderIdType>
zshared::UnsignedDecimal DepthFeedConverter<OrderIdType>::GetOrderPrice(OrderIdType order_id) {
  Order *order = orders_[order_id];
  if (0 == order) {
    throw std::runtime_error("Order does not exist");
  }

  return order->level->price;
}

template<class OrderIdType>
void DepthFeedConverter<OrderIdType>::UpdateSnapshot(bool is_bid) {
  snapshot_.is_trade_update = false;

  if (is_bid) {
    int i = 0;
    for (typename LevelMap::reverse_iterator it = bids_.rbegin(); it != bids_.rend() && i < MARKET_DATA_DEPTH; ++it, ++i) {
      snapshot_.bids[i] = it->second->price.GetDoubleValue();
      snapshot_.bid_sizes[i] = it->second->volume;
    }
    for (; i < MARKET_DATA_DEPTH; i++) {
      snapshot_.bids[i] = 0.0;
      snapshot_.bid_sizes[i] = 0; 
    }
  } else {
    int i = 0;
    for (typename LevelMap::iterator it = asks_.begin(); it != asks_.end() && i < MARKET_DATA_DEPTH; ++it, ++i) {
      snapshot_.asks[i] = it->second->price.GetDoubleValue();
      snapshot_.ask_sizes[i] = it->second->volume;
    }
    for (; i < MARKET_DATA_DEPTH; i++) {
      snapshot_.asks[i] = 0.0;
      snapshot_.ask_sizes[i] = 0;
    }
  }   
}

}
