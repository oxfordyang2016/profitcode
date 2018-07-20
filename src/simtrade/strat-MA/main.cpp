#include <string.h>
#include <stdio.h>
#include <zmq.hpp>
#include <order.h>
#include <recver.h>
#include <sender.h>
#include <market_snapshot.h>
#include <tr1/unordered_map>

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <string>

#include "simtrade/strat-MA/strategy.h"

void HandleLeft() {
}

void PrintResult() {
}

void* RunExchangeListener(void *param) {
  Strategy* st = reinterpret_cast<Strategy*>(param);
  Recver recver("exchange_info");
  while (true) {
    ExchangeInfo info;
    info = recver.Recv(info);
    st->UpdateExchangeInfo(info);
  }
  return NULL;
}

int main() {
  std::vector<std::string> sleep_time_v;
  sleep_time_v.push_back("10:14:00-10:30:00");
  sleep_time_v.push_back("11:29:20-13:30:00");
  sleep_time_v.push_back("00:58:20-09:00:00");
  TimeController tc(sleep_time_v, "21:00:00", "14:55:30");
  Recver pd_recver("pricer_data");
  // Strategy s("ni1901", "ni1811", 50, 30, tc, 1);
  pthread_t exchange_thread;
  if (pthread_create(&exchange_thread,
                     NULL,
                     &RunExchangeListener,
                     &s) != 0) {
    perror("pthread_create");
    exit(1);
  }
  while (true) {
    PricerData pd;
    pd = pd_recver.Recv(pd);
    pd.Show(stdout);
    s.UpdateData(shot);
  }
  HandleLeft();
  PrintResult();
}
