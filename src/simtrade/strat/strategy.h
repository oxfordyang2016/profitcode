#ifndef SRC_SIMTRADE_STRAT_STRATEGY_H_
#define SRC_SIMTRADE_STRAT_STRATEGY_H_

#include <market_snapshot.h>
#include <order.h>
#include <sender.h>
#include <exchange_info.h>
#include <order_status.h>
#include <tr1/unordered_map>

#include <cmath>
#include <vector>
#include <string>

#include "simtrade/strat/timecontroller.h"

struct StratStatus {
  enum SS {
    OFF,
    OPEN,
    ON,
    CLOSE
  };
};

class Strategy {
 public:
  Strategy(std::string main_ticker, std::string hedge_ticker, int maxpos, double tick_size, TimeController tc, int contract_size, bool enable_stdout = true, bool enable_file = true);
  ~Strategy();

  void Start();
  void Stop();
  void UpdateData(MarketSnapshot shot);
  void UpdateExchangeInfo(ExchangeInfo info);
  bool IsReady();
 private:
  void RequestQryPos();
  void NewOrder(std::string contract, OrderSide::Enum side, int size = 1, bool control_price = false);
  void ModOrder(Order* o);
  void DelOrder(int ref);

  double OrderPrice(std::string contract, OrderSide::Enum side, bool control_price = false);

  int GenerateUniqueOrderRef();

  void SimpleHandle(int line);

  bool IsHedged() {
    int main_pos = position_map[main_shot.ticker];
    int hedge_pos = position_map[hedge_shot.ticker];
    return (main_pos == -hedge_pos);
  }

  double PriceCorrector(double price, bool is_upper = false) {
    int a = price/min_price;
    if (is_upper && fabs(a*min_price-price) < 0.00001) {
      return a*min_price + min_price;
    }
    return a*min_price;
  }

  double CalBalancePrice() {
    int netpos = position_map[main_shot.ticker];
    if (netpos > 0) {  // buy pos, sell close order
      return PriceCorrector(hedge_shot.asks[0]+avgcost_main-avgcost_hedge, true);
    } else if (netpos < 0) {
      return PriceCorrector(hedge_shot.bids[0]+avgcost_main-avgcost_hedge);
    } else {
      printf("pos is 0 when calbalance price!\n");
      exit(1);
    }
  }

  bool TradeClose(std::string contract, int size) {
    int pos = position_map[contract];
    return (pos*size <= 0);
  }

  void UpdateAvgCost(std::string contract, double trade_price, int size) {
    double capital_change = trade_price*size;
    if (contract == main_shot.ticker) {
      int current_pos = position_map[contract];
      int pre_pos = current_pos - size;
      avgcost_main = (avgcost_main * pre_pos + capital_change)/current_pos;
    } else if (contract == hedge_shot.ticker) {
      int current_pos = position_map[contract];
      int pre_pos = current_pos - size;
      avgcost_hedge = (avgcost_hedge * pre_pos + capital_change)/current_pos;
    } else {
      printf("updateavgcost error: unknown ticker %s\n", contract.c_str());
      exit(1);
    }
  }

  bool DoubleEqual(double a, double b, double min_vaule = 0.0000001) {
    if (fabs(a-b) < min_vaule) {
      return true;
    }
    return false;
  }

  bool PriceChange(double current_price, double reasonable_price, OrderSide::Enum side = OrderSide::Buy) {
    bool is_bilateral = true;
    if (position_map[main_shot.ticker] > 0 && side == OrderSide::Sell) {
      is_bilateral = false;
    }
    if (position_map[main_shot.ticker] < 0 && side == OrderSide::Buy) {
      is_bilateral = false;
    }
    if (is_bilateral) {
      if (fabs(current_price - reasonable_price) <= edurance) {
        return false;
      }
      return true;
    } else {
      if (side == OrderSide::Buy) {
        if (current_price > reasonable_price) {
          return true;
        }
        if (reasonable_price - current_price > edurance) {
          return true;
        }
        return false;
      } else {
        if (current_price < reasonable_price) {
          return true;
        }
        if (current_price - reasonable_price > edurance) {
          return true;
        }
        return false;
      }
    }
  }

  void CancelAllHedge();
  void CancelAllMain();
  void ClearValidOrder();
  void ModerateMainOrders();
  void ModerateHedgeOrders();
  void UpdatePos(Order* o, ExchangeInfo info);
  void CloseAllTodayPos();
  void AddCloseOrderSize(OrderSide::Enum side) {
    pthread_mutex_lock(&add_size_mutex);
    Order * reverse_order = NULL;
    for (std::tr1::unordered_map<int, Order*>::iterator it = main_order_map.begin(); it != main_order_map.end(); it++) {
      if (it->second->Valid() && it->second->side == side) {
        reverse_order = it->second;
        reverse_order->size++;
        printf("add close ordersize from %d -> %d\n", reverse_order->size-1, reverse_order->size);
        ModOrder(reverse_order);
      } else if (it->second->status == OrderStatus::Modifying && it->second->side == side) {
        reverse_order = it->second;
        reverse_order->size++;
        printf("2nd add close ordersize from %d -> %d\n", reverse_order->size-1, reverse_order->size);
      }
    }
    if (reverse_order == NULL) {
      printf("not found reverse side order!119\n");
      exit(1);
    }
    pthread_mutex_unlock(&add_size_mutex);
    printf("release the lock\n");
  }

  std::vector<std::string> m_contracts;
  bool position_ready;
  bool is_started;
  Sender* sender;
  MarketSnapshot main_shot;
  MarketSnapshot hedge_shot;
  tr1::unordered_map<int, Order*>main_order_map;
  tr1::unordered_map<int, Order*>hedge_order_map;
  tr1::unordered_map<std::string, int> position_map;
  int order_ref;
  StratStatus::SS current_status;
  pthread_mutex_t order_ref_mutex;
  pthread_mutex_t order_map_mutex;
  pthread_mutex_t add_size_mutex;
  pthread_mutex_t mod_mutex;
  int max_pos;
  FILE* order_file;
  FILE* exchange_file;
  bool e_s;
  bool e_f;
  double poscapital;
  double min_price;
  double price_control;
  double avgcost_main;
  double avgcost_hedge;
  double edurance;
  TimeController m_tc;
  int m_contract_size;
};

#endif  // SRC_SIMTRADE_STRAT_STRATEGY_H_
