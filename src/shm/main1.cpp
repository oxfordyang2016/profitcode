#include "shm.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>

int main() {
  Shm m1;
  m1.Connect("huangxinyu");
  char* a = (char*)m1.Read();
  printf("read %s\n", a);
  sleep(1);
}
