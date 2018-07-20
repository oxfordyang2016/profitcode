#ifndef SRC_UMBRELLA_UMBRELLA_H_
#define SRC_UMBRELLA_UMBRELLA_H_

#include <stdio.h>
#include <stdlib.h>

#include <tr1/unordered_map>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "common/orders/umbrella_order.h"
#include "common/orders/umbrella_order_update.h"

#include "common/logging/order_logger.h"
#include "common/logging/parameter_logger.h"
#include "common/logging/pnl_logger.h"
#include "common/logging/position_logger.h"

#include "common/instrument_definitions.h"
#include "common/market_snapshot.h"
#include "common/parameter_update.h"
#include "common/strategy.h"

#include "umbrella/netter/netter.h"
#include "umbrella/umbrella_order_info.h"

class Umbrella {
 public:
  explicit Umbrella(const std::string & identity,
                    FILE* umbrella_log_file,
                    FILE* order_log_file,
                    FILE* parameter_log_file,
                    FILE* position_log_file,
                    FILE* pnl_log_file,
                    FILE* graph_log_file,
                    const timeval & current_time);

  virtual ~Umbrella() = 0;

  void StartStrategy(const char* strategy_library_path,
                     const char* startup_config_path,
                     const InstrumentDefinitions & instrument_definitions,
                     int user_data_base = 0,
                     const std::string & account_name = "");


  void OnTickerCancelLimit(const std::string & ticker, bool over_danger_limit, int user_data_base);

  inline int NumStrategies() const {
    return static_cast<int>(strategies_.size());
  }

 protected:
  void UpdatePnlLog(bool force_update);
  void UpdateCurrentTime(const timeval & current_time);

  inline const timeval & current_time() const {
    return current_time_;
  }

  inline const std::string & GetIdentity() const {
    return identity_;
  }

  void OnParameterUpdate(int strategy_id,
                         const std::string & key,
                         const std::string & value);
  void OnMarketUpdate(const MarketSnapshot & snapshot);
  void OnUmbrellaOrderUpdate(const UmbrellaOrderUpdate & update);

  virtual void Stop() = 0;
  virtual void Subscribe(const std::string & ticker) = 0;

  // calls SendOrder (optional logging parameters)
  void SendUmbrellaOrders(int64_t* time_step_A = NULL,
                          int64_t* time_step_B = NULL,
                          int64_t* time_step_C = NULL,
                          int64_t* time_step_N = NULL);

  virtual void SendOrder(const std::string & identity,
                         UmbrellaOrder* order) = 0;

  //////////////////////////////////////////////////
  // Virtual to enable the simulator to override the logging behavior

  virtual void LogOrder(const timeval & current_time, int strat_id, const UmbrellaOrder & order) {
    order_logger_.Log(current_time, strat_id, order);
  }
  virtual void LogOrder(const timeval & current_time, int strat_id, const UmbrellaOrderUpdate & update) {
    order_logger_.Log(current_time, strat_id, update);
  }
  virtual void LogPnl(int strat_id, double pnl) {
    pnl_logger_.Log(current_time(), strat_id, pnl);
  }

  virtual TopicLogger* GetGraphLogger() {
    return &graph_logger_;
  }

 private:
  void WriteUmbrellaLog(FILE* umbrella_log_file);
  void InitializeFlushTime();

  // umbrella's local mapping of ticker -> id
  int GetInstrumentUmbrellaId(const std::string & ticker);

  void RefreshUmbrellaOrders();

 private:
  std::string identity_;

  timeval current_time_;
  timeval last_pnl_update_time_;
  bool first_pnl_update_needed_;

  OrderLogger order_logger_;
  ParameterLogger parameter_logger_;
  PositionLogger position_logger_;
  PnlLogger pnl_logger_;
  TopicLogger graph_logger_;

  //////////////////////

  std::vector<Strategy*> strategies_;  // length strategies

  std::vector<MarketSnapshot> all_market_snapshots_;  // length instruments

  // length strategies (points to all_market_snapshots_ (so we better make sure
  // that all_market_snapshots_ doesn't reallocate it's data array...)
  std::vector<std::vector<const MarketSnapshot*> > local_market_snapshots_;

  std::tr1::unordered_map<std::string, int> ticker_to_id_;

  // for a given instrument, store which strategies trade it
  std::vector<std::vector<int> > instrument_strategies_;  // length instrument

  std::vector<bool> strategy_needs_updating_;  // length strategies
  std::vector<timeval> last_time_strategy_updated_;  // length strategies
  std::vector<bool> instruments_touched_flag_;  // length instruments
  std::vector<int> instruments_touched_;

  std::vector<int> num_tradeable_;  // length strategies
  std::vector<double> strategy_pnl_;  // length strategies
  std::vector<int> user_data_offset_;  // length strategies

  // Offset to randomize strategy updates
  size_t strat_offset_;

  //////////////////////

  // maps the umbrella_order_id to the strategy_id and the original
  // umbrella_order
  std::map<int, UmbrellaOrderInfo> umbrella_order_id_to_info_;

  Netter netter_;

  ///////////////////////

  DISALLOW_COPY_AND_ASSIGN(Umbrella);
};

#endif  // SRC_UMBRELLA_UMBRELLA_H_
