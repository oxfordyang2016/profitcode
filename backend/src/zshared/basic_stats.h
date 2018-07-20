#ifndef SRC_ZSHARED_BASIC_STATS_H_
#define SRC_ZSHARED_BASIC_STATS_H_

#include <algorithm>
#include <limits>

namespace zshared {

template<class T>
struct BasicStats {
  T sum, min, max, count;

  BasicStats() {
    Reset();
  }

  void Reset() {
    sum = 0;
    min = std::numeric_limits<T>::max();
    max = std::numeric_limits<T>::min();
    count = 0;
  }

  void Event(T value) {
    sum += value;
    min = std::min(min, value);
    max = std::max(max, value);
    count++;
  }
};
}

#endif  // SRC_ZSHARED_BASIC_STATS_H_
