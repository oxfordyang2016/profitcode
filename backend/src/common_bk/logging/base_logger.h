#ifndef SRC_COMMON_LOGGING_BASE_LOGGER_H_
#define SRC_COMMON_LOGGING_BASE_LOGGER_H_

#include <pthread.h>
#include <shq/shq.h>
#include <stdio.h>
#include <sys/time.h>

#include <tr1/memory>

#include <string>
#include <queue>

// Inlined here to avoid circular include with wanli_util.h
#define DISALLOW_COPY_AND_ASSIGN(TypeName)      \
  TypeName(const TypeName&);                    \
  void operator=(const TypeName&)

class BaseLogger {
 public:
  // column_names will appear in header
  explicit BaseLogger(FILE* stream);
  explicit BaseLogger(const char* file);  // creates stream

  ~BaseLogger();

  void Log(const char* str, ...);
  void Log(const char* str, va_list arglist);
  void LogTimestamp(const char* str, ...);

 private:
  static void* ThreadEntry(void* arg);

 private:
  void Initialize();

  // called by worker thread
  void ThreadLogMessages();

 private:
  FILE* stream_;
  bool we_own_file_;

  pthread_t thread_;
  bool is_alive;

  std::string shq_topic_address_;
  std::tr1::shared_ptr<shq::Publisher> publisher_;
  std::tr1::shared_ptr<shq::Subscriber> subscriber_;

  DISALLOW_COPY_AND_ASSIGN(BaseLogger);
};

#endif  // SRC_COMMON_LOGGING_BASE_LOGGER_H_
