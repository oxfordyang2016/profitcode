#ifndef SRC_MARKET_HANDLERS_ZBATSDATA_UNIT_H_
#define SRC_MARKET_HANDLERS_ZBATSDATA_UNIT_H_

#include <stdint.h>
#include <string>
#include <vector>
#include <utility>

namespace bats {

class Unit {
 public:
  explicit Unit(const uint8_t id)
    : id_(id) {
  }

  uint8_t GetId() const {
    return id_;
  }

 private:
  uint8_t id_;
};
}

#endif  // SRC_MARKET_HANDLERS_ZBATSDATA_UNIT_H_
