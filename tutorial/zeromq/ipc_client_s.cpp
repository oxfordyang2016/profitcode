#include <zmq.hpp>
#include <string>
#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>

struct huang {
  char a[1024];
  int b;
  char c[10];
};

using namespace std;
int main() {
  zmq::context_t context(1);
  zmq::socket_t socket(context, ZMQ_SUB);
  socket.setsockopt(ZMQ_SUBSCRIBE, 0, 0);
  socket.connect("ipc://my_test");
  char a[2048];
  huang* h;
  while (true) {
    timeval t;
    // socket.recv(a, sizeof(a));
    socket.recv(a, sizeof(a));
    h = reinterpret_cast<huang*>(a);
    gettimeofday(&t, NULL);
    // std::cout << "recved " << a << " at " << t.tv_sec << " " << t.tv_usec << endl;
    printf("recved %s %d %s at %ld %ld\n", h->a, h->b, h->c, t.tv_sec, t.tv_usec);
  }
}
