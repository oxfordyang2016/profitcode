#ifndef ORDER_H_
#define ORDER_H_

#include "define.h"
#include "order_side.h"
#include "order_action.h"
#include "order_status.h"
#include "offset.h"

#include <stdio.h>
#include <sys/time.h>

struct Order {
  char contract[MAX_CONTRACT_LENGTH];
  double price;
  int size;
  OrderSide::Enum side;
  int order_ref;
  OrderAction::Enum action;
  OrderStatus::Enum status;
  Offset::Enum offset;

  bool Valid() {
    if (status == OrderStatus::SubmitNew || status == OrderStatus::New) {
      return true;
    }
    return false;
  }
  void Show(FILE* stream) const {
    timeval time;
    gettimeofday(&time, NULL);
    fprintf(stream, "%ld %06ld Order %s |",
            time.tv_sec, time.tv_usec, contract);

      fprintf(stream, " %lf@%d %s %d %s %s %s\n", price, size, OrderSide::ToString(side), order_ref, OrderAction::ToString(action), OrderStatus::ToString(status), Offset::ToString(offset));
  }
};

#endif  //  ORDER_H_
