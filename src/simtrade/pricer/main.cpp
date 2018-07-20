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

double CalEMA(std::vector<double>*ema_v, MarketSnapshot shot, int param = 10) {
  shot.Show(stdout);
  if (ema_v->empty()) {
    return shot.last_trade;
  }
  double ema = (ema_v->back() * (param-1) + shot.last_trade)/param;
  return ema;
}

int GetNextCalTime(int sec, int interval = 60) {  // default is 1min
  int a = sec/60;
  return (a+1)*60;
}

int main() {
  int interval = 60;
  Recver r("data");
  std::vector<double>ema_v;
  std::vector<double>ma_v;
  int next_cal_time = 9*3600;
  int time_zone_diff = 8*3600;
  long int a = 0;
  MarketSnapshot pre_shot;
  bool is_first = true;
  while (a < 1531378798) {
    MarketSnapshot shot;
    shot = r.Recv(shot);
    if (is_first) {
      pre_shot = shot;
      is_first = false;
    }
    a = shot.time.tv_sec;
    int sec = (shot.time.tv_sec + time_zone_diff)%(24*3600);
    if (sec >= next_cal_time) {
      if (sec - next_cal_time > interval) {
        int makeup_times = (sec-next_cal_time)/interval;
        printf("time is %d:%d:%d, nexttime is %d:%d:%d, makeup %d times\n", sec/3600, sec%3600/60, sec%60, next_cal_time/3600, next_cal_time%3600/60, next_cal_time%60, makeup_times);
        for (int i = 0; i< makeup_times; i++) {
          ema_v.push_back(ema_v.back());
        }
      }
      double new_ema = CalEMA(&ema_v, pre_shot);
      ema_v.push_back(new_ema);
      next_cal_time = GetNextCalTime(sec);
    }
    pre_shot = shot;
  }
  printf("start print vector out:\n");
  for (int i = 0; i < ema_v.size(); i++) {
    printf("%d: %lf\n", i, ema_v[i]);
  }
}
