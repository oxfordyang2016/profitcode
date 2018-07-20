#include <stdio.h>

#include <iostream>

#include "simtrade/simdata/datagener.h"

int main() {
  DataGener dg("data", "/root/nick_private/data.log");
  dg.Run();
}
