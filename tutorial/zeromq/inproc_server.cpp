#include <zmq.hpp>
#include <string>
#include <iostream>

using namespace std;

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

int main() {
  zmq::context_t context(0);
  zmq::socket_t socket(context, ZMQ_PUB);
  socket.bind("inproc://my_test");
  char a[32];
  pthread_t thread;
  if (pthread_create(&thread,
                     NULL,
                     &Run,
                     &context) != 0) {
    exit(1);
  }
  while (std::cin >> a) {
    socket.send(a, sizeof(a));
    std::cout << "send " << a << endl;
  }
}
