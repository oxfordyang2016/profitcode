#ifndef SRC_COMMON_INSTRUMENT_DEFINITIONS_H_
#define SRC_COMMON_INSTRUMENT_DEFINITIONS_H_

#include <boost/regex_fwd.hpp>
#include <boost/scoped_ptr.hpp>

#include <tr1/unordered_map>
#include <libconfig.h++>

#include <string>
#include <vector>

#include "common/wanli_util.h"
#include "common/limits.h"

class InstrumentDefinitions {
 public:
  explicit InstrumentDefinitions(const char* config_file);

  // return -1 if no matches
  int FindIndex(const std::string & ticker);

  // return -1 if no match
  // doens't do local cache (preserves const)
  int FindIndexConst(const std::string & ticker) const;

  inline double GetTickSize(int index) const {
    return tick_sizes_[index];
  }

  inline double GetValueOfOnePoint(int index) const {
    return value_of_one_points_[index];
  }

  inline int num_patterns() const { return ticker_patterns_.size(); }

  // can't return reference since str() creates temporary variable
  const std::string GetPattern(int index) const;

 private:
  std::vector<std::tr1::shared_ptr<boost::regex> > ticker_patterns_;
  std::vector<double> tick_sizes_;
  std::vector<double> value_of_one_points_;

  std::tr1::unordered_map<std::string, int> idx_lookup_;

  DISALLOW_COPY_AND_ASSIGN(InstrumentDefinitions);
};

#endif  // SRC_COMMON_INSTRUMENT_DEFINITIONS_H_
