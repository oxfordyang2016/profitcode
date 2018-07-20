// #ifndef USEFULFUNCTION_H_
// #define USEFULFUNCTION_H_

#include "define.h"
#include "order_side.h"
#include "order_action.h"
#include "order_status.h"
#include "offset.h"

#include <stdio.h>
#include <sys/time.h>

#include <string>
#include <vector>

std::vector<std::string> SplitStr(std::string str, std::string pattern) {
  std::string::size_type pos;
  std::vector<std::string> result;
  str = str + pattern;
  int size = str.size();
  for (int i = 0; i < size - 1; i++) {
      pos = str.find(pattern, i);
      int ps = static_cast<int>(pos);
      if (ps < size) {
            std::string s = str.substr(i, ps-i);
            result.push_back(s);
            i = ps + pattern.size() - 1;
          }
    }
  return result;
}

int main() {
  std::string time = "-9:00:00-11:30:00-";
  std::vector<std::string> v = SplitStr(time, "-");
  for (int i = 0; i < v.size(); i++) {
    printf("string:%s\n", v[i].c_str());
  }
}

// #endif  //  USEFULFUNCTION_H_
