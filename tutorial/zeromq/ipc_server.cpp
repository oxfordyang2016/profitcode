#include <zmq.hpp>
#include <string>
#include <iostream>
#include <unistd.h>
#include <sys/time.h>

using namespace std;

/*
void* Run(void *param) {
  zmq::context_t* context = reinterpret_cast<zmq::context_t*>(param);
  // zmq::context_t context(0);
  zmq::socket_t socket(*context, ZMQ_SUB);
  socket.setsockopt(ZMQ_SUBSCRIBE, "test", 4);
  socket.connect("inproc://my_test");
  char a[32];
  while (true) {
    socket.recv(a, sizeof(a));
    sleep(1);
    std::cout << "recv " << a << endl;
  }
}
*/

int main() {
  zmq::context_t context(1);
  zmq::socket_t socket(context, ZMQ_PUB);
  socket.bind("ipc://my_test");
  char a[1024];
  while (std::cin >> a) {
    timeval t;
    socket.send(a, sizeof(a));
    gettimeofday(&t, NULL);
    // std::cout << "send " << a << " at" << t.tv_sec << " " << t.tv_usec << endl;
    printf("recved %s at %ld %ld\n", a, t.tv_sec, t.tv_usec);
  }
}
