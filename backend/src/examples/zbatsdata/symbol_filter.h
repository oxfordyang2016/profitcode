#ifndef SRC_MARKET_HANDLERS_ZBATSDATA_SYMBOL_FILTER_H_
#define SRC_MARKET_HANDLERS_ZBATSDATA_SYMBOL_FILTER_H_

#include <string>
#include <set>

namespace bats {

class Symbol;
class LongSymbol;

class SymbolFilter {
 public:
  // Filters based on exact match only
  void AddFilter(const std::string & filter);

  bool IsValid(const Symbol & symbol) const;
  bool IsValid(const LongSymbol & symbol) const;
  bool IsValid(const std::string & symbol) const;

 private:
  std::set<std::string> filters_;
};
}

#endif  // SRC_MARKET_HANDLERS_ZBATSDATA_SYMBOL_FILTER_H_
