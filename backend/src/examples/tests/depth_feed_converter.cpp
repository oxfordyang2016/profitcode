#include <gtest/gtest.h>
#include <string>

#include "common/depth_feed_converter.h"

const std::string kTestSymbol = "TEST";
typedef exchanges::DepthFeedConverter<int> Converter;

void Verify(
    Converter &converter,
    const zshared::UnsignedDecimal &bid_price,
    int bid_volume,
    const zshared::UnsignedDecimal &ask_price,
    int ask_volume) {
  MarketSnapshot & snapshot = converter.GetSnapshot();
  int snapshot_bid_volume = 0, snapshot_ask_volume = 0;

  for (int i = 0; i < MARKET_DATA_DEPTH; i++) {
    snapshot_bid_volume += snapshot.bid_sizes[i];
    snapshot_ask_volume += snapshot.ask_sizes[i];
  }

  EXPECT_EQ(bid_volume, snapshot_bid_volume);
  EXPECT_EQ(ask_volume, snapshot_ask_volume);
  if (bid_volume != 0) {
    EXPECT_DOUBLE_EQ(bid_price.GetDoubleValue(), snapshot.bids[0]);
  }
  if (ask_volume != 0) {
    EXPECT_DOUBLE_EQ(ask_price.GetDoubleValue(), snapshot.asks[0]);
  }
}

TEST(DepthFeedConverter, AddRemoveBid) {
  Converter converter(kTestSymbol);
  zshared::UnsignedDecimal price(100, 0);

  converter.ProcessAddOrder(1, 10, price, true);
  EXPECT_EQ(10U, converter.GetOrderSize(1));
  Verify(converter, price, 10, price, 0);

  converter.ProcessAddOrder(2, 15, price, true);
  Verify(converter, price, 25, price, 0);

  converter.ProcessDeleteOrder(1);
  Verify(converter, price, 15, price, 0);

  converter.ProcessDeleteOrder(2);
  Verify(converter, price, 0, price, 0);
}

TEST(DepthFeedConverter, RemoveMiddle) {
  Converter converter(kTestSymbol);
  zshared::UnsignedDecimal price(100, 0);

  converter.ProcessAddOrder(1, 10, price, false);
  converter.ProcessAddOrder(2, 11, price, false);
  converter.ProcessAddOrder(3, 12, price, false);

  converter.ProcessDeleteOrder(2);
  Verify(converter, price, 0, price, 22);
}

TEST(DepthFeedConverter, RemoveAdd) {
  Converter converter(kTestSymbol);
  zshared::UnsignedDecimal price(100, 0);

  converter.ProcessAddOrder(1, 10, price, false);
  converter.ProcessAddOrder(2, 11, price, false);
  converter.ProcessAddOrder(3, 12, price, false);

  converter.ProcessDeleteOrder(3);
  Verify(converter, price, 0, price, 21);

  converter.ProcessAddOrder(4, 13, price, false);
  Verify(converter, price, 0, price, 34);
}

TEST(DepthFeedConverter, AddOrderTwice) {
  Converter converter(kTestSymbol);
  zshared::UnsignedDecimal price(100, 0);

  converter.ProcessAddOrder(1, 10, price, false);
  EXPECT_THROW(converter.ProcessAddOrder(1, 10, price, false), std::runtime_error);
}

TEST(DepthFeedConverter, ModifyOrder) {
  Converter converter(kTestSymbol);
  zshared::UnsignedDecimal price(100, 0);
  zshared::UnsignedDecimal new_price(150, 0);

  converter.ProcessAddOrder(1, 10, price, true);
  converter.ProcessAddOrder(2, 11, price, true);
  converter.ProcessModifyOrder(1, 5, price);
  Verify(converter, price, 16, price, 0);
  converter.ProcessModifyOrder(2, 11, new_price);
  Verify(converter, new_price, 16, price, 0);
  converter.ProcessDeleteOrder(2);
  Verify(converter, price, 5, price, 0);
  converter.ProcessModifyOrder(1, 15, new_price);
  Verify(converter, new_price, 15, price, 0);
}

TEST(DepthFeedConverter, ReduceOrder) {
  Converter converter(kTestSymbol);
  zshared::UnsignedDecimal price(100, 0);

  converter.ProcessAddOrder(1, 10, price, true);
  converter.ProcessReduceOrder(1, 5);
  Verify(converter, price, 5, price, 0);
  converter.ProcessReduceOrder(1, 5);
  Verify(converter, price, 0, price, 0);

  // Shouldn't be able to reduce an order below 0
  converter.ProcessAddOrder(2, 10, price, true);
  EXPECT_THROW(converter.ProcessReduceOrder(2, 15), std::runtime_error);
}

TEST(DepthFeedConverter, Trade) {
  Converter converter(kTestSymbol);
  zshared::UnsignedDecimal price(100, 0);

  converter.ProcessAddOrder(1, 10, price, true);
  converter.ProcessAddOrder(2, 12, price, false);
  Verify(converter, price, 10, price, 12);
  EXPECT_FALSE(converter.GetSnapshot().is_trade_update);

  converter.ProcessDeleteOrder(1);
  converter.ProcessReduceOrder(2, 10);
  Verify(converter, price, 0, price, 2);
  EXPECT_FALSE(converter.GetSnapshot().is_trade_update);

  converter.ProcessTrade(10, price);
  MarketSnapshot snapshot = converter.GetSnapshot();
  EXPECT_EQ(true, snapshot.is_trade_update);
  EXPECT_EQ(10, snapshot.last_trade_size);
  EXPECT_EQ(10, snapshot.volume);
  EXPECT_DOUBLE_EQ(price.GetDoubleValue(), snapshot.last_trade);

  converter.ProcessAddOrder(3, 2, price, true);
  converter.ProcessDeleteOrder(2);
  converter.ProcessDeleteOrder(3);
  Verify(converter, price, 0, price, 0);
  EXPECT_FALSE(converter.GetSnapshot().is_trade_update);

  converter.ProcessTrade(2, price);
  snapshot = converter.GetSnapshot();
  EXPECT_EQ(true, snapshot.is_trade_update);
  EXPECT_EQ(2, snapshot.last_trade_size);
  EXPECT_EQ(12, snapshot.volume);
  EXPECT_DOUBLE_EQ(price.GetDoubleValue(), snapshot.last_trade);
}

TEST(DepthFeedConverter, Time) {
  Converter converter(kTestSymbol);
  zshared::UnsignedDecimal price(100, 0);

  converter.GetSnapshot().time.tv_sec = 12345;
  converter.GetSnapshot().time.tv_usec = 54;

  converter.ProcessAddOrder(1, 15, price, true);
  Verify(converter, price, 15, price, 0);

  timeval time = converter.GetSnapshot().time;
  EXPECT_EQ(12345, time.tv_sec);
  EXPECT_EQ(54, time.tv_usec);
}

TEST(DepthFeedConverter, MultipleLevels) {
  Converter converter(kTestSymbol);
  zshared::UnsignedDecimal bid_price1(100, 1);
  zshared::UnsignedDecimal bid_price2(150, 1);
  zshared::UnsignedDecimal bid_price3(250, 1);
  zshared::UnsignedDecimal ask_price1(275, 1);
  zshared::UnsignedDecimal ask_price2(276, 1);
  zshared::UnsignedDecimal ask_price3(300, 1);

  converter.ProcessAddOrder(1, 10, bid_price1, true);
  converter.ProcessAddOrder(2, 11, bid_price2, true);
  converter.ProcessAddOrder(3, 12, bid_price3, true);

  converter.ProcessAddOrder(4, 13, ask_price1, false);
  converter.ProcessAddOrder(5, 14, ask_price2, false);
  converter.ProcessAddOrder(6, 15, ask_price3, false);

  Verify(converter, bid_price3, 33, ask_price1, 42);

  MarketSnapshot snapshot = converter.GetSnapshot();
  EXPECT_DOUBLE_EQ(bid_price2.GetDoubleValue(), snapshot.bids[1]);
  EXPECT_EQ(11, snapshot.bid_sizes[1]);
  EXPECT_DOUBLE_EQ(bid_price1.GetDoubleValue(), snapshot.bids[2]);
  EXPECT_EQ(10, snapshot.bid_sizes[2]);
  EXPECT_DOUBLE_EQ(ask_price2.GetDoubleValue(), snapshot.asks[1]);
  EXPECT_EQ(14, snapshot.ask_sizes[1]);
  EXPECT_DOUBLE_EQ(ask_price3.GetDoubleValue(), snapshot.asks[2]);
  EXPECT_EQ(15, snapshot.ask_sizes[2]);

  converter.ProcessDeleteOrder(2);
  converter.ProcessDeleteOrder(5);
  Verify(converter, bid_price3, 22, ask_price1, 28);

  snapshot = converter.GetSnapshot();
  EXPECT_DOUBLE_EQ(bid_price3.GetDoubleValue(), snapshot.bids[0]);
  EXPECT_EQ(12, snapshot.bid_sizes[0]);
  EXPECT_DOUBLE_EQ(bid_price1.GetDoubleValue(), snapshot.bids[1]);
  EXPECT_EQ(10, snapshot.bid_sizes[1]);
  EXPECT_DOUBLE_EQ(ask_price1.GetDoubleValue(), snapshot.asks[0]);
  EXPECT_EQ(13, snapshot.ask_sizes[0]);
  EXPECT_DOUBLE_EQ(ask_price3.GetDoubleValue(), snapshot.asks[1]);
  EXPECT_EQ(15, snapshot.ask_sizes[1]);
}
