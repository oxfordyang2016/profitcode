#ifndef SRC_COMMON_WANLI_UTIL_H_
#define SRC_COMMON_WANLI_UTIL_H_

#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>

#include <string>

#include "zshared/util.h"
#include "logging/base_logger.h"

extern BaseLogger log_stdout;

void Log(FILE* stream, const char* str, ...);
void Log(FILE* stream, const timeval & time, const char* str, ...);

// printf color stuff
#define RED  "\033[22;31m"
#define GREEN  "\033[22;32m"
#define YELLOW  "\033[22;33m"
#define RESET_COLOR "\033[0m"

#ifdef DEBUG
#define LOG_DEBUG(format, ...)                                          \
  Log(stderr, "DEBUG   %s: %3d: %s: " format "\n",                      \
      ExtractFileName(__FILE__), __LINE__, __func__, ##__VA_ARGS__)
#else
#define LOG_DEBUG(format, ...)
#endif

#define LOG_TOPIC_STDOUT(topic, format, ...)                             \
  Log(stdout, topic " " format "\n", ##__VA_ARGS__)
#define LOG_TOPIC_STDERR(topic, format, ...)                             \
  Log(stderr, topic " " format "\n", ##__VA_ARGS__)

#ifdef DEBUG
#define LOG_DEBUG_WITH_TIME(time, format, ...)                           \
  Log(stderr, time, "DEBUG   %s: %3d: %s: " format "\n",                 \
      ExtractFileName(__FILE__), __LINE__, __func__, ##__VA_ARGS__)
#else
#define LOG_DEBUG_WITH_TIME(format, ...)
#endif

#define LOG_INFO(format, ...)                                           \
  log_stdout.LogTimestamp("INFO    %s: %3d: %s: " format "\n",                   \
      ExtractFileName(__FILE__), __LINE__, __func__, ##__VA_ARGS__)
#define LOG_WARNING(format, ...)                                        \
  Log(stderr, "WARNING %s: %3d: %s: " format "\n",                   \
      ExtractFileName(__FILE__), __LINE__, __func__, ##__VA_ARGS__);
#define LOG_ERROR(format, ...)                                          \
  Log(stderr, "ERROR   %s: %3d: %s: " format "\n",                   \
      ExtractFileName(__FILE__), __LINE__, __func__, ##__VA_ARGS__);
/*
  // colored log messages (makes things harder to parse later)
#define LOG_WARNING(format, ...)                                        \
  fprintf(stderr, YELLOW);                                              \
  Log(stderr, "WARNING %s: %3d: %s: " format "\n",                      \
      ExtractFileName(__FILE__), __LINE__, __func__, ##__VA_ARGS__);    \
  fprintf(stderr, RESET_COLOR)
#define LOG_ERROR(format, ...)                                          \
  fprintf(stderr, RED);                                                 \
  Log(stderr, "ERROR   %s: %3d: %s: " format "\n",                      \
      ExtractFileName(__FILE__), __LINE__, __func__, ##__VA_ARGS__);    \
  fprintf(stderr, RESET_COLOR)
*/

#define EPS 0.00000001

// a - b
inline long DiffTimeUs(const timeval & a,
                       const timeval & b) {
  long diff = a.tv_sec - b.tv_sec;
  diff *= 1000000;
  diff += a.tv_usec - b.tv_usec;

  return diff;
}

inline long DiffTimeMs(const timeval & a,
                       const timeval & b) {
  long diff = a.tv_sec - b.tv_sec;
  diff *= 1000;
  diff += (a.tv_usec - b.tv_usec)/1000;

  return diff;
}

inline double DiffTimeSeconds(const timeval & a,
                              const timeval & b) {
  double diff = a.tv_sec - b.tv_sec;
  diff += (a.tv_usec - b.tv_usec)*(1.0/1000000.0);

  return diff;
}

const char* ExtractFileName(const char* full_path);

std::string GetExchange(const char* ticker);
std::string GetContract(const char* ticker);

// converts interface (such as "eth0") to inet_address
std::string ConvertInterfaceToInetAddress(const char* interface);

std::string GetHostName();

#endif  // SRC_COMMON_WANLI_UTIL_H_
