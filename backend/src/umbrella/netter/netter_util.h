#ifndef SRC_UMBRELLA_NETTER_NETTER_UTIL_H_
#define SRC_UMBRELLA_NETTER_NETTER_UTIL_H_

#include <vector>

#include "common/orders/strategy_order.h"
#include "common/orders/umbrella_order.h"
#include "common/orders/order_side.h"
#include "umbrella/netter/existing_umbrella_order.h"

// main entry function, calls all the other ones belowx
// pending_cancel_limit = how many pending cancel's we'll tolerate
// before subtracting them out of our order requests
void ProcessStrategyOrders(const char* ticker,
                           OrderSide::Enum side,
                           std::vector<StrategyOrder> *strategy_orders,
                           std::vector<ExistingUmbrellaOrder>*
                           existing_umbrella_orders,
                           std::vector<UmbrellaOrder>* umbrella_orders,
                           int pending_cancel_limit,
                           int* order_id_counter,
                           int* max_num_new_orders,
                           bool allow_price_modifies,
                           bool allow_size_modifies,
                           int min_order_size,
                           const timeval & time,
                           int default_user_data,
                           int max_netter_group);

void SubtractInFlightOrders(OrderSide::Enum side,
                            std::vector<StrategyOrder> *strategy_orders,
                            const std::vector<ExistingUmbrellaOrder>
                            & existing_umbrella_orders,
                            int pending_cancel_limit,
                            int netter_group);

void AdjustUmbrellaOrders(const char* ticker,
                          OrderSide::Enum side,
                          std::vector<StrategyOrder> *strategy_orders,
                          std::vector<ExistingUmbrellaOrder>*
                          existing_umbrella_orders,
                          std::vector<UmbrellaOrder>* umbrella_orders,
                          int* order_id_counter,
                          int* max_num_new_orders,
                          bool allow_price_modifies,
                          bool allow_size_modifies,
                          int min_order_size,
                          const timeval & time,
                          int default_user_data,
                          int netter_group);

bool AddUmbrellaOrder(const char* ticker,
                      OrderSide::Enum side,
                      int order_id,
                      int size,
                      double price,
                      std::vector<UmbrellaOrder>* umbrella_orders,
                      const timeval & time,
                      int user_data,
                      std::vector<ExistingUmbrellaOrder>
                      *existing_umbrella_orders);

void CancelUmbrellaOrder(const char* ticker,
                         OrderSide::Enum side,
                         int order_id,
                         const timeval & time,
                         int user_data,
                         std::vector<UmbrellaOrder>* umbrella_orders);

void ReplaceUmbrellaOrder(const char* ticker,
                          OrderSide::Enum side,
                          int order_id,
                          int size,
                          double price,
                          const timeval & time,
                          int user_data,
                          std::vector<UmbrellaOrder>* umbrella_orders);

#endif  // SRC_UMBRELLA_NETTER_NETTER_UTIL_H_
