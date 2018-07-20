#include "shm.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>

int main() {
  Shm m1;
  m1.Connect("huangxinyu");
  std::string input_str;
  printf("enter the string:");
  while (std::cin >> input_str) {
    char input_c[1000];
    snprintf(input_c, sizeof(input_c), "%s", input_str.c_str());
    printf("%s\n", input_c);
    m1.Write(input_c);
    sleep(1);
    printf("\nenter the string:");
  }
}
