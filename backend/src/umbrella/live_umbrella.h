#ifndef SRC_UMBRELLA_LIVE_UMBRELLA_H_
#define SRC_UMBRELLA_LIVE_UMBRELLA_H_

#include <shq/shq.h>
#include <zmq.hpp>
#include <zshared/basic_stats.h>
#include <string>
#include <vector>
#include <utility>

#include "umbrella/umbrella.h"
#include "umbrella/umbrella_order_router.h"

#include "common/messages.h"
#include "common/logging/latency_logger.h"

struct OrderHandlerUpdateInfo {
  shq::TopicMessageSubscriber<messages::CancelUpdateMessage>* subscriber_;
  int user_data_base_;
};

class LiveUmbrella : public Umbrella {
 public:
  LiveUmbrella(
      const std::string & identity,
      FILE* umbrella_log_file,
      FILE* order_log_file,
      FILE* parameter_log_file,
      FILE* position_log_file,
      FILE* pnl_log_file,
      FILE* graph_log_file,
      FILE* latency_log_file,
      const std::vector<std::string> & market_data_feeds,
      const std::vector<std::pair<std::string, std::string > > & zrisk_order_destinations,
      const std::vector<std::string> & zrisk_order_update_feeds,
      const std::vector<std::string> & parameter_update_sockets,
      const timeval & current_time);

  void Start();
  void Stop();

  void RegisterOrderHandlerUpdateTopic(const std::string & order_handler_update_topic, int user_data_base);

 private:
  void Subscribe(const std::string & ticker);
  void SendOrder(const std::string & identity,
                 UmbrellaOrder* order);

  int ProcessMarketData(const timeval &current_time, zshared::BasicStats<int64_t>* perf_data);
  void ProcessParameterUpdate();

  void ProcessOrderHandlerMessages();

 private:
  inline bool is_running() const {
    return is_running_;
  }

 private:
  bool is_running_;

  LatencyLogger latency_logger_;

  shq::TopicMessageSubscriber<MarketSnapshot> market_data_subscriber_;

  UmbrellaOrderRouter umbrella_order_router_;
  shq::TopicMessageSubscriber<UmbrellaOrderUpdate> umbrella_order_update_subscriber_;

  std::vector<OrderHandlerUpdateInfo> order_handler_update_subscribers_;

  zmq::context_t context_;
  zmq::socket_t parameter_update_socket_;
};

#endif  // SRC_UMBRELLA_LIVE_UMBRELLA_H_
