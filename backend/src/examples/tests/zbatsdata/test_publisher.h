#ifndef SRC_TESTS_ZBATSDATA_TEST_PUBLISHER_H_
#define SRC_TESTS_ZBATSDATA_TEST_PUBLISHER_H_

#include <string>

#include "market_handlers/zbatsdata/snapshot_publisher.h"

class TestPublisher : public bats::ISnapshotPublisher {
 public:
  TestPublisher()
    : prefix_("BATS_TEST"),
      published_count_(0) {
  }
  ~TestPublisher() {}

  virtual void Publish(const MarketSnapshot & snapshot) {
    snapshot_[1] = snapshot_[0];
    snapshot_[0] = snapshot;
    ++published_count_;
  }

  virtual const std::string & GetPrefix() const {
    return prefix_;
  }

  const MarketSnapshot & GetSnapshot(int offset = 0) const {
    return snapshot_[offset];
  }

  int GetPublishedCount() const {
    return published_count_;
  }

 private:
  MarketSnapshot snapshot_[2];
  std::string prefix_;
  int published_count_;
};

#endif  // SRC_TESTS_ZBATSDATA_TEST_PUBLISHER_H_
