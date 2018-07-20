#include "market_handlers/zbatsdata/unit_processor.h"

#include <algorithm>
#include <functional>

#include "common/zentero_util.h"
#include "market_handlers/zbatsdata/spin_listener.h"

namespace bats {

UnitProcessor::UnitProcessor(
    const Unit & unit,
    const TcpServer & spin_server,
    ISnapshotPublisher* publisher,
    const SymbolFilter & filter,
    LatencyLogger* latency_logger,
    size_t max_cached_messages,
    TopicLogger* topic_logger)
    : unit_(unit),
      spin_server_(spin_server),
      sequence_no_(0),
      processor_publisher_(publisher),
      processor_filter_(filter),
      latency_logger_(latency_logger),
      max_cached_messages_(max_cached_messages),
      needs_restart_(true),
      topic_logger_(topic_logger) {
}

void UnitProcessor::Restart(uint32_t min_sequence_num) {
  int retry_count = 0;
  int sleep_time = 250;  // ms
  while (needs_restart_) {
    processor_.reset(new MessageProcessor(processor_publisher_,
                                          processor_filter_,
                                          latency_logger_,
                                          topic_logger_));

    // SpinListener will set processor_ to not publish internally
    topic_logger_->Log("INFO", "Starting Spin");
    try {
      uint32_t last_seq_num_processed = SpinListener::RebuildBook(unit_,
                                                                  spin_server_,
                                                                  min_sequence_num,
                                                                  processor_.get(),
                                                                  topic_logger_);
      topic_logger_->Log("INFO", "Spin finished at %d", last_seq_num_processed);
      sequence_no_ = last_seq_num_processed + 1;
      needs_restart_ = false;
    } catch (std::runtime_error & error) {
      if (++retry_count <= 10) {
        topic_logger_->Log("ERROR", "Exception during Spin '%s'.  Retry %d",
          error.what(), retry_count);

        usleep(sleep_time * 1000);

        // Exponential backoff
        sleep_time = std::min(sleep_time * 2, 30000);
      } else {
        // Re-throw the error
        throw error;
      }
    }
  }
}

// only need to check cache in two cases:
// a) just successfully processed a message
// b) just did a restart
void UnitProcessor::Process(const SequencedUnit* container) {
  if (container->header.unit != unit_.GetId()) {
    throw std::runtime_error("Incorrect unit passed to processor");
  }

  if (container->header.message_count == 0) {
    // Heartbeat messages can be safely ignored
    return;
  }

  if (!needs_restart_) {
    if (ProcessMessages(container)) {
      // just processed a message successfully
      CheckCachedMessages();
      return;
    }

    const uint32_t first_sequence_no = container->header.sequence_no;
    const uint32_t last_sequence_no = first_sequence_no + container->header.message_count;
    // old message, no need to cache it
    if (sequence_no_ >= last_sequence_no) {
      return;
    }
  }

  // we get here either when we need a restart or the message needs to be cached

  if (container->header.sequence_no == 0) {
    // this message always processed, wait until we get a message with an actual
    // sequence number
    return;
  }

  // store in cache if possible, otherwise reset cache and force restart
  if (cached_containers_.size() >= max_cached_messages_) {
    topic_logger_->Log("WARNING", "Too many cached messages %d - clearing cache",
      max_cached_messages_);
    while (!cached_containers_.empty()) {
      const char* cached_bytes = cached_containers_.top();
      delete[] cached_bytes;
      cached_containers_.pop();
    }
    needs_restart_ = true;
  }

  // Cache the container
  topic_logger_->Log("INFO", "Caching packet (seq_no %d | msg_count %d | unit %d)",
                     container->header.sequence_no,
                     container->header.message_count,
                     unit_.GetId());
  char* raw_bytes = new char[container->header.length];
  memcpy(raw_bytes, container, container->header.length);
  cached_containers_.push(raw_bytes);

  // if we need to restart, do it and then try this function again
  if (needs_restart_) {
    Restart(container->header.sequence_no);
    CheckCachedMessages();
  }
}

bool UnitProcessor::ProcessMessages(const SequencedUnit *container) {
  const uint32_t first_sequence_no = container->header.sequence_no;
  const uint32_t last_sequence_no = first_sequence_no + container->header.message_count;

  // if first_sequence_no == 0, then process no matter what (unsequenced message)
  // if our next expected sequence number is greater than this entire packet,
  // we don't need to process it
  if (first_sequence_no != 0 && sequence_no_ >= last_sequence_no) {
    return false;
  }

  // the gap case
  if (first_sequence_no != 0 && sequence_no_ < first_sequence_no) {
    return false;
  }

  // Start processing at the correct message
  int start = first_sequence_no == 0 ? 0 : sequence_no_ - first_sequence_no;

  // Go through all the messages, and update the state
  int offset = 0;
  for (int i = 0; i < container->header.message_count; i++) {
    const Message *message = reinterpret_cast<const Message*>(container->data + offset);
    if (i >= start) {
      processor_->Process(message);
    }
    offset += message->header.length;
  }

  if (container->header.sequence_no != 0) {
    sequence_no_ = last_sequence_no;
  }

  return true;
}

void UnitProcessor::CheckCachedMessages() {
  // Check for any cached messages that are now available
  while (!cached_containers_.empty()) {
    const char* cached_bytes = cached_containers_.top();
    const SequencedUnit* container = reinterpret_cast<const SequencedUnit*>(cached_bytes);

    // Check if we can process this message
    if (ProcessMessages(container)) {
      topic_logger_->Log("INFO", "Processed cached message %d | unit %d",
        container->header.sequence_no, unit_.GetId());

      cached_containers_.pop();
      delete[] cached_bytes;
    } else if (sequence_no_ > container->header.sequence_no) {
      topic_logger_->Log("INFO", "Cached message %d is now stale", container->header.sequence_no);

      cached_containers_.pop();
      delete[] cached_bytes;
    } else {
      // We haven't caught up to the cache yet
      break;
    }
  }
}

}  // namespace bats
