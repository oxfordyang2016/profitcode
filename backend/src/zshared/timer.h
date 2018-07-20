#ifndef SRC_ZSHARED_TIMER_H_
#define SRC_ZSHARED_TIMER_H_

#include <stdint.h>
#include <sys/time.h>

namespace zshared {

class Timer {
 public:
  Timer() {
    Reset();
  }

  explicit Timer(const timeval & time)
    : time_(time) {
  }

  // Initialize time in milliseconds
  explicit Timer(uint64_t time) {
    time_.tv_sec = time / 1000000;
    time_.tv_usec = time % 1000000;
  }

  inline void Reset() {
    gettimeofday(&time_, NULL);
  }

  inline uint64_t Microseconds() const {
    return time_.tv_sec * 1000000 + time_.tv_usec;
  }

  inline int64_t Elapsed() const {
    return Timer().Microseconds() - Microseconds();
  }

  inline double ElapsedSeconds() const {
    return Elapsed() / 1000000.0;
  }

  inline timeval GetTimeval() const {
    return time_;
  }

 private:
  timeval time_;
};
}

#endif  // SRC_ZSHARED_TIMER_H_
