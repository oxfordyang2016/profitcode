#ifndef SRC_COMMON_LIMITS_H_
#define SRC_COMMON_LIMITS_H_

// in MarketSnapshot.h and many Order structures
#define MAX_TICKER_LENGTH 32

// In umbrella_order_update.h
const size_t MAX_IDENTITY_LENGTH = 36;

// in parameter_update.h
#define MAX_HOST_NAME_LENGTH 32
#define MAX_PARAMETER_NAME_LENGTH 32
#define MAX_PARAMETER_VALUE_LENGTH 32

#define MAX_NUM_INSTRUMENTS 500

#define MAX_LINE_LENGTH 3000

// in netter.cpp (max orders in one instance from one strategy,instrument pair)
#define UMBRELLA_ORDERS_RESERVE_LENGTH 200

// in umbrella.cpp
#define MAX_NUM_SNAPSHOTS_PROCESSED_PER_TICK 500

// initial sizes of many hash tables
#define MAX_OUTSTANDING_ORDERS 3000

// initialize size of hash table for instrument definitions
#define INSTRUMENT_DEFINITIONS_RESERVE_SIZE 300

// in base_logger.cpp
#define MAX_LOG_MESSAGE_LENGTH 1000

#define MARKET_DATA_DEPTH 5

// in risk_manager_order
#define MAX_CL_ORDER_ID_LENGTH 16

// in strategies/base_strategy.h
const int kMaxNetterGroupLimit = 10;

#endif  // SRC_COMMON_LIMITS_H_
