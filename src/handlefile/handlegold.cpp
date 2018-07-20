#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

std::vector<std::string> Split(std::string raw_string, char split_char) {
  std::vector<std::string> result;
  int pos = -1; 
  for (unsigned int i = 0; i < raw_string.size(); i++) {
    if (raw_string[i] == split_char) {
      if (raw_string.substr(pos+1, i-pos-1) != "") {
        result.push_back(raw_string.substr(pos+1, i-pos-1));
      }
      pos = i;
    }   
  }
  result.push_back(raw_string.substr(pos+1, raw_string.size()-pos));
  return result;
}

int main() {
  std::fstream ag;
  ag.open("AU1712.txt");
  char g[1000];
  while (!ag.eof()) {
    ag.getline(g, 1000);
    std::vector<string> v = Split(g, '\t');
    cout << v[8] << "\n";
  }
  /*
  std::string s = "a  b c  asd      huang";
  std::vector<std::string> v = Split(s, ' ');
  for (int i = 0; i < v.size(); i++) {
    std::cout << v[i] << "\n";
  }
  */
}
