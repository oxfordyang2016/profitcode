#ifndef SRC_ZSHARED_THREAD_H_
#define SRC_ZSHARED_THREAD_H_

#include <pthread.h>
#include <string>

namespace zshared {

enum {
  LOW_PRIORITY_CPU = 1,
};

class Thread {
 public:
  // binds the current thread to this cpu
  // throws exception on failure
  static void BindToCpu(int cpu_id);

  // Returns the cpu the current thread is executing on
  static int GetCpuId();

  // pass in start function, parameters, cpu_id to pin to
  // will exit(1) on failure!
  static pthread_t StartPinnedThread(void (*start_routine)(void* params),
                                     void* params,
                                     int cpu_id,
                                     const std::string & thread_name);
};

}  // namespace zshared

#endif  // SRC_ZSHARED_THREAD_H_
