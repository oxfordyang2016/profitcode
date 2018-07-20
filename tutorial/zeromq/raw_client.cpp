#include <zmq.hpp>
#include <string>
#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>

using namespace std;
int main() {
  zmq::context_t context(1);
  zmq::socket_t socket(context, ZMQ_SUB);
  socket.setsockopt(ZMQ_SUBSCRIBE, 0, 0);
  socket.connect("tcp://localhost:6777");
  char a[1024];
  while (true) {
    timeval t;
    socket.recv(a, sizeof(a));
    gettimeofday(&t, NULL);
    // std::cout << "recved " << a << " at " << t.tv_sec << " " << t.tv_usec << endl;
    printf("recved %s at %ld %ld\n", a, t.tv_sec, t.tv_usec);
  }
}
