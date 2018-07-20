#ifndef SRC_UMBRELLA_MODULES_ORDER_HANDLER_UPDATE_MODULE_H_
#define SRC_UMBRELLA_MODULES_ORDER_HANDLER_UPDATE_MODULE_H_

#include <shq/shq.h>
#include <string>
#include <vector>

#include "umbrella/modules/umbrella_module.h"

class Umbrella;

class OrderHandlerUpdateModule : public UmbrellaModule {
 public:
  OrderHandlerUpdateModule(Umbrella* umbrella,
                           const std::vector<std::string> & order_handler_update_topics);

  virtual void Process();

 private:
  shq::MessageSubscriber order_handler_update_subscriber_;
};

#endif  // SRC_UMBRELLA_MODULES_ORDER_HANDLER_UPDATE_MODULE_H_
