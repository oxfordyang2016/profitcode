#include "market_handlers/zbatsdata/messages.h"

#include <string>

namespace bats {

union SymbolUnion {
  Symbol symbol;
  struct {
    uint32_t four_bytes;
    uint16_t two_bytes;
  } bytes;
};

Symbol CreateSymbol(const std::string &raw_symbol) {
  Symbol symbol;
  for (size_t i = 0; i < 6; i++) {
    if (i < raw_symbol.size()) {
      symbol.symbol[i] = raw_symbol[i];
    } else {
      symbol.symbol[i] = ' ';
    }
  }
  return symbol;
}

SymbolIndex Symbol::GetAsIndex() const {
  const SymbolUnion *symbol_union = reinterpret_cast<const SymbolUnion*>(this);
  // This matches the eight character symbol.  Little endian only!
  return
    symbol_union->bytes.four_bytes |
    (static_cast<uint64_t>(symbol_union->bytes.two_bytes) << 32) |
    (0x2020ULL << 48);
}

std::string Symbol::GetAsString() const {
  std::string result;
  for (size_t i = 0; i < sizeof(symbol); i++) {
    if (symbol[i] == ' ') {
      break;
    }
    result += symbol[i];
  }
  return result;
}

union LongSymbolUnion {
  LongSymbol symbol;
  uint64_t eight_bytes;
};

LongSymbol CreateLongSymbol(const std::string &raw_symbol) {
  LongSymbol symbol;
  for (size_t i = 0; i < 8; i++) {
    if (i < raw_symbol.size()) {
      symbol.symbol[i] = raw_symbol[i];
    } else {
      symbol.symbol[i] = ' ';
    }
  }
  return symbol;
}

SymbolIndex LongSymbol::GetAsIndex() const {
  const LongSymbolUnion *symbol_union = reinterpret_cast<const LongSymbolUnion*>(this);
  return symbol_union->eight_bytes;
}

std::string LongSymbol::GetAsString() const {
  std::string result;
  for (size_t i = 0; i < sizeof(symbol); i++) {
    if (symbol[i] == ' ') {
      break;
    }
    result += symbol[i];
  }
  return result;
}
}
