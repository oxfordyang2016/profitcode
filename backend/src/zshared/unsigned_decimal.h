#ifndef SRC_ZSHARED_UNSIGNED_DECIMAL_H_
#define SRC_ZSHARED_UNSIGNED_DECIMAL_H_

#include <stdint.h>
#include <stdexcept>

namespace zshared {

class UnsignedDecimal {
 public:
  UnsignedDecimal() {}
  UnsignedDecimal(uint64_t value, uint32_t fractional_digits) {
    uint64_t pow10 = kPow10[fractional_digits];
    uint64_t whole = value / pow10;
    uint64_t fractional = value - (whole * pow10);

    // Normalize fractional to 6 decimal places
    const uint32_t kNormalizedDigits = 6;
    if (fractional_digits > kNormalizedDigits) {
      throw std::runtime_error("Can't have more fractional digits than normalized digits");
    }
    fractional *= kPow10[kNormalizedDigits - fractional_digits];

    value_ = (whole * kPow10[kNormalizedDigits]) + fractional;
    double_value_ = whole + (static_cast<double>(fractional) / kPow10[kNormalizedDigits]);
  }

  void SetValues(uint64_t value, double double_value) {
    value_ = value;
    double_value_ = double_value;
  }

  inline uint64_t GetRawValue() const {
    return value_;
  }

  inline double GetDoubleValue() const {
    return double_value_;
  }

 private:
  uint64_t value_;
  double double_value_;

  static const uint64_t kPow10[10];
};
}

#endif  // SRC_ZSHARED_UNSIGNED_DECIMAL_H_
