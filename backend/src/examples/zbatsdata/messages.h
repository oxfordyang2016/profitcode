#ifndef SRC_MARKET_HANDLERS_ZBATSDATA_MESSAGES_H_
#define SRC_MARKET_HANDLERS_ZBATSDATA_MESSAGES_H_

#include <stdint.h>
#include <string>

#include "zshared/unsigned_decimal.h"

namespace bats {

// Based off of BATS Multicast PITCH Specification 2.15.1

enum kMessageTypes {
  // General messages
  kMessageTypeLogin                 = 0x01,
  kMessageTypeLoginResponse         = 0x02,
  kMessageTypeGapRequest            = 0x03,
  kMessageTypeGapResponse           = 0x04,

  // Multicast messages
  kMessageTypeTime                  = 0x20,
  kMessageTypeAddOrderLong          = 0x21,
  kMessageTypeAddOrderShort         = 0x22,
  kMessageTypeOrderExecuted         = 0x23,
  kMessageTypeOrderExecutedAtPrice  = 0x24,
  kMessageTypeOrderReduceSizeLong   = 0x25,
  kMessageTypeOrderReduceSizeShort  = 0x26,
  kMessageTypeOrderModifiedLong     = 0x27,
  kMessageTypeOrderModifiedShort    = 0x28,
  kMessageTypeDeleteOrder           = 0x29,
  kMessageTypeTradeLong             = 0x2A,
  kMessageTypeTradeShort            = 0x2B,
  kMessageTypeTradeBreak            = 0x2C,
  kMessageTypeEndOfSession          = 0x2D,
  kMessageTypeSymbolMapping         = 0x2E,
  kMessageTypeAddOrderExpanded      = 0x2F,
  kMessageTypeTradeExpanded         = 0x30,
  kMessageTypeTradingStatus         = 0x31,
  kMessageTypeUnitClear             = 0x97,

  // Spin server messages
  kMessageTypeSpinImageAvailable    = 0x80,
  kMessageTypeSpinRequest           = 0x81,
  kMessageTypeSpinResponse          = 0x82,
  kMessageTypeSpinFinished          = 0x83,
};

#pragma pack(1)

struct SequencedUnitHeader {
  uint16_t length;
  uint8_t  message_count;
  uint8_t  unit;
  uint32_t sequence_no;
};

struct SequencedUnit {
  SequencedUnitHeader header;
  char data[];
};

struct MessageHeader {
  uint8_t length;
  uint8_t message_type;
};

struct Message {
  MessageHeader header;
  char data[];
};

typedef uint64_t SymbolIndex;

struct Symbol {
  SymbolIndex GetAsIndex() const;
  std::string GetAsString() const;

  char symbol[6];  // Symbol name right padded with spaces
};

struct LongSymbol {
  SymbolIndex GetAsIndex() const;
  std::string GetAsString() const;

  char symbol[8];  // Symbol name right padded with spaces
};

Symbol CreateSymbol(const std::string &raw_symbol);
LongSymbol CreateLongSymbol(const std::string &raw_symbol);

struct BLPrice {
  uint64_t price;

  inline zshared::UnsignedDecimal GetUnsignedDecimal() const {
    return zshared::UnsignedDecimal(price, 4);
  }
};

struct BSPrice {
  uint16_t price;

  inline zshared::UnsignedDecimal GetUnsignedDecimal() const {
    return zshared::UnsignedDecimal(price, 2);
  }
};

////////////////////////////////////////////////////////////
// Book related messages
////////////////////////////////////////////////////////////

struct TimeMessage {
  MessageHeader header;

  uint32_t time;   // Number of seconds since midnight EST
};

struct AddOrderLongMessage {
  MessageHeader header;

  uint32_t ns;        // Nanosecond offset from last timestamp
  uint64_t order_id;  // Day-specific identifier assigned to this order
  char     side;      // 'B'uy or 'S'ell
  uint32_t size;      // Number of shares/contracts being added
  Symbol   symbol;
  BLPrice  price;
  uint8_t  flags;     // Add flags (Bit 0: 0 - not displayed in SIP, 1 displayed in SIP)
};

struct AddOrderShortMessage {
  MessageHeader header;

  uint32_t ns;        // Nanosecond offset from last timestamp
  uint64_t order_id;  // Day-specific identifier assigned to this order
  char     side;      // 'B'uy or 'S'ell
  uint16_t size;      // Number of shares/contracts being added
  Symbol   symbol;
  BSPrice  price;
  uint8_t  flags;     // Bit 0: unset - not displayed in SIP, set - displayed in SIP
};

struct AddOrderExpandedMessage {
  MessageHeader header;

  uint32_t    ns;           // Nanosecond offset from last timestamp
  uint64_t    order_id;     // Day-specific identifier assigned to this order
  char        side;         // 'B'uy or 'S'ell
  uint32_t    size;         // Number of shares/contracts being added
  LongSymbol  symbol;
  BLPrice     price;
  uint8_t     flags;        // Add flags (Bit 0: 0 - not displayed in SIP, 1 displayed in SIP)
  uint32_t    participant;  // Optional: MPID of firm
};

struct OrderExecutedMessage {
  MessageHeader header;

  uint32_t ns;            // Nanosecond offset from last timestamp
  uint64_t order_id;      // Order ID of a previously sent Add Order message
  uint32_t size;          // Number of shares/contracts executed
  uint64_t execution_id;  // Day-unique execution identifier
};

struct OrderExecutedAtPriceMessage {
  MessageHeader header;

  uint32_t ns;              // Nanosecond offset from last timestamp
  uint64_t order_id;        // Order ID of a previously sent Add Order message
  uint32_t size;            // Number of shares/contracts executed
  uint32_t remaining_size;  // Number of shares/contracts remaining after execution
  uint64_t execution_id;    // Day-unique execution identifier
  BLPrice  price;           // Execution price of the order
};

struct OrderReduceSizeLongMessage {
  MessageHeader header;

  uint32_t ns;            // Nanosecond offset from last timestamp
  uint64_t order_id;      // Order ID of a previously sent Add Order message
  uint32_t size;          // Number of shares/contracts canceled
};

struct OrderReduceSizeShortMessage {
  MessageHeader header;

  uint32_t ns;            // Nanosecond offset from last timestamp
  uint64_t order_id;      // Order ID of a previously sent Add Order message
  uint16_t size;          // Number of shares/contracts canceled
};

struct OrderModifiedLongMessage {
  MessageHeader header;

  uint32_t ns;            // Nanosecond offset from last timestamp
  uint64_t order_id;      // Order ID of a previously sent Add Order message
  uint32_t size;          // Number of shares/contracts remaining on this order
  BLPrice  price;         // Limit order price after this modify
  char     flags;         // Bit 0: unset - Not displayed in SIP, set - Displayed in SIP
                          // Bit 1: unset - Reset priority, set - Maintain priority
};

struct OrderModifiedShortMessage {
  MessageHeader header;

  uint32_t ns;            // Nanosecond offset from last timestamp
  uint64_t order_id;      // Order ID of a previously sent Add Order message
  uint16_t size;          // Number of shares/contracts remaining on this order
  BSPrice  price;         // Limit order price after this modify
  char     flags;         // Bit 0: unset - Not displayed in SIP, set - Displayed in SIP
                          // Bit 1: unset - Reset priority, set - Maintain priority
};

struct DeleteOrderMessage {
  MessageHeader header;

  uint32_t ns;        // Nanosecond offset from last timestamp
  uint64_t order_id;  // Day-specific identifier assigned to this order
};

struct TradeLongMessage {
  MessageHeader header;

  uint32_t ns;              // Nanosecond offset from last timestamp
  uint64_t order_id;        // Day-specific identifier assigned to this order
  char     side;            // Always 'B'uy, regardless of resting side
  uint32_t size;            // Incremental number of shares/contracts executed
  Symbol   symbol;
  BLPrice  price;
  uint64_t execution_id;    // Day-unique execution identifier
};

struct TradeShortMessage {
  MessageHeader header;

  uint32_t ns;              // Nanosecond offset from last timestamp
  uint64_t order_id;        // Day-specific identifier assigned to this order
  char     side;            // Always 'B'uy, regardless of resting side
  uint16_t size;            // Incremental number of shares/contracts executed
  Symbol   symbol;
  BSPrice  price;
  uint64_t execution_id;    // Day-unique execution identifier
};

struct TradeExpandedMessage {
  MessageHeader header;

  uint32_t    ns;              // Nanosecond offset from last timestamp
  uint64_t    order_id;        // Day-specific identifier assigned to this order
  char        side;            // Always 'B'uy, regardless of resting side
  uint32_t    size;            // Incremental number of shares/contracts executed
  LongSymbol  symbol;
  BLPrice     price;
  uint64_t    execution_id;    // Day-unique execution identifier
};

struct TradingStatusMessage {
  MessageHeader header;

  uint32_t ns;              // Nanosecond offset from last timestamp
  uint64_t order_id;        // Day-specific identifier assigned to this order
  Symbol   symbol;
  char     halt_status;     // 'H' - halted, 'Q' - quote-only, 'T' - trading
  char     reg_sho_action;  // '0' - No price test in effect
                            // '1' - Reg SHO price test restriction in effect
  char     reserved1;
  char     reserved2;
};

struct UnitClearMessage {
  MessageHeader header;

  uint32_t ns;  // Nanosecond offset from last unit timestamp
};

////////////////////////////////////////////////////////////
// Misc messages
////////////////////////////////////////////////////////////

struct LoginMessage {
  MessageHeader header;

  char session_sub_id[4];
  char username[4];
  char filler[2];
  char password[10];
};

struct LoginResponseMessage {
  MessageHeader header;

  char status;  // 'A' - Login accepted
                // 'N' - Not authorized
                // 'B' - Session in use
                // 'S' - Invalid session
};

struct GapRequestMessage {
  MessageHeader header;

  uint8_t  unit;          // Unit for gap request
  uint32_t sequence_no;   // Sequence of first message
  uint16_t count;         // Number of messages requested
};

struct GapResponseMessage {
  MessageHeader header;

  uint8_t  unit;          // Unit for gap request
  uint32_t sequence_no;   // Sequence of first message
  uint16_t count;         // Number of messages requested
  char     status;        // 'A' - Accepted
                          // All non-'A' responses should be considered rejections
};

struct SpinImageAvailableMessage {
  MessageHeader header;

  uint32_t sequence_no;  // The latest sequence number which is available for spinning
};

struct SpinRequestMessage {
  MessageHeader header;

  uint32_t sequence_no;  // Request book information up to this sequence number
};

struct SpinResponseMessage {
  MessageHeader header;

  uint32_t sequence_no;  // The sequence number that will be filled up to
  uint32_t order_count;  // Number of orders contained in this spin
  char     status;       // 'A'ccepted,
                         // 'O'ut of range (spin no longer avail.),
                         // 'S'pin already in progress
};

struct SpinFinishedMessage {
  MessageHeader header;

  uint32_t sequence_no;
};

#pragma pack()
}

#endif  // SRC_MARKET_HANDLERS_ZBATSDATA_MESSAGES_H_
