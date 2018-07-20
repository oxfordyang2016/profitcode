#include <gtest/gtest.h>
#include <tr1/memory>
#include <string>

#include "market_handlers/zbatsdata/latency_logger.h"
#include "market_handlers/zbatsdata/message_processor.h"
#include "market_handlers/zbatsdata/snapshot_publisher.h"
#include "market_handlers/zbatsdata/symbol_filter.h"

#include "../tests/zbatsdata/test_publisher.h"

const std::string kSymbolString = "$_TE";
const std::string kExpectedTicker = "BATS_TEST/$_TE/";
const bats::Symbol kSymbol = bats::CreateSymbol(kSymbolString);
const bats::LongSymbol kLongSymbol = bats::CreateLongSymbol(kSymbolString);

const uint16_t kPrice = 1234;
const uint64_t kLongPrice = 123400;
const double kDoublePrice = 12.34;

const int kOrderSize = 99;

class MessageProcessor : public ::testing::Test {
 public:
  void SetUp() {
    latency_logger_.reset(new bats::LatencyLogger("test"));
    topic_logger_.reset(new TopicLogger("/dev/null"));
  }

  bats::SymbolFilter filter_;
  std::tr1::shared_ptr<bats::LatencyLogger> latency_logger_;
  std::tr1::shared_ptr<TopicLogger> topic_logger_;
};

template<class AddOrderType, class Symbol, class Price>
void TestAddOrder(
    bats::MessageProcessor *processor,
    TestPublisher *publisher,
    bats::kMessageTypes message_type,
    Symbol symbol,
    Price price) {
  AddOrderType message;
  message.header.length = sizeof(AddOrderType);
  message.header.message_type = message_type;

  message.ns = 1000;
  message.order_id = 1;
  message.side = 'B';
  message.size = kOrderSize;
  message.symbol = symbol;
  message.price.price = price;
  message.flags = 0;

  processor->Process(reinterpret_cast<bats::Message*>(&message));

  ASSERT_EQ(1, publisher->GetPublishedCount());
  MarketSnapshot snapshot = publisher->GetSnapshot();
  EXPECT_EQ(kExpectedTicker, snapshot.ticker);
  EXPECT_DOUBLE_EQ(kDoublePrice, snapshot.bids[0]);
  EXPECT_EQ(kOrderSize, snapshot.bid_sizes[0]);
  EXPECT_EQ(0, snapshot.ask_sizes[0]);
  EXPECT_FALSE(snapshot.is_trade_update);

  message.order_id = 2;
  message.side = 'S';
  processor->Process(reinterpret_cast<bats::Message*>(&message));
  snapshot = publisher->GetSnapshot();
  EXPECT_DOUBLE_EQ(kDoublePrice, snapshot.asks[0]);
  EXPECT_EQ(kOrderSize, snapshot.bid_sizes[0]);
  EXPECT_EQ(kOrderSize, snapshot.ask_sizes[0]);
  EXPECT_FALSE(snapshot.is_trade_update);

  message.side = ' ';
  EXPECT_THROW(processor->Process(reinterpret_cast<bats::Message*>(&message)), std::runtime_error);
}

TEST_F(MessageProcessor, AddOrderLong) {
  TestPublisher publisher;
  bats::MessageProcessor processor(&publisher, filter_, latency_logger_.get(), topic_logger_.get());

  TestAddOrder<bats::AddOrderLongMessage>(
    &processor,
    &publisher,
    bats::kMessageTypeAddOrderLong,
    kSymbol,
    kLongPrice);
}

TEST_F(MessageProcessor, AddOrderShort) {
  TestPublisher publisher;
  bats::MessageProcessor processor(&publisher, filter_, latency_logger_.get(), topic_logger_.get());

  TestAddOrder<bats::AddOrderShortMessage>(
    &processor,
    &publisher,
    bats::kMessageTypeAddOrderShort,
    kSymbol,
    kPrice);
}

TEST_F(MessageProcessor, AddOrderExpandedMessage) {
  TestPublisher publisher;
  bats::MessageProcessor processor(&publisher, filter_, latency_logger_.get(), topic_logger_.get());

  TestAddOrder<bats::AddOrderExpandedMessage>(
    &processor,
    &publisher,
    bats::kMessageTypeAddOrderExpanded,
    kLongSymbol,
    kLongPrice);
}

uint64_t MakeExecutionId(const std::string & s) {
  if (s.size() != 9) {
    throw std::runtime_error("Badly formatted execution id " + s);
  }
  uint64_t result = 0;
  for (size_t i = 0; i < s.size(); ++i) {
    result *= 36;
    if (isdigit(s[i])) {
      result += s[i] - '0';
    } else {
      result += (s[i] - 'A') + 10;
    }
  }
  return result;
}

TEST_F(MessageProcessor, OrderExecuted) {
  TestPublisher publisher;
  bats::MessageProcessor processor(&publisher, filter_, latency_logger_.get(), topic_logger_.get());

  TestAddOrder<bats::AddOrderShortMessage>(
    &processor,
    &publisher,
    bats::kMessageTypeAddOrderShort,
    kSymbol,
    kPrice);

  bats::OrderExecutedMessage message;
  message.header.length = sizeof(bats::OrderExecutedMessage);
  message.header.message_type = bats::kMessageTypeOrderExecuted;
  message.ns = 2;
  message.order_id = 1;
  message.size = 20;
  message.execution_id = MakeExecutionId("01ABCDEFG");

  processor.Process(reinterpret_cast<bats::Message*>(&message));
  EXPECT_EQ(4, publisher.GetPublishedCount());

  MarketSnapshot snapshot = publisher.GetSnapshot(1);
  EXPECT_EQ(kExpectedTicker, snapshot.ticker);
  EXPECT_EQ(20, snapshot.last_trade_size);
  EXPECT_EQ(20, snapshot.volume);
  EXPECT_TRUE(snapshot.is_trade_update);

  snapshot = publisher.GetSnapshot(0);
  EXPECT_EQ(kExpectedTicker, snapshot.ticker);
  EXPECT_DOUBLE_EQ(kDoublePrice, snapshot.bids[0]);
  EXPECT_DOUBLE_EQ(kDoublePrice, snapshot.asks[0]);

  // Ignore off exchange executions
  message.ns = 3;
  message.order_id = 2;
  message.size = 25;
  message.execution_id = MakeExecutionId("R00012345");

  processor.Process(reinterpret_cast<bats::Message*>(&message));
  EXPECT_EQ(5, publisher.GetPublishedCount());

  snapshot = publisher.GetSnapshot(1);
  EXPECT_EQ(20, snapshot.last_trade_size);
  EXPECT_EQ(20, snapshot.volume);
  EXPECT_FALSE(snapshot.is_trade_update);

  snapshot = publisher.GetSnapshot(0);
  EXPECT_EQ(kExpectedTicker, snapshot.ticker);
  EXPECT_DOUBLE_EQ(kDoublePrice, snapshot.bids[0]);
  EXPECT_DOUBLE_EQ(kDoublePrice, snapshot.asks[0]);
}

TEST_F(MessageProcessor, OrderExecutedAtPrice) {
  TestPublisher publisher;
  bats::MessageProcessor processor(&publisher, filter_, latency_logger_.get(), topic_logger_.get());

  TestAddOrder<bats::AddOrderShortMessage>(
    &processor,
    &publisher,
    bats::kMessageTypeAddOrderShort,
    kSymbol,
    kPrice);

  // Basic OrderExecutedAtPrice - matches OrderExecuted
  bats::OrderExecutedAtPriceMessage message;
  message.header.length = sizeof(bats::OrderExecutedAtPriceMessage);
  message.header.message_type = bats::kMessageTypeOrderExecutedAtPrice;
  message.ns = 2000;
  message.order_id = 1;
  message.size = 50;
  message.remaining_size = kOrderSize - 50;
  message.execution_id = 12345;
  message.price.price = kLongPrice;

  EXPECT_EQ(2, publisher.GetPublishedCount());
  processor.Process(reinterpret_cast<bats::Message*>(&message));
  EXPECT_EQ(4, publisher.GetPublishedCount());

  MarketSnapshot snapshot = publisher.GetSnapshot(1);
  EXPECT_EQ(kExpectedTicker, snapshot.ticker);
  EXPECT_EQ(50, snapshot.last_trade_size);
  EXPECT_EQ(50, snapshot.volume);
  EXPECT_EQ(true, snapshot.is_trade_update);

  snapshot = publisher.GetSnapshot(0);
  EXPECT_EQ(kExpectedTicker, snapshot.ticker);
  EXPECT_DOUBLE_EQ(kDoublePrice, snapshot.bids[0]);
  EXPECT_DOUBLE_EQ(kDoublePrice, snapshot.asks[0]);
  EXPECT_EQ(kOrderSize - 50, snapshot.bid_sizes[0]);
  EXPECT_EQ(kOrderSize, snapshot.ask_sizes[0]);
  EXPECT_EQ(50, snapshot.last_trade_size);
  EXPECT_EQ(50, snapshot.volume);
  EXPECT_FALSE(snapshot.is_trade_update);

  // OrderExecutedAtPrice with different price - no real change
  message.ns = 3000;
  message.order_id = 1;
  message.size = 25;
  message.remaining_size = 24;
  message.execution_id = 12346;
  message.price.price = kLongPrice + 1;

  processor.Process(reinterpret_cast<bats::Message*>(&message));
  EXPECT_EQ(6, publisher.GetPublishedCount());

  snapshot = publisher.GetSnapshot(1);
  EXPECT_EQ(kExpectedTicker, snapshot.ticker);
  EXPECT_EQ(25, snapshot.last_trade_size);
  EXPECT_EQ(75, snapshot.volume);
  EXPECT_TRUE(snapshot.is_trade_update);

  snapshot = publisher.GetSnapshot(0);
  EXPECT_EQ(kExpectedTicker, snapshot.ticker);
  EXPECT_DOUBLE_EQ(kDoublePrice, snapshot.bids[0]);
  EXPECT_DOUBLE_EQ(kDoublePrice, snapshot.asks[0]);
  EXPECT_EQ(24, snapshot.bid_sizes[0]);
  EXPECT_EQ(kOrderSize, snapshot.ask_sizes[0]);
  EXPECT_EQ(25, snapshot.last_trade_size);
  EXPECT_EQ(75, snapshot.volume);
  EXPECT_FALSE(snapshot.is_trade_update);

  // OrderExecutedAtPrice with different volume
  message.ns = 3000;
  message.order_id = 2;
  message.size = 25;
  message.remaining_size = 15;
  message.execution_id = 12347;
  message.price.price = kLongPrice;

  processor.Process(reinterpret_cast<bats::Message*>(&message));

  snapshot = publisher.GetSnapshot(1);
  EXPECT_EQ(kExpectedTicker, snapshot.ticker);
  EXPECT_EQ(25, snapshot.last_trade_size);
  EXPECT_EQ(100, snapshot.volume);
  EXPECT_TRUE(snapshot.is_trade_update);

  snapshot = publisher.GetSnapshot(0);
  EXPECT_EQ(kExpectedTicker, snapshot.ticker);
  EXPECT_DOUBLE_EQ(kDoublePrice, snapshot.bids[0]);
  EXPECT_DOUBLE_EQ(kDoublePrice, snapshot.asks[0]);
  EXPECT_EQ(24, snapshot.bid_sizes[0]);
  EXPECT_EQ(15, snapshot.ask_sizes[0]);
  EXPECT_EQ(25, snapshot.last_trade_size);
  EXPECT_EQ(100, snapshot.volume);
  EXPECT_FALSE(snapshot.is_trade_update);

  // OrderExecutedAtPrice with different volume and price
  message.ns = 3000;
  message.order_id = 2;
  message.size = 10;
  message.remaining_size = 1;
  message.execution_id = 12347;
  message.price.price = kLongPrice + 100;

  processor.Process(reinterpret_cast<bats::Message*>(&message));

  snapshot = publisher.GetSnapshot(1);
  EXPECT_EQ(kExpectedTicker, snapshot.ticker);
  EXPECT_EQ(10, snapshot.last_trade_size);
  EXPECT_EQ(110, snapshot.volume);
  EXPECT_TRUE(snapshot.is_trade_update);

  snapshot = publisher.GetSnapshot(0);
  EXPECT_EQ(kExpectedTicker, snapshot.ticker);
  EXPECT_DOUBLE_EQ(kDoublePrice, snapshot.bids[0]);
  EXPECT_DOUBLE_EQ(kDoublePrice + 0.01, snapshot.asks[0]);
  EXPECT_EQ(24, snapshot.bid_sizes[0]);
  EXPECT_EQ(1, snapshot.ask_sizes[0]);
  EXPECT_EQ(10, snapshot.last_trade_size);
  EXPECT_EQ(110, snapshot.volume);
  EXPECT_FALSE(snapshot.is_trade_update);
}

TEST_F(MessageProcessor, ReduceSizeLong) {
  TestPublisher publisher;
  bats::MessageProcessor processor(&publisher, filter_, latency_logger_.get(), topic_logger_.get());

  TestAddOrder<bats::AddOrderShortMessage>(
    &processor,
    &publisher,
    bats::kMessageTypeAddOrderShort,
    kSymbol,
    kPrice);

  bats::OrderReduceSizeLongMessage message;
  message.header.length = sizeof(bats::OrderReduceSizeLongMessage);
  message.header.message_type = bats::kMessageTypeOrderReduceSizeLong;
  message.ns = 2000;
  message.order_id = 2;
  message.size = 50;

  processor.Process(reinterpret_cast<bats::Message*>(&message));

  MarketSnapshot snapshot = publisher.GetSnapshot();
  EXPECT_EQ(kExpectedTicker, snapshot.ticker);
  EXPECT_DOUBLE_EQ(kDoublePrice, snapshot.bids[0]);
  EXPECT_DOUBLE_EQ(kDoublePrice, snapshot.asks[0]);
  EXPECT_EQ(kOrderSize, snapshot.bid_sizes[0]);
  EXPECT_EQ(kOrderSize - 50, snapshot.ask_sizes[0]);
  EXPECT_FALSE(snapshot.is_trade_update);
}

TEST_F(MessageProcessor, ReduceSizeShort) {
  TestPublisher publisher;
  bats::MessageProcessor processor(&publisher, filter_, latency_logger_.get(), topic_logger_.get());

  TestAddOrder<bats::AddOrderShortMessage>(
    &processor,
    &publisher,
    bats::kMessageTypeAddOrderShort,
    kSymbol,
    kPrice);

  bats::OrderReduceSizeShortMessage message;
  message.header.length = sizeof(bats::OrderReduceSizeShortMessage);
  message.header.message_type = bats::kMessageTypeOrderReduceSizeShort;
  message.ns = 2000;
  message.order_id = 2;
  message.size = 50;

  processor.Process(reinterpret_cast<bats::Message*>(&message));

  MarketSnapshot snapshot = publisher.GetSnapshot();
  EXPECT_EQ(kExpectedTicker, snapshot.ticker);
  EXPECT_DOUBLE_EQ(kDoublePrice, snapshot.bids[0]);
  EXPECT_DOUBLE_EQ(kDoublePrice, snapshot.asks[0]);
  EXPECT_EQ(kOrderSize, snapshot.bid_sizes[0]);
  EXPECT_EQ(kOrderSize - 50, snapshot.ask_sizes[0]);
  EXPECT_FALSE(snapshot.is_trade_update);
}

TEST_F(MessageProcessor, OrderModifiedLong) {
  TestPublisher publisher;
  bats::MessageProcessor processor(&publisher, filter_, latency_logger_.get(), topic_logger_.get());

  TestAddOrder<bats::AddOrderShortMessage>(
    &processor,
    &publisher,
    bats::kMessageTypeAddOrderShort,
    kSymbol,
    kPrice);

  bats::OrderModifiedLongMessage message;
  message.header.length = sizeof(bats::OrderModifiedLongMessage);
  message.header.message_type = bats::kMessageTypeOrderModifiedLong;
  message.ns = 2000;
  message.order_id = 2;
  message.size = 50;
  message.price.price = kLongPrice + 100;
  message.flags = 0;

  processor.Process(reinterpret_cast<bats::Message*>(&message));

  MarketSnapshot snapshot = publisher.GetSnapshot();
  EXPECT_EQ(kExpectedTicker, snapshot.ticker);
  EXPECT_DOUBLE_EQ(kDoublePrice, snapshot.bids[0]);
  EXPECT_DOUBLE_EQ(kDoublePrice + 0.01, snapshot.asks[0]);
  EXPECT_EQ(kOrderSize, snapshot.bid_sizes[0]);
  EXPECT_EQ(50, snapshot.ask_sizes[0]);
  EXPECT_FALSE(snapshot.is_trade_update);

  // Now, treat the modify as a delete
  message.size = 0;
  processor.Process(reinterpret_cast<bats::Message*>(&message));

  snapshot = publisher.GetSnapshot();
  EXPECT_DOUBLE_EQ(0, snapshot.asks[0]);
  EXPECT_EQ(0, snapshot.ask_sizes[0]);
  EXPECT_FALSE(snapshot.is_trade_update);
}

TEST_F(MessageProcessor, OrderModifiedShort) {
  TestPublisher publisher;
  bats::MessageProcessor processor(&publisher, filter_, latency_logger_.get(), topic_logger_.get());

  TestAddOrder<bats::AddOrderShortMessage>(
    &processor,
    &publisher,
    bats::kMessageTypeAddOrderShort,
    kSymbol,
    kPrice);

  bats::OrderModifiedShortMessage message;
  message.header.length = sizeof(bats::OrderModifiedShortMessage);
  message.header.message_type = bats::kMessageTypeOrderModifiedShort;
  message.ns = 2000;
  message.order_id = 2;
  message.size = 50;
  message.price.price = kPrice + 1;
  message.flags = 0;

  processor.Process(reinterpret_cast<bats::Message*>(&message));

  MarketSnapshot snapshot = publisher.GetSnapshot();
  EXPECT_EQ(kExpectedTicker, snapshot.ticker);
  EXPECT_DOUBLE_EQ(kDoublePrice, snapshot.bids[0]);
  EXPECT_DOUBLE_EQ(kDoublePrice + 0.01, snapshot.asks[0]);
  EXPECT_EQ(kOrderSize, snapshot.bid_sizes[0]);
  EXPECT_EQ(50, snapshot.ask_sizes[0]);
  EXPECT_FALSE(snapshot.is_trade_update);

  // Now, treat the modify as a delete
  message.size = 0;
  processor.Process(reinterpret_cast<bats::Message*>(&message));

  snapshot = publisher.GetSnapshot();
  EXPECT_DOUBLE_EQ(0, snapshot.asks[0]);
  EXPECT_EQ(0, snapshot.ask_sizes[0]);
}

TEST_F(MessageProcessor, DeleteOrder) {
  TestPublisher publisher;
  bats::MessageProcessor processor(&publisher, filter_, latency_logger_.get(), topic_logger_.get());

  TestAddOrder<bats::AddOrderShortMessage>(
    &processor,
    &publisher,
    bats::kMessageTypeAddOrderShort,
    kSymbol,
    kPrice);

  bats::DeleteOrderMessage message;
  message.header.length = sizeof(bats::DeleteOrderMessage);
  message.header.message_type = bats::kMessageTypeDeleteOrder;
  message.ns = 2000;
  message.order_id = 1;

  processor.Process(reinterpret_cast<bats::Message*>(&message));

  MarketSnapshot snapshot = publisher.GetSnapshot();
  EXPECT_EQ(kExpectedTicker, snapshot.ticker);
  EXPECT_DOUBLE_EQ(0, snapshot.bids[0]);
  EXPECT_DOUBLE_EQ(kDoublePrice, snapshot.asks[0]);
  EXPECT_EQ(0, snapshot.bid_sizes[0]);
  EXPECT_EQ(kOrderSize, snapshot.ask_sizes[0]);
  EXPECT_FALSE(snapshot.is_trade_update);

  // Now, treat the modify as a delete
  message.order_id = 2;
  processor.Process(reinterpret_cast<bats::Message*>(&message));

  snapshot = publisher.GetSnapshot();
  EXPECT_DOUBLE_EQ(0, snapshot.asks[0]);
  EXPECT_EQ(0, snapshot.ask_sizes[0]);
}

template<class TradeMessageType, class Symbol, class Price>
void TestTrade(
    const bats::SymbolFilter & filter,
    bats::LatencyLogger* latency_logger,
    TopicLogger* topic_logger,
    bats::kMessageTypes message_type,
    Symbol symbol,
    Price price) {
  TestPublisher publisher;
  bats::MessageProcessor processor(&publisher, filter, latency_logger, topic_logger);

  TestAddOrder<bats::AddOrderShortMessage>(
    &processor,
    &publisher,
    bats::kMessageTypeAddOrderShort,
    kSymbol,
    kPrice);

  TradeMessageType message;
  message.header.length = sizeof(TradeMessageType);
  message.header.message_type = message_type;
  message.ns = 2000;
  message.order_id = 3;
  message.side = 'B';
  message.size = 50;
  message.symbol = symbol;
  message.price.price = price;
  message.execution_id = 3;

  processor.Process(reinterpret_cast<bats::Message*>(&message));

  // Trade messages should not impact book
  MarketSnapshot snapshot = publisher.GetSnapshot();
  EXPECT_EQ(kExpectedTicker, snapshot.ticker);
  EXPECT_DOUBLE_EQ(kDoublePrice, snapshot.bids[0]);
  EXPECT_DOUBLE_EQ(kDoublePrice, snapshot.asks[0]);
  EXPECT_EQ(kOrderSize, snapshot.bid_sizes[0]);
  EXPECT_EQ(kOrderSize, snapshot.ask_sizes[0]);
  EXPECT_DOUBLE_EQ(kDoublePrice, snapshot.last_trade);
  EXPECT_EQ(50, snapshot.last_trade_size);
  EXPECT_EQ(50, snapshot.volume);
  EXPECT_EQ(true, snapshot.is_trade_update);
}

TEST_F(MessageProcessor, TradeLong) {
  TestTrade<bats::TradeLongMessage>(filter_,
                                    latency_logger_.get(),
                                    topic_logger_.get(),
                                    bats::kMessageTypeTradeLong,
                                    kSymbol,
                                    kLongPrice);
}

TEST_F(MessageProcessor, TradeShort) {
  TestTrade<bats::TradeShortMessage>(filter_,
                                     latency_logger_.get(),
                                    topic_logger_.get(),
                                     bats::kMessageTypeTradeShort,
                                     kSymbol,
                                     kPrice);
}

TEST_F(MessageProcessor, TradeExpanded) {
  TestTrade<bats::TradeExpandedMessage>(
    filter_,
    latency_logger_.get(),
    topic_logger_.get(),
    bats::kMessageTypeTradeExpanded,
    kLongSymbol,
    kLongPrice);
}

TEST_F(MessageProcessor, TimeMessage) {
  for (int i = 0; i < 120; ++i) {
    TestPublisher publisher;
    bats::MessageProcessor processor(&publisher, filter_, latency_logger_.get(), topic_logger_.get());

    bats::TimeMessage message;
    message.header.length = sizeof(bats::TimeMessage);
    message.header.message_type = bats::kMessageTypeTime;
    message.time = 2000 + i;

    processor.Process(reinterpret_cast<bats::Message*>(&message));

    // Time messages shouldn't publish anything
    EXPECT_EQ(0, publisher.GetPublishedCount());

    // Ensure time is being set to something
    TestAddOrder<bats::AddOrderShortMessage>(
      &processor,
      &publisher,
      bats::kMessageTypeAddOrderShort,
      kSymbol,
      kPrice);

    MarketSnapshot snapshot = publisher.GetSnapshot();
    EXPECT_NE(0, snapshot.time.tv_sec);
  }
}

// We should be able to cleanly handle new message types
TEST_F(MessageProcessor, UnknownMessage) {
  TestPublisher publisher;
  bats::MessageProcessor processor(&publisher, filter_, latency_logger_.get(), topic_logger_.get());

  char bytes[100];
  bats::Message *message = reinterpret_cast<bats::Message*>(bytes);
  message->header.length = sizeof(bytes);
  message->header.message_type = 0x50;

  processor.Process(message);

  EXPECT_EQ(0, publisher.GetPublishedCount());
}

TEST_F(MessageProcessor, TimeWrapping) {
  const uint64_t one_hour = 60 * 60 * 1000000ULL;
  for (int i = -5; i <= 5; i++) {
    EXPECT_EQ(i, bats::LatencyLogger::NormalizeDiff(i + one_hour));
    EXPECT_EQ(i, bats::LatencyLogger::NormalizeDiff(i - one_hour));
  }
}
