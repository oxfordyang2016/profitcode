#ifndef SENDER_H_
#define SENDER_H_

#include "define.h"
#include "order.h"
#include <zmq.hpp>
#include <string>
#include "exchange_info.h"
#include "common/market_snapshot.h"

using namespace std;

class Sender {
 public:
  Sender(string name) {
    pthread_mutex_init(&mutex, NULL);
    con = new zmq::context_t(1);
    sock = new zmq::socket_t(*con, ZMQ_PUB);
    string ipc_name = "ipc://" + name;
    sock->bind(ipc_name.c_str());
    sleep(1);
  }

  ~Sender() {
    delete con;
    delete sock;
  }

  void Send(MarketSnapshot shot) {
    char buffer[BUFFER_SIZE];
    if (BUFFER_SIZE < sizeof(shot)) {
      printf("buffer size is not enough, 28\n");
      exit(1);
    }
    memcpy(buffer, &shot, sizeof(shot));
    pthread_mutex_lock(&mutex);
    sock->send(buffer, sizeof(buffer));
    pthread_mutex_unlock(&mutex);
  }

  void Send(Order order) {
    char buffer[BUFFER_SIZE];
    if (BUFFER_SIZE < sizeof(order)) {
      printf("buffer size is not enough, 28\n");
      exit(1);
    }
    memcpy(buffer, &order, sizeof(order));
    pthread_mutex_lock(&mutex);
    sock->send(buffer, sizeof(buffer));
    pthread_mutex_unlock(&mutex);
  }

  void Send(ExchangeInfo info) {
    char buffer[BUFFER_SIZE];
    if (BUFFER_SIZE < sizeof(info)) {
      printf("buffer size is not enough, 28\n");
      exit(1);
    }
    memcpy(buffer, &info, sizeof(info));
    pthread_mutex_lock(&mutex);
    /*
    try {
      sock->send(buffer, sizeof(buffer));
    } catch(int errno) {
      if (errno == EINTR) {
        printf("interrupt!");
        exit(1);
      }
    }
    int mark = 0;
    do {
      if (mark > 0) {
        printf("met EINTR %d\n", mark);
      }
      int nbytes = zmq_send(sock, buffer, sizeof(buffer), 0);
      if (zmq_errno () == EAGAIN)
        return;
      mark++;
    } while ((zmq_errno() == EINTR));
    */
    sock->send(buffer, sizeof(buffer));
    /*
    int nbytes = zmq_send(sock, buffer, sizeof(buffer));
    if (zmq_errno() == EAGAIN)
      printf("error is eagin\n");
    if ()
    */
    pthread_mutex_unlock(&mutex);
  }

 private:
  zmq::context_t* con;
  zmq::socket_t* sock;
  pthread_mutex_t mutex;
};

#endif // SENDER_H_
