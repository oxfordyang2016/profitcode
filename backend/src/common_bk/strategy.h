#ifndef SRC_COMMON_STRATEGY_H_
#define SRC_COMMON_STRATEGY_H_

#include <assert.h>
#include <stdio.h>
#include <sys/time.h>

#include <string>
#include <vector>

#include "common/market_snapshot.h"
#include "common/orders/strategy_order.h"
#include "common/orders/order_side.h"
#include "common/instrument_record.h"

#include "common/strategy_setup_workspace.h"

class Strategy {
 public:
  virtual ~Strategy() {}
  virtual void ProcessTick(const timeval & current_time,
                           const std::vector<const MarketSnapshot*> & snapshots,
                           const std::vector<int> & local_positions) = 0;
  virtual bool ProcessParameterUpdate(const timeval & current_time,
                                      const std::string & key,
                                      const std::string & value) = 0;
  virtual void OnFill(const timeval & current_time,
                      int strategy_instrument_id,
                      OrderSide::Enum side,
                      int size,
                      double price,
                      const std::vector<int> & local_positions) = 0;

  ///////////////////

  virtual const InstrumentRecord & Instrument(int idx) const = 0;
  virtual const std::vector<InstrumentRecord> & Instruments() const = 0;

  virtual std::vector<StrategyOrder>*
  GetBuyStrategyOrders(int instrument) = 0;

  virtual std::vector<StrategyOrder>*
  GetSellStrategyOrders(int instrument) = 0;

  virtual void ClearOrderLevels() = 0;

  virtual bool is_ready() const = 0;
  virtual bool is_stopped() const = 0;
  virtual bool is_paused() const = 0;

  virtual double pnl_limit() const = 0;

  // called after placing a new order
  virtual void DecreaseCounter(const timeval & current_time) = 0;
  // number of new orders left that can be placed (-1 = no limit)
  virtual int counter() const = 0;

  // so we know how many netter iterations we need to do
  virtual int max_netter_group() const = 0;

  // maximum amount of time to go between update calls to the strategy
  virtual int idle_update_time_ms() {
    return 3000;
  }
};

typedef Strategy* (*strategy_create_function)
    (const StrategySetupWorkspace &);

extern "C" Strategy* Create(const StrategySetupWorkspace & workspace);

#endif  // SRC_COMMON_STRATEGY_H_
