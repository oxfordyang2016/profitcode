#ifndef SRC_SIMTRADE_SIMDATA_DATAGENER_H_
#define SRC_SIMTRADE_SIMDATA_DATAGENER_H_

#include <zmq.hpp>
#include <sender.h>
#include <market_snapshot.h>

#include <iostream>
#include <vector>
#include <fstream>
#include <string>

class DataGener {
 public:
  DataGener(std::string shm_name, std::string data_file);
  ~DataGener();
  void Run();

  std::vector<std::string> Split(std::string raw_string, char split_char) {
    std::vector<std::string> result;
    int pos = -1;
    for (unsigned int i = 0; i < raw_string.size(); i++) {
      if (raw_string[i] == split_char) {
        result.push_back(raw_string.substr(pos+1, i-pos-1));
        pos = i;
      }
    }
    result.push_back(raw_string.substr(pos+1, raw_string.size()-pos));
    return result;
  }

 private:
  MarketSnapshot HandleSnapshot(std::string raw_shot);
  std::ifstream raw_file;
  std::string file_name;
  zmq::context_t* context;
  zmq::socket_t* socket;
  Sender* sender;
  FILE* out_file;
};

#endif  // SRC_SIMTRADE_SIMDATA_DATAGENER_H_
