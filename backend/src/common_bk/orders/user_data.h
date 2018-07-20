#ifndef SRC_COMMON_ORDERS_USER_DATA_H_
#define SRC_COMMON_ORDERS_USER_DATA_H_

#include <stdexcept>

namespace UserData {

enum OrderType {
  kOrderType_Limit = 1,
  kOrderType_FAK = 1,
};

enum CustomData {
  kCustomData_IgnoreResend = 1,
};

inline int ToOrderType(int user_data) {
  return user_data & 0xFF;
}

inline int ToChinaAccount(int user_data) {
  return (user_data >> 8) & 0xFF;
}

inline int ToCustomData(int user_data) {
  return (user_data >> 16) & 0xFF;
}

inline int FromChinaAccount(int china_account) {
  if (china_account < 0 || china_account >= 256) {
    throw std::runtime_error("china_account value out of range");
  }
  return china_account << 8;
}
}

#endif  // SRC_COMMON_ORDERS_USER_DATA_H_
