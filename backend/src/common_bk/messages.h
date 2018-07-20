#ifndef SRC_COMMON_MESSAGES_H_
#define SRC_COMMON_MESSAGES_H_

#include "common/limits.h"

namespace messages {

enum kMessageTypes {
  kMessageTypeCancelUpdate = 0x01,
};

#pragma pack(push, 1)

struct MessageHeader {
  uint16_t length;
  uint16_t message_type;
};

struct Message {
  MessageHeader header;
  char data[];
};

struct CancelUpdateMessage {
  MessageHeader header;

  char ticker[MAX_TICKER_LENGTH];
  bool over_warning_cancel_limit;
  bool over_danger_cancel_limit;

  static const kMessageTypes kType = kMessageTypeCancelUpdate;
};

#pragma pack(pop)

template<class MessageType>
void InitializeMessage(MessageType* message) {
  message->header.length = sizeof(MessageType);
  message->header.message_type = MessageType::kType;
}
}

#endif  // SRC_COMMON_MESSAGES_H_
