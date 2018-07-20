#include <iostream>
#include <string>
#include <vector>

#include "simtrade/simdata/datagener.h"

#define MAXN 10000000
#define SIZE_OF_SNAPSHOT 1024

// using namespace std;
DataGener::DataGener(std::string des_name, std::string data_file)
  : file_name(data_file),
    context(NULL),
    socket(NULL) {
  sender = new Sender(des_name.c_str());
  out_file = fopen("test.txt", "w");
}

DataGener::~DataGener() {
  raw_file.close();
  fclose(out_file);
  delete sender;
}

MarketSnapshot DataGener::HandleSnapshot(std::string raw_shot) {
  MarketSnapshot shot;
  std::vector<std::string> symbol_v = Split(Split(raw_shot, '|')[0], ' ');
  /*
  for (unsigned int i = 0; i < symbol_v.size(); i++) {
    printf("%s\n", symbol_v[i].c_str());
  }
  */
  snprintf(shot.ticker, sizeof(shot.ticker), "%s", symbol_v[symbol_v.size()-2].c_str());
  std::string price_info = Split(raw_shot, '|')[1];
  price_info.erase(0, 1);
  price_info.erase(price_info.size()-1, 1);
  std::vector<std::string>bid_ask = Split(price_info, ' ');
  shot.bids[0] = atof(bid_ask[0].c_str());
  shot.asks[0] = atof(bid_ask[1].c_str());
  std::string size_info = Split(raw_shot, '|')[2];
  // printf("size info is %s\n", size_info.c_str());
  std::vector<std::string>size_v = Split(size_info, 'x');
  /*
  for (unsigned int i = 0; i < size_v.size(); i++) {
    printf("size[%d] = %s\n", i, size_v[i].c_str());
  }
  */
  shot.bid_sizes[0] = atoi(size_v[0].c_str());
  shot.ask_sizes[0] = atoi(size_v[1].c_str());
  shot.time.tv_sec = atoi(symbol_v[0].c_str());
  // shot.last_trade = Split(raw_shot, '|').back();
  std::vector<std::string> last_v = Split(Split(raw_shot, '|').back(), ' ');
  last_v.erase(last_v.begin());
  /*
  for (unsigned int i = 0; i < last_v.size(); i++) {
    printf("last[%d] = %s\n", i, last_v[i].c_str());
  }
  */
  shot.last_trade = atof(last_v[0].c_str());
  shot.last_trade_size = atoi(last_v[1].c_str());
  shot.volume = atoi(last_v[2].c_str());
  shot.turnover = atof(last_v[4].c_str());
  shot.open_interest = atoi(last_v[5].c_str());
  // printf("last is %s\n", Split(raw_shot, '|').back().c_str());
  // shot.Show(stdout, 1);
  return shot;
}

void DataGener::Run() {
  std::cout << file_name << endl;
  char buffer[SIZE_OF_SNAPSHOT];
  while (true) {
    raw_file.open(file_name.c_str(), ios::in);
    while (!raw_file.eof()) {
      raw_file.getline(buffer, SIZE_OF_SNAPSHOT);
      std::cout << buffer << endl;
      if (buffer[0] == '\0') {
        continue;
      }
      MarketSnapshot snapshot = HandleSnapshot(buffer);
      snapshot.Show(stdout);
      // char c[2048];
      // memcpy(c, &snapshot, sizeof(snapshot));
      sender->Send(snapshot);
      // std::cout << "send" << c << " out" << endl;
      // sleep(1);
    }
    raw_file.close();
    exit(1);
  }
}
