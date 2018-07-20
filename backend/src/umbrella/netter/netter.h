#ifndef SRC_UMBRELLA_NETTER_NETTER_H_
#define SRC_UMBRELLA_NETTER_NETTER_H_

#include <assert.h>

#include <vector>

#include "common/logging/position_logger.h"
#include "common/orders/umbrella_order.h"
#include "common/orders/umbrella_order_update.h"
#include "common/wanli_util.h"
#include "common/strategy.h"
#include "umbrella/netter/existing_umbrella_order.h"

class Netter {
 public:
  explicit Netter(PositionLogger* position_logger);

  bool OnUmbrellaOrderUpdate(const UmbrellaOrderUpdate & update,
                             int strategy_id,
                             int strategy_instrument_id);

  // calls strategy.ClearOrderLevels()
  std::vector<UmbrellaOrder>* GetUmbrellaOrders(int strategy_id,
                                                int strategy_instrument_id,
                                                Strategy* strategy,
                                                const timeval & time);

  inline const std::vector<int> & GetLocalPositions(int strategy_id) {
    assert(strategy_id < static_cast<int>(local_positions_.size()));
    return local_positions_[strategy_id];
  }

  inline long BuyTotal(int strategy_id, int local_instrument_id) const {
    return buy_total_[strategy_id][local_instrument_id];
  }

  inline long SellTotal(int strategy_id, int local_instrument_id) const {
    return sell_total_[strategy_id][local_instrument_id];
  }

  inline double AverageBuyPrice(int strategy_id,
                                int local_instrument_id) const {
    return average_buy_price_[strategy_id][local_instrument_id];
  }

  inline double AverageSellPrice(int strategy_id,
                                 int local_instrument_id) const {
    return average_sell_price_[strategy_id][local_instrument_id];
  }

  void InitializeForStrategy(const timeval & current_time,
                             int strategy_id,
                             int num_instruments,
                             int num_tradeable);

 private:
  PositionLogger* position_logger_;

  std::vector<UmbrellaOrder> umbrella_orders_;
  std::vector<std::vector<int> > local_positions_;
  std::vector<int> num_tradeable_;

  std::vector<std::vector<long> > buy_total_;
  std::vector<std::vector<long> > sell_total_;

  std::vector<std::vector<double> > average_buy_price_;
  std::vector<std::vector<double> > average_sell_price_;

  // strategy, instrument (local), side
  std::vector<std::vector<std::vector<std::vector<ExistingUmbrellaOrder> > > >
  existing_umbrella_orders_;

  int order_id_counter_;

  DISALLOW_COPY_AND_ASSIGN(Netter);
};

#endif  // SRC_UMBRELLA_NETTER_NETTER_H_
