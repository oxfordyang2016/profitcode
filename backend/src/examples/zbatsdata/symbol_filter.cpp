#include "market_handlers/zbatsdata/symbol_filter.h"

#include <string>

#include "market_handlers/zbatsdata/messages.h"

namespace bats {

void SymbolFilter::AddFilter(const std::string & filter) {
  filters_.insert(filter);
}

bool SymbolFilter::IsValid(const Symbol & symbol) const {
  return IsValid(symbol.GetAsString());
}

bool SymbolFilter::IsValid(const LongSymbol & symbol) const {
  return IsValid(symbol.GetAsString());
}

bool SymbolFilter::IsValid(const std::string & symbol) const {
  // Special case, if no filters, everything is allowed
  if (filters_.size() == 0) {
    return true;
  }

  return filters_.count(symbol) > 0;
}

}  // namespace bats
