#ifndef SRC_SIMTRADE_STRAT-MA_STRATEGY_H_
#define SRC_SIMTRADE_STRAT-MA_STRATEGY_H_

#include <market_snapshot.h>
#include <order.h>
#include <sender.h>
#include <exchange_info.h>
#include <order_status.h>
#include <pricer_data.h>
#include <tr1/unordered_map>

#include <cmath>
#include <vector>
#include <string>

#include "simtrade/strat-MA/timecontroller.h"

class Strategy {
 public:
  Strategy();
  ~Strategy();

  void Start();
  void Stop();
  void UpdateData(MarketSnapshot shot);
  void UpdateExchangeInfo(ExchangeInfo info);
  bool DataReady();
 private:
  void RequestQryPos();
  void NewOrder(std::string contract, OrderSide::Enum side, int size = 1, bool control_price = false);
  void ModOrder(Order* o);
  void DelOrder(int ref);

  double OrderPrice(std::string contract, OrderSide::Enum side, bool control_price = false);

  int GenerateUniqueOrderRef();

  void SimpleHandle(int line);

  double PriceCorrector(double price, bool is_upper = false) {
    int a = price/min_price;
    if (is_upper && fabs(a*min_price-price) < 0.00001) {
      return a*min_price + min_price;
    }
    return a*min_price;
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

  std::vector<std::string> m_contracts;
  bool position_ready;
  bool is_started;
  Sender* sender;
  MarketSnapshot main_shot;
  MarketSnapshot hedge_shot;
  tr1::unordered_map<std::string, tr1::unordered_map<std::string, double>> pricer_map;  // contract:topic:data
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

#endif  // SRC_SIMTRADE_STRAT-MA_STRATEGY_H_
