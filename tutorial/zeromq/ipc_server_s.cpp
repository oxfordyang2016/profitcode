#include <zmq.hpp>
#include <string>
#include <iostream>
#include <unistd.h>
#include <sys/time.h>

using namespace std;

struct huang {
  char a[1024];
  int b;
  char c[10];
};
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
  char a[2048];
  while (std::cin >> a) {
    timeval t;
    huang h;
    snprintf(h.a, sizeof(h.a), "%s", a);
    h.b = 10;
    snprintf(h.c, sizeof(h.c), "%s", "ending");
    char temp[4096];
    memcpy(temp, &h, sizeof(h));

    socket.send(temp, sizeof(temp));
    gettimeofday(&t, NULL);
    // std::cout << "send " << a << " at" << t.tv_sec << " " << t.tv_usec << endl;
    printf("sent %s at %ld %ld\n", a, t.tv_sec, t.tv_usec);
  }
}
