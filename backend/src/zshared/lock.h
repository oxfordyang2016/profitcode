#ifndef SRC_ZSHARED_LOCK_H_
#define SRC_ZSHARED_LOCK_H_

#include <pthread.h>

namespace zshared {

template<class Lock>
class LockGuard {
 public:
  explicit LockGuard(Lock* lock)
    : lock_(lock) {
    lock_->Lock();
  }

  ~LockGuard() {
    lock_->Unlock();
  }

 private:
  Lock* lock_;
};

class SpinLock {
 public:
  SpinLock();
  ~SpinLock();

  void Lock();
  bool TryLock();
  void Unlock();

 private:
  pthread_spinlock_t spin_lock_;
};

class Mutex {
 public:
  explicit Mutex(bool recursive = false);
  ~Mutex();

  void Lock();
  bool TryLock();
  void Unlock();

 private:
  pthread_mutex_t mutex_;
};
}

#endif  // SRC_ZSHARED_LOCK_H_
