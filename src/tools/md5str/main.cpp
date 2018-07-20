#include <md5.h>

#include <cstdio>
#include <string>

int main(int argc, char** argv) {
  if (argc != 2) {
    printf("Usage: %s <string>\n", argv[0]);
    return -1;
  }

  std::string msg = util::MD5(argv[1]).toStr();
  printf("%s: %s\n", argv[1], msg.c_str());
  return 0;
}
