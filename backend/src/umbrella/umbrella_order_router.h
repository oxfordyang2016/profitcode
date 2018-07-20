#ifndef SRC_UMBRELLA_UMBRELLA_ORDER_ROUTER_H_
#define SRC_UMBRELLA_UMBRELLA_ORDER_ROUTER_H_

#include <string>
#include <vector>

#include "common/wanli_util.h"

class UmbrellaOrderRouter {
 public:
  UmbrellaOrderRouter() {
  }

  void AddDestination(const std::string & prefix,
                      const std::string & destination) {
    publishers_.push_back(std::tr1::shared_ptr<shq::Publisher>(new shq::Publisher(destination)));
    prefixes_.push_back(prefix);
  }

  // returns true if could find destination
  bool SendOrder(const UmbrellaOrder & order) {
    for (size_t i = 0; i < prefixes_.size(); ++i) {
      // find first prefix that matches
      if (strncmp(order.ticker, prefixes_[i].c_str(), prefixes_[i].size()) == 0) {
        publishers_[i]->Send(&order, sizeof(order));
        return true;
      }
    }
    return false;
  }

 private:
  std::vector<std::string> prefixes_;
  std::vector<std::tr1::shared_ptr<shq::Publisher> > publishers_;

 private:
  DISALLOW_COPY_AND_ASSIGN(UmbrellaOrderRouter);
};


#endif  // SRC_UMBRELLA_UMBRELLA_ORDER_ROUTER_H_
