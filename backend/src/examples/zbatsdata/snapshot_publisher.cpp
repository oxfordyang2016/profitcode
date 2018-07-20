#include "market_handlers/zbatsdata/snapshot_publisher.h"

#include <string>

#include "common/market_snapshot.h"
#include "common/zentero_util.h"

namespace bats {

ShqSnapshotPublisher::ShqSnapshotPublisher(const std::string & topic_name,
                                           const std::string & prefix)
  : publisher_(topic_name),
    prefix_(prefix) {
}

void ShqSnapshotPublisher::Publish(const MarketSnapshot & snapshot) {
  publisher_.Send(&snapshot, sizeof(snapshot));
}

const std::string & ShqSnapshotPublisher::GetPrefix() const {
  return prefix_;
}

////////////////////////////////////

FileSnapshotPublisher::FileSnapshotPublisher(FILE* file, const std::string & prefix)
  : file_(file),
    prefix_(prefix) {
}

void FileSnapshotPublisher::Publish(const MarketSnapshot & snapshot) {
  snapshot.Show(file_);
}

const std::string & FileSnapshotPublisher::GetPrefix() const {
  return prefix_;
}

////////////////////////////////////

BinaryFileSnapshotPublisher::BinaryFileSnapshotPublisher(FILE* file, const std::string & prefix)
  : file_(file),
    prefix_(prefix) {
  // Code copied from ~/datatools/src/snapshot/snapshot_reader.cpp
  // Write a signature, so we know it's a converted file
  fwrite(&SnapshotBinarySignature, 1, 1, file_);
  char dummy[3];
  fwrite(dummy, 3, 1, file_);
}

void BinaryFileSnapshotPublisher::Publish(const MarketSnapshot & snapshot) {
  fwrite(&snapshot, sizeof(snapshot), 1, file_);
}

const std::string & BinaryFileSnapshotPublisher::GetPrefix() const {
  return prefix_;
}
}
