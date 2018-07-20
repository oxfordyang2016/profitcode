#ifndef SRC_MARKET_HANDLERS_ZBATSDATA_SNAPSHOT_PUBLISHER_H_
#define SRC_MARKET_HANDLERS_ZBATSDATA_SNAPSHOT_PUBLISHER_H_

#include <shq/shq.h>
#include <string>

class MarketSnapshot;

namespace bats {

class ISnapshotPublisher {
 public:
  virtual ~ISnapshotPublisher() {}
  virtual void Publish(const MarketSnapshot &snapshot) = 0;
  virtual const std::string & GetPrefix() const = 0;
};

class ShqSnapshotPublisher : public ISnapshotPublisher {
 public:
  explicit ShqSnapshotPublisher(const std::string & topic_name,
                                const std::string & prefix);
  virtual void Publish(const MarketSnapshot & snapshot);
  virtual const std::string & GetPrefix() const;

 private:
  shq::Publisher publisher_;
  std::string prefix_;
};

class FileSnapshotPublisher : public ISnapshotPublisher {
 public:
  explicit FileSnapshotPublisher(FILE *file, const std::string & prefix);
  virtual void Publish(const MarketSnapshot & snapshot);
  virtual const std::string & GetPrefix() const;

 private:
  FILE *file_;
  std::string prefix_;
};

class BinaryFileSnapshotPublisher : public ISnapshotPublisher {
 public:
  explicit BinaryFileSnapshotPublisher(FILE *file, const std::string & prefix);
  virtual void Publish(const MarketSnapshot & snapshot);
  virtual const std::string & GetPrefix() const;

 private:
  FILE *file_;
  std::string prefix_;
};
}

#endif  // SRC_MARKET_HANDLERS_ZBATSDATA_SNAPSHOT_PUBLISHER_H_
