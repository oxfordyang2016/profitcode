#ifndef SRC_SIMTRADE_CTPORDER_TOKEN_MANAGER_H_
#define SRC_SIMTRADE_CTPORDER_TOKEN_MANAGER_H_

#include <order.h>
#include <stdio.h>
#include <stdlib.h>
#include <ThostFtdcUserApiDataType.h>

#include <tr1/unordered_map>
#include <string>

class TokenManager {
 public:
  TokenManager():
    ctp_id(0) {
    pthread_mutex_init(&token_mutex, NULL);
  }
  void Init() {
  }

  void RegisterToken(std::string contract, int num, OrderSide::Enum side) {
    // TODO(nick): add lock
    printf("Register token %s %s %d\n", contract.c_str(), OrderSide::ToString(side), num);
    if (side == OrderSide::Buy) {
      sell_token[contract] += num;
    } else {
      buy_token[contract] += num;
    }
  }
  void RegisterOrderRef(Order o) {
    if (o.order_ref == 0) {
      order_id_map.clear();
    }
    order_map[ctp_id] = o;
    order_id_map[o.order_ref] = ctp_id++;
  }

  int GetCtpId(Order o) {
    std::tr1::unordered_map<int, int>::iterator it = order_id_map.find(o.order_ref);
    if (it == order_id_map.end()) {
      printf("ctporderref not found for %d\n", o.order_ref);
      return -1;
    }
    return it->second;
  }

  int GetOrderRef(int ctp_id) {
    for (std::tr1::unordered_map<int, int>::iterator it = order_id_map.begin(); it != order_id_map.end(); it++) {
      if (it->second == ctp_id) {
        return it->first;
      }
    }
    printf("ctpref %d not found!\n", ctp_id);
    return -1;
  }

  Order GetOrder(int ctp_order_ref) {
    std::tr1::unordered_map<int, Order>::iterator it = order_map.find(ctp_order_ref);
    if (it == order_map.end()) {
      printf("order not found for ctpref %d\n", ctp_order_ref);
    }
    return it->second;
  }

  int CheckOffset(Order order) {
    printf("check offset for order %d: size is %d, side is %s, and token is: buy %d, sell %d\n", order.order_ref, order.size, OrderSide::ToString(order.side), buy_token[order.contract], sell_token[order.contract]);
    int ctp_order_ref = GetCtpId(order);
    int pos;
    if (order.side == OrderSide::Buy) {
      pos = buy_token[order.contract];
    } else if (order.side == OrderSide::Sell) {
      pos = sell_token[order.contract];
    } else {
      printf("token unknown side!\n");
      exit(1);
    }
    if (pos < order.size) {
      return THOST_FTDC_OF_Open;
    } else {
      if (order.side == OrderSide::Buy) {
        pthread_mutex_lock(&token_mutex);
        buy_token[order.contract] -= order.size;
        pthread_mutex_unlock(&token_mutex);
      } else {
        pthread_mutex_lock(&token_mutex);
        sell_token[order.contract] -= order.size;
        pthread_mutex_unlock(&token_mutex);
      }
      is_close[ctp_order_ref] = true;
      return THOST_FTDC_OF_CloseToday;
    }
  }

  void HandleFilled(Order o) {
    int ctp_order_ref = GetCtpId(o);
    if (o.side == OrderSide::Buy && !is_close[ctp_order_ref]) {
      pthread_mutex_lock(&token_mutex);
      sell_token[o.contract] += o.size;
      pthread_mutex_unlock(&token_mutex);
    } else if (o.side == OrderSide::Sell && !is_close[ctp_order_ref]) {
      pthread_mutex_lock(&token_mutex);
      buy_token[o.contract] += o.size;
      pthread_mutex_unlock(&token_mutex);
    } else {
      // printf("unknown side!listener 251\n");
    }
  }

  void HandleCancelled(Order o) {
    int ctp_order_ref = GetCtpId(o);
    if (o.side == OrderSide::Buy && is_close[ctp_order_ref]) {
      pthread_mutex_lock(&token_mutex);
      buy_token[o.contract] += o.size;
      pthread_mutex_unlock(&token_mutex);
    } else if (o.side == OrderSide::Sell && is_close[ctp_order_ref]) {
      pthread_mutex_lock(&token_mutex);
      sell_token[o.contract] += o.size;
      pthread_mutex_unlock(&token_mutex);
    } else {
      // printf("unknown side!listener 251\n");
    }
  }

 private:
  std::tr1::unordered_map<std::string, int> buy_token;
  std::tr1::unordered_map<std::string, int> sell_token;
  std::tr1::unordered_map<int, int> order_id_map;
  std::tr1::unordered_map<int, Order> order_map;
  std::tr1::unordered_map<int, bool> is_close;
  pthread_mutex_t token_mutex;
  int ctp_id;
};

#endif  // SRC_SIMTRADE_CTPORDER_TOKEN_MANAGER_H_
