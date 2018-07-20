#ifndef SRC_COMMON_INSTRUMENT_RECORD_H_
#define SRC_COMMON_INSTRUMENT_RECORD_H_

#include <string>

struct InstrumentRecord {
  std::string ticker;

  int position_limit;
  int order_limit;
  int pending_order_limit;

  double tick_size;
  double value_of_one_point;
  bool allow_price_modifies;
  bool allow_size_modifies;
  int min_order_size;
  int user_data;

  InstrumentRecord()
      : ticker("UNKNOWN/UNKNOWN/"),
        position_limit(0),
        order_limit(0),
        pending_order_limit(0),
        tick_size(1.0),
        value_of_one_point(1.0),
        allow_price_modifies(true),
        allow_size_modifies(true),
        min_order_size(0),
        user_data(0) {
  }
};

#endif  // SRC_COMMON_INSTRUMENT_RECORD_H_
