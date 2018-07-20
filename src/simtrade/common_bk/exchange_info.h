#ifndef EXCHANGE_INFO_H_
#define EXCHANGE_INFO_H_

#include "define.h"
#include "info_type.h"

struct ExchangeInfo {
  InfoType::Enum type;
  int order_ref;
  int trade_size;
  double trade_price;
  char reason[EXCHANGE_INFO_SIZE];

  void Show(FILE* stream) const {
    timeval time;
    gettimeofday(&time, NULL);
    fprintf(stream, "%ld %06ld exchangeinfo %d |",
            time.tv_sec, time.tv_usec, order_ref);

    fprintf(stream, " %lf@%d %s\n", trade_price, trade_size, InfoType::ToString(type));
  }
};

#endif  //  EXCHANGE_INFO_H_
