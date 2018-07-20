#ifndef SRC_COMMON_LOGGING_ORDER_LOGGER_H_
#define SRC_COMMON_LOGGING_ORDER_LOGGER_H_

#include <sys/time.h>

#include "common/logging/topic_logger.h"
#include "common/orders/umbrella_order.h"
#include "common/orders/umbrella_order_update.h"

class OrderLogger {
 public:
  explicit OrderLogger(FILE* stream);

  void Log(const timeval & current_time,
           int strategy_id,
           const UmbrellaOrder & order);

  void Log(const timeval & current_time,
           int strategy_id,
           const UmbrellaOrderUpdate & update);

 private:
  TopicLogger logger_;

  DISALLOW_COPY_AND_ASSIGN(OrderLogger);
};

#endif  // SRC_COMMON_LOGGING_ORDER_LOGGER_H_
