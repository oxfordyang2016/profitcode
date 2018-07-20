#ifndef RECVER_H_
#define RECVER_H_

#include "define.h"
#include "order.h"
#include <zmq.hpp>
#include <string>
#include "exchange_info.h"
#include "market_snapshot.h"
#include "pricer_data.h"

using namespace std;

class Recver {
 public:
  Recver(string name) {
    con = new zmq::context_t(1);
    sock = new zmq::socket_t(*con, ZMQ_SUB);
    string ipc_name = "ipc://" + name;
    sock->setsockopt(ZMQ_SUBSCRIBE, 0, 0);
    sock->connect(ipc_name.c_str());
    sleep(1);
  }

  ~Recver() {
    delete con;
    delete sock;
  }

  MarketSnapshot Recv(MarketSnapshot shot) {
    char buffer[BUFFER_SIZE];
    if (BUFFER_SIZE < sizeof(shot)) {
      printf("buffer size is not enough!\n");
      exit(1);
    }
    sock->recv(buffer, sizeof(buffer));
    MarketSnapshot* s = reinterpret_cast<MarketSnapshot*>(buffer);
    return *s;
  }

  Order Recv(Order order) {
    char buffer[BUFFER_SIZE];
    if (BUFFER_SIZE < sizeof(order)) {
      printf("buffer size is not enough!\n");
      exit(1);
    }
    sock->recv(buffer, sizeof(buffer));
    Order* o = reinterpret_cast<Order*>(buffer);
    return *o;
  }

  ExchangeInfo Recv(ExchangeInfo i) {
    char buffer[BUFFER_SIZE];
    if (BUFFER_SIZE < sizeof(i)) {
      printf("buffer size is not enough!\n");
      exit(1);
    }
    sock->recv(buffer, sizeof(buffer));
    ExchangeInfo* info = reinterpret_cast<ExchangeInfo*>(buffer);
    return *info;
  }

  PricerData Recv(PricerData p) {
    char buffer[BUFFER_SIZE];
    if (BUFFER_SIZE < sizeof(p)) {
      printf("buffer size is not enough!\n");
      exit(1);
    }
    sock->recv(buffer, sizeof(buffer));
    PricerData* pd = reinterpret_cast<PricerData*>(buffer);
    return *pd;
  }

 private:
  zmq::context_t* con;
  zmq::socket_t* sock;
};

#endif  //  RECVER_H_
