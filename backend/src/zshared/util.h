#ifndef SRC_ZSHARED_UTIL_H_
#define SRC_ZSHARED_UTIL_H_

#define DISALLOW_COPY_AND_ASSIGN(TypeName)      \
  TypeName(const TypeName&);                    \
  void operator=(const TypeName&)

// ZSTATIC_ASSERT is used for compile time assertions.
// ie. ZSTATIC_ASSERT(false) will cause the compile to fail
//
// It uses two tricks:
// 1. Negative array sizes are not allowed.
// 2. You can forward declare functions as many times as you want.
#define ZSTATIC_ASSERT(condition) \
  extern void zstatic_assert(int arg[(condition) ? 1 : -1])

#endif  // SRC_ZSHARED_UTIL_H_
